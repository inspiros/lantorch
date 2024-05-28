#pragma once

#include <gst/gst.h>
#include <gst/gstmeta.h>

#include "std/function_ptr.h"

#include "gst_buffer_probe.h"

/**
 * Lite version_ of NvDsFrameMeta is intended to be attached with a probe.
 */
typedef struct _GstFrameMeta {
    GstMeta meta;
    /** Holds the current frame number of the source. */
    guint64 frame_num;
    /** Holds the presentation timestamp (PTS) of the frame. */
    GstClockTime pts;
    /** Holds the decode timestamp (DTS) of the frame. */
    GstClockTime dts;
    /** Holds the duration of the frame. */
    GstClockTime duration;
    /** Holds the width of the frame. */
    gint source_frame_width;
    /** Holds the height of the frame. */
    gint source_frame_height;
} GstFrameMeta;

GType gst_frame_meta_api_get_type();

const GstMetaInfo *gst_frame_meta_get_info();

#define GST_FRAME_META_ADD(buf) ((GstFrameMeta *) gst_buffer_add_meta(buf, gst_frame_meta_get_info(), NULL))
#define GST_FRAME_META_GET(buf) ((GstFrameMeta *) gst_buffer_get_meta(buf, gst_frame_meta_api_get_type()))
#define GST_FRAME_META_INFO (gst_frame_meta_get_info())
#define GST_FRAME_META_API_TYPE (gst_frame_meta_api_get_type())

inline GstFrameMeta *gst_buffer_get_frame_meta(GstBuffer *buf) {
    return GST_FRAME_META_GET(buf);
}

/**
 * A self destruct callback class that add GstFrameMeta to buffer.
 */
class GstFrameMetaAddProbe : public GstBufferProbe {
    guint64 frame_count_ = 0;
public:
    using MetaType = GstFrameMeta;

    GstFrameMetaAddProbe() = default;

    [[nodiscard]] inline guint64 frame_count() const;

    inline void reset();

    GstPadProbeReturn call(GstPad *pad, GstPadProbeInfo *info, gpointer) override;
};
