#include "gst_frame_meta.h"

namespace {
    inline gboolean init_func(GstMeta *meta, gpointer params, GstBuffer *buffer) {
        auto *frame_meta = (GstFrameMeta *) meta;
        frame_meta->frame_num = 0;
        frame_meta->pts = GST_CLOCK_TIME_NONE;
        frame_meta->dts = GST_CLOCK_TIME_NONE;
        frame_meta->duration = GST_CLOCK_TIME_NONE;
        frame_meta->source_frame_width = 0;
        frame_meta->source_frame_height = 0;
        return TRUE;
    }

    inline gboolean transform_func(GstBuffer *dest_buf,
                                   GstMeta *src_meta,
                                   GstBuffer *src_buf,
                                   GQuark type,
                                   gpointer data) {
        GstMeta *dest_meta = gst_buffer_add_meta(dest_buf, gst_frame_meta_get_info(), NULL);
        auto *src_frame_meta = (GstFrameMeta *) src_meta;
        auto *dest_frame_meta = (GstFrameMeta *) dest_meta;

        dest_frame_meta->frame_num = src_frame_meta->frame_num;
        dest_frame_meta->pts = src_frame_meta->pts;
        dest_frame_meta->dts = src_frame_meta->dts;
        dest_frame_meta->duration = src_frame_meta->duration;
        dest_frame_meta->source_frame_width = src_frame_meta->source_frame_width;
        dest_frame_meta->source_frame_height = src_frame_meta->source_frame_height;
        return TRUE;
    }

    inline void free_func(GstMeta *meta, GstBuffer *buffer) {
    }
}

GType gst_frame_meta_api_get_type() {
    static GType type;
    static const gchar *tags[] = {NULL};
    if (g_once_init_enter(&type)) {
        GType _type = gst_meta_api_type_register("GstFrameMetaAPI", tags);
        g_once_init_leave(&type, _type);
    }
    return type;
}

const GstMetaInfo *gst_frame_meta_get_info() {
    static const GstMetaInfo *meta_info = NULL;
    if (g_once_init_enter(&meta_info)) {
        const GstMetaInfo *meta =
                gst_meta_register(gst_frame_meta_api_get_type(),
                                  "GstFrameMeta",
                                  sizeof(GstFrameMeta),
                                  init_func,
                                  free_func,
                                  transform_func);
        g_once_init_leave(&meta_info, meta);
    }
    return meta_info;
}

guint64 GstFrameMetaAddProbe::frame_count() const {
    return frame_count_;
}

void GstFrameMetaAddProbe::reset() {
    frame_count_ = 0;
}

GstPadProbeReturn GstFrameMetaAddProbe::call(GstPad *pad, GstPadProbeInfo *info, gpointer) {
    auto *buf = (GstBuffer *) info->data;

    GstCaps *caps = gst_pad_get_current_caps(pad);
    GstStructure *structure = gst_caps_get_structure(caps, 0);
    gint width = g_value_get_int(gst_structure_get_value(structure, "width"));
    gint height = g_value_get_int(gst_structure_get_value(structure, "height"));
//    auto *framerate = gst_structure_get_value(structure, "framerate");
//    if (G_VALUE_TYPE(framerate) == GST_TYPE_FRACTION_RANGE)
//        framerate = gst_value_get_fraction_range_min(framerate);
    if (gst_buffer_is_writable(buf)) {
        auto frame_meta = GST_FRAME_META_ADD(buf);
        if (frame_meta == NULL)
            return GST_PAD_PROBE_OK;

        /* Add metadata */
        frame_meta->frame_num = frame_count_++;
        frame_meta->pts = GST_BUFFER_PTS(buf);
        frame_meta->dts = GST_BUFFER_DTS(buf);
        frame_meta->duration = GST_BUFFER_DURATION(buf);
        frame_meta->source_frame_width = width;
        frame_meta->source_frame_height = height;
    } else {
        // if buf is not writable, just modify it
        buf->offset = frame_count_++;
        buf->offset_end = buf->offset + 1;
    }
    return GST_PAD_PROBE_OK;
}
