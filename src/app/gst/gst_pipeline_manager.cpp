#include "gst_pipeline_manager.h"
#include "gst/gst_bin_get_element.h"
#include "gst/gst_element_pad_link.h"

#include <fmt/core.h>

#include "../app_config.h"

namespace {
    inline gboolean gst_pipeline_set_state(GstElement *pipeline, GstState state) {
        if (!pipeline)
            return FALSE;
        switch (gst_element_set_state(pipeline, state)) {
            case GST_STATE_CHANGE_SUCCESS:
            case GST_STATE_CHANGE_ASYNC:
            case GST_STATE_CHANGE_NO_PREROLL:
                return TRUE;
            case GST_STATE_CHANGE_FAILURE:
            default:
                gst_element_set_state(pipeline, GST_STATE_NULL);
                return FALSE;
        }
    }
}

GstPipelineManager::GstPipelineManager() {
    init_pipeline();
}

GstPipelineManager::~GstPipelineManager() {
    if (pipeline_) {
        gst_element_set_state(pipeline_, GST_STATE_NULL);
        while (GST_OBJECT_REFCOUNT(GST_OBJECT_CAST(pipeline_)))
            gst_object_unref(pipeline_);
    }
}

void GstPipelineManager::init_pipeline() {
    auto pipeline_config = AppConfig::instance()["app"]["gst"]["pipeline"];

    /* Init pipeline */
    pipeline_ = gst_pipeline_new(pipeline_config["name"].as<std::string>().c_str());
    GError *error = NULL;
    auto stem_bin_description = pipeline_config["stem_bin"]["description"].as<std::string>();
    auto stem_bin = gst_parse_launch_full(
            stem_bin_description.c_str(), NULL, GST_BIN_PARSE_FLAGS, &error);
    if (error)
        g_error("failed to init stem_bin");
    if (!gst_bin_get_by_name(GST_BIN(stem_bin), "inference_tee"))
        g_error("inference_tee not found in stem_bin description");

    /* Add elements */
    add_bin("stem", stem_bin);

    /* Add probes */
    auto *frame_meta_add_probe = new GstFrameMetaAddProbe();
    frame_meta_add_probe->add_on(
            get_element(
                    pipeline_config["frame_meta_probe"]["element"].as<std::string>().c_str()),
            pipeline_config["frame_meta_probe"]["pad"].as<std::string>().c_str());
}

GstElement *GstPipelineManager::pipeline() const noexcept {
    return pipeline_;
}

std::pair<GstState, GstState> GstPipelineManager::state(GstClockTime timeout) const {
    GstState s, p;
    gst_element_get_state(pipeline_, &s, &p, timeout);
    return std::make_pair(s, p);
}

GstElement *GstPipelineManager::add_inference_bin(const gchar *name) {
    auto pipeline_config = AppConfig::instance()["app"]["gst"]["pipeline"];

    auto inference_bin_description =
            fmt::format(
                    pipeline_config["inference_bin"]["description"].as<std::string>(),
                    fmt::arg("bin_name", name));

    GError *error = NULL;
    GstElement *bin = gst_parse_launch_full(
            inference_bin_description.c_str(), NULL, GST_BIN_PARSE_FLAGS, &error);
    if (error)
        g_error("failed to init %s_bin", name);

    add_bin(name, bin);

    /* Link inference_tee */
    GstElement *bin_sink_element = NULL;
    if (pipeline_config["inference_bin"]["src_element"].IsDefined()) {
        bin_sink_element = gst_bin_get_by_name(
                GST_BIN(bin), fmt::format(
                        pipeline_config["inference_bin"]["src_element"].as<std::string>(),
                        fmt::arg("bin_name", name)).c_str());
    } else {
        bin_sink_element = gst_bin_get_element_by_factory_name(GST_BIN(bin), "queue") ?:
                           gst_bin_get_element_by_factory_name(GST_BIN(bin), "queue2") ?:
                           gst_bin_get_first_added_element(GST_BIN(bin));
    }
    if (!gst_element_link(get_element("inference_tee"), bin_sink_element))
        g_error("failed to link tee -> %s_bin", name);
    return bin;
}

void GstPipelineManager::add_bin(const gchar *name, GstElement *bin) {
    g_assert(bins_.find(name) == bins_.end());
    gst_bin_add(GST_BIN(pipeline_), bin);
    gst_element_sync_state_with_parent(bin);
    bins_.insert({name, bin});
}

GstElement *GstPipelineManager::get_bin(const gchar *name) const {
    return bins_.at(name);
}

GstElement *GstPipelineManager::get_element(const gchar *name) const {
    return gst_bin_get_by_name(GST_BIN(pipeline_), name);
}

GstElement *GstPipelineManager::get_element_by_factory_name(const gchar *factory_name) const {
    return gst_bin_get_element_by_factory_name(GST_BIN(pipeline_), factory_name);
}

bool GstPipelineManager::set_state(GstState new_state) {
    return gst_pipeline_set_state(pipeline_, new_state);
}
