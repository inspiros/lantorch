#pragma once

#include <gst/video/video-format.h>

#include <opencv2/core/hal/interface.h>

#include <QImage>

inline constexpr int gst_video_format_to_cv_mat_type(GstVideoFormat format) {
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
            return CV_8UC4;
        case GST_VIDEO_FORMAT_RGB:
        case GST_VIDEO_FORMAT_BGR:
            return CV_8UC3;
        case GST_VIDEO_FORMAT_GRAY8:
            return CV_8UC1;
        case GST_VIDEO_FORMAT_GRAY16_BE:
        case GST_VIDEO_FORMAT_GRAY16_LE:
            return CV_8UC2;
        default:
            g_error("%s cannot be converted to OpenCV data type", gst_video_format_to_string(format));
    }
}

inline constexpr QImage::Format gst_video_format_to_qimage_format(GstVideoFormat format) {
    switch (format) {
        case GST_VIDEO_FORMAT_RGBx:
            return QImage::Format_RGBX8888;
        case GST_VIDEO_FORMAT_RGBA:
            return QImage::Format_RGBA8888;
        case GST_VIDEO_FORMAT_ARGB:
            return QImage::Format_ARGB32;
        case GST_VIDEO_FORMAT_RGB:
            return QImage::Format_RGB888;
        case GST_VIDEO_FORMAT_BGR:
            return QImage::Format_BGR888;
        case GST_VIDEO_FORMAT_GRAY8:
            return QImage::Format_Grayscale8;
        case GST_VIDEO_FORMAT_GRAY16_BE:
        case GST_VIDEO_FORMAT_GRAY16_LE:
            return QImage::Format_Grayscale16;
        case GST_VIDEO_FORMAT_RGB16:
            return QImage::Format_RGB16;
        default:
            g_warning("%s cannot be converted to QImage::Format", gst_video_format_to_string(format));
            return QImage::Format_Invalid;
    }
}
