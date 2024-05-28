#pragma once

#include <gst/gst.h>
#include <gst/video/video-format.h>
#include "gst/gst_frame_meta.h"

#include <opencv2/core.hpp>

#include <QMetaType>
#include <QImage>

#undef slots

#include <ATen/ATen.h>

#define slots Q_SLOTS

class GstInferenceSample {
    GstSample *sample_ = NULL;
    gboolean unref_ = TRUE;
    GstBuffer *buf_ = NULL;
    GstMapInfo map_info_{};
    gboolean map_successful_ = FALSE;
    GstFrameMeta *frame_meta_ = NULL;
    gint width_ = -1, height_ = -1;
    GstVideoFormat format_ = GST_VIDEO_FORMAT_UNKNOWN;

public:
    GstInferenceSample() = default;

    explicit GstInferenceSample(GstSample *sample, gboolean unref = TRUE);

    GstInferenceSample(GstBuffer *buf, GstCaps *caps, const GstSegment *segment, GstStructure *info);

    GstInferenceSample(const GstInferenceSample &other);

    ~GstInferenceSample();

    [[nodiscard]] GstSample *sample() const noexcept;

    [[nodiscard]] GstCaps *caps() const;

    [[nodiscard]] GstSegment *segment() const;

    [[nodiscard]] const GstStructure *info() const;

    [[nodiscard]] GstBuffer *buffer() const noexcept;

    [[nodiscard]] GstMapInfo map_info() const noexcept;

    [[nodiscard]] gboolean map_successful() const noexcept;

    [[nodiscard]] guint8 *data() const noexcept;

    [[nodiscard]] gsize data_size() const noexcept;

    [[nodiscard]] gint width() const noexcept;

    [[nodiscard]] gint height() const noexcept;

    [[nodiscard]] GstFrameMeta *frame_meta() const noexcept;

    [[nodiscard]] guint64 frame_id() const;

    [[nodiscard]] GstClockTime pts() const;

    [[nodiscard]] GstClockTime dts() const;

    [[nodiscard]] GstClockTime duration() const;

    [[nodiscard]] gint source_width() const;

    [[nodiscard]] gint source_height() const;

    [[nodiscard]] GstInferenceSample to(GstCaps *to_caps) const;

    [[nodiscard]] GstInferenceSample to(GstVideoFormat format) const;

    [[nodiscard]] cv::Mat get_image() const;

    [[nodiscard]] QImage get_qimage() const;

    [[nodiscard]] at::Tensor get_tensor() const;
};

Q_DECLARE_METATYPE(GstInferenceSample)
