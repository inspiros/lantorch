#include "gst_buffer.h"

#include <cstring>
#include <cmath>

namespace {
    template<gint ru = 0>
    inline gint rstride(gint base) {
        if constexpr (ru == 0)
            return base;
        else
            return (gint) std::ceil((gdouble) (base) / ru) * ru;
    }
}

GstBuffer *gst_buffer_new_memdup_from_contiguous(
        gconstpointer data, gint width, gint height, GstVideoFormat format) {
    gint src_step, step;
    // Ref: https://gstreamer.freedesktop.org/documentation/additional/design/mediatype-video-raw.html?gi-language=c#formats
    switch (format) {
        case GST_VIDEO_FORMAT_AYUV:
        case GST_VIDEO_FORMAT_RGBx:
        case GST_VIDEO_FORMAT_BGRx:
        case GST_VIDEO_FORMAT_xRGB:
        case GST_VIDEO_FORMAT_xBGR:
        case GST_VIDEO_FORMAT_RGBA:
        case GST_VIDEO_FORMAT_BGRA:
        case GST_VIDEO_FORMAT_ARGB:
        case GST_VIDEO_FORMAT_ABGR:
            return gst_buffer_new_memdup(data, height * width * 4);
        case GST_VIDEO_FORMAT_RGB:
        case GST_VIDEO_FORMAT_BGR:
            src_step = width * 3;
            step = rstride<4>(src_step);
            break;
        case GST_VIDEO_FORMAT_GRAY8:
            src_step = width;
            step = rstride<4>(src_step);
            break;
        case GST_VIDEO_FORMAT_GRAY16_BE:
        case GST_VIDEO_FORMAT_GRAY16_LE:
//        case GST_VIDEO_FORMAT_RGB16:
//        case GST_VIDEO_FORMAT_BGR16:
            src_step = width * 2;
            step = rstride<4>(src_step);
            break;
        default:
            g_error("%s is not supported", gst_video_format_to_string(format));
    }
    gsize s = step * height;
    const auto *data_uc = (const guint8 *) data;
    auto *buf = gst_buffer_new();
    auto mem = gst_allocator_alloc(NULL, s, NULL);
    gst_buffer_append_memory(buf, mem);
    GstMapInfo map_info;
    gst_buffer_map(buf, &map_info, GST_MAP_WRITE);
#pragma unroll
    for (gint row = 0; row < height; row++) {
        std::memcpy(map_info.data + row * step, data_uc + row * src_step, src_step);
    }
    return buf;
}
