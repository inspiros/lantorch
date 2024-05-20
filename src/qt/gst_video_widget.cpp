#include "gst_video_widget.h"

#include <cmath>

#include <gst/video/videooverlay.h>

#include "gst/gst_probe_utils.h"

GstVideoWidget::GstVideoWidget(QWidget *parent)
        : MediaWidget(parent) {
}

GstVideoWidget::~GstVideoWidget() {
    remove_probes();
}

void GstVideoWidget::set_qwidget5videosink(GstElement *sink) {
    if (!g_str_equal(gst_object_get_name(GST_OBJECT(gst_element_get_factory(sink))), "qwidget5videosink"))
        g_error("sink is not an instance of qwidget5videosink");
    remove_probes();
    sink_ = sink;
    g_object_set(G_OBJECT(sink), "widget", this, NULL);
    update_force_aspect_ratio();
    add_probes();
    emit sink_changed(sink);
}

void GstVideoWidget::set_overlay_videosink(GstElement *sink) {
    auto *overlay = GST_VIDEO_OVERLAY(sink);
    remove_probes();
    sink_ = sink;
    gst_video_overlay_set_window_handle(overlay, this->winId());
    update_force_aspect_ratio();
    add_probes();
    emit sink_changed(sink);
}

void GstVideoWidget::remove_sink() {
    remove_probes();
    sink_ = NULL;
}

GstElement *GstVideoWidget::sink() const {
    return sink_;
}

bool GstVideoWidget::has_sink() const {
    return sink_;
}

void GstVideoWidget::add_probes() {
    if (has_sink()) {
        // frame id probe
        probes_ids_[0] = gst_element_pad_add_probe(
                sink_, "sink", GST_PAD_PROBE_TYPE_BUFFER,
                [=](GstPad *pad, GstPadProbeInfo *info, gpointer) -> GstPadProbeReturn {
                    GstBuffer *buf = gst_pad_probe_info_get_buffer(info);
                    if (buf) {
                        emit frame_id_changed(buf->offset);
                        emit frame_pts_changed(buf->pts);
                    }
                    return GST_PAD_PROBE_OK;
                });
        // frame size probe
        probes_ids_[1] = gst_element_pad_add_probe(
                sink_, "sink", GST_PAD_PROBE_TYPE_BUFFER,
                [=](GstPad *pad, GstPadProbeInfo *info, gpointer) -> GstPadProbeReturn {
                    GstCaps *caps = gst_pad_get_current_caps(pad);
                    GstStructure *structure = gst_caps_get_structure(caps, 0);
                    auto width = g_value_get_int(gst_structure_get_value(structure, "width"));
                    auto height = g_value_get_int(gst_structure_get_value(structure, "height"));
                    if (width != frame_width() || height != frame_height())
                            emit frame_size_changed({width, height});
                    gst_caps_unref(caps);
                    return GST_PAD_PROBE_OK;
                });
    }
}

void GstVideoWidget::remove_probes() {
    if (probes_ids_[0] && has_sink()) {
        try {
            if (GST_OBJECT_REFCOUNT(GST_OBJECT_CAST(sink_))) {
                gst_element_pad_remove_probe(sink_, "sink", probes_ids_[0]);
                gst_element_pad_remove_probe(sink_, "sink", probes_ids_[1]);
            }
        } catch (...) {
        }
    }
    probes_ids_ = {0, 0};
}

void GstVideoWidget::update_force_aspect_ratio() {
    if (has_sink() && g_object_class_find_property(G_OBJECT_GET_CLASS(sink_), "force-aspect-ratio")) {
        gboolean far_value;
        g_object_get(G_OBJECT(sink_), "force-aspect-ratio", &far_value, NULL);
        set_force_aspect_ratio(far_value);
    } else
        set_force_aspect_ratio(false);
}
