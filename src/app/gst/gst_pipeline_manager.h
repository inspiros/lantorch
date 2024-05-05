#pragma once

#include <map>

#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <gst/gstelement.h>
#include "gst/gst_frame_meta.h"

class GstPipelineManager {
    static constexpr GstParseFlags GST_BIN_PARSE_FLAGS = (GstParseFlags)(
            GST_PARSE_FLAG_FATAL_ERRORS |
            GST_PARSE_FLAG_NO_SINGLE_ELEMENT_BINS |
            GST_PARSE_FLAG_PLACE_IN_BIN);

    GstElement *pipeline_;
    std::map<const gchar *, GstElement *> bins_;

public:
    GstPipelineManager();

    ~GstPipelineManager();

    [[nodiscard]] inline GstElement *pipeline() const noexcept;

    [[nodiscard]] std::pair<GstState, GstState> state(GstClockTime timeout = 100000000) const;

    GstElement *add_inference_bin(const gchar *name);

    [[nodiscard]] GstElement *get_bin(const gchar *name) const;

    [[nodiscard]] GstElement *get_element(const gchar *name) const;

    [[nodiscard]] GstElement *get_element_by_factory_name(const gchar *factory_name) const;

    bool set_state(GstState new_state);

protected:
    void init_pipeline();

    void add_bin(const gchar *name, GstElement *bin);
};
