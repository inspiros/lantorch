#include "gst_inference_sample.h"

#include <gst/video/video-converter.h>

#include "std/function_ptr.h"

#include "utils/video_format.h"

GstInferenceSample::GstInferenceSample(GstSample *sample, gboolean unref)
        : sample_(sample), unref_(unref) {
    if (sample_) {
        GstCaps *caps = gst_sample_get_caps(sample_);
        GstStructure *structure = gst_caps_get_structure(caps, 0);
        gst_structure_get_int(structure, "width", &width_);
        gst_structure_get_int(structure, "height", &height_);
        format_ = gst_video_format_from_string(gst_structure_get_string(structure, "format"));

        buf_ = gst_sample_get_buffer(sample_);
        frame_meta_ = gst_buffer_get_frame_meta(buf_);
        map_successful_ = gst_buffer_map(buf_, &map_info_, GST_MAP_READ);
    }
}

GstInferenceSample::GstInferenceSample(GstBuffer *buf, GstCaps *caps, const GstSegment *segment, GstStructure *info)
        : GstInferenceSample(gst_sample_new(buf, caps, segment, info), TRUE) {}

GstInferenceSample::GstInferenceSample(const GstInferenceSample &other)
        : sample_(other.sample_), unref_(other.unref_), buf_(other.buf_), frame_meta_(other.frame_meta_),
          width_(other.width_), height_(other.height_), format_(other.format_) {
    if (unref_ && sample_)
        gst_sample_ref(sample_);
    map_successful_ = gst_buffer_map(buf_, &map_info_, GST_MAP_READ);
}

GstInferenceSample::~GstInferenceSample() {
    if (map_successful_)
        gst_buffer_unmap(buf_, &map_info_);
    if (unref_)
        gst_sample_unref(sample_);
}

GstSample *GstInferenceSample::sample() const noexcept {
    return sample_;
}

GstCaps *GstInferenceSample::caps() const {
    return gst_sample_get_caps(sample_);
}

GstSegment *GstInferenceSample::segment() const {
    return gst_sample_get_segment(sample_);
}

const GstStructure *GstInferenceSample::info() const {
    return gst_sample_get_info(sample_);
}

GstBuffer *GstInferenceSample::buffer() const noexcept {
    return buf_;
}

GstMapInfo GstInferenceSample::map_info() const noexcept {
    return map_info_;
}

gboolean GstInferenceSample::map_successful() const noexcept {
    return map_successful_;
}

guint8 *GstInferenceSample::data() const noexcept {
    return map_info_.data;
}

gsize GstInferenceSample::data_size() const noexcept {
    return map_info_.size;
}

gint GstInferenceSample::width() const noexcept {
    return width_;
}

gint GstInferenceSample::height() const noexcept {
    return height_;
}

GstFrameMeta *GstInferenceSample::frame_meta() const noexcept {
    return frame_meta_;
}

guint64 GstInferenceSample::frame_id() const {
    if (frame_meta_)
        return frame_meta_->frame_num;
    return buf_->offset;
}

GstClockTime GstInferenceSample::pts() const {
    if (frame_meta_)
        return frame_meta_->pts;
    return buf_->pts;
}

GstClockTime GstInferenceSample::dts() const {
    if (frame_meta_)
        return frame_meta_->dts;
    return buf_->dts;
}

GstClockTime GstInferenceSample::duration() const {
    if (frame_meta_)
        return frame_meta_->duration;
    return buf_->duration;
}

gint GstInferenceSample::source_width() const {
    return frame_meta_->source_frame_width;
}

gint GstInferenceSample::source_height() const {
    return frame_meta_->source_frame_height;
}

GstInferenceSample GstInferenceSample::to(GstCaps *to_caps) const {
    if (!sample_)
        return *this;
    GError *err = NULL;
    auto *target_sample = gst_video_convert_sample(sample_, to_caps, GST_SECOND, &err);
    if (err) {
        gint code = err->code;
        const char *msg = err->message;
        g_error_free(err);
        gst_sample_unref(target_sample);
        g_printerr("Unable to convert sample to %s. Encountered error %d: %s\n",
                   gst_caps_to_string(to_caps), code, msg);
        return {};
    }
    return GstInferenceSample(target_sample, TRUE);
}

GstInferenceSample GstInferenceSample::to(GstVideoFormat format) const {
    if (!sample_)
        return *this;
    GstCaps *target_caps = gst_caps_copy(gst_sample_get_caps(sample_));
    gst_caps_set_simple(target_caps, "format", G_TYPE_STRING, gst_video_format_to_string(format), NULL);
    auto res = to(target_caps);
    gst_caps_unref(target_caps);
    return res;
}

cv::Mat GstInferenceSample::get_image() const {
    if (map_successful()) {
        auto img = cv::Mat(cv::Size(width_, height_), gst_video_format_to_cv_mat_type(format_),
                           (unsigned char *) data(),
                           data_size() / height_);
        return img;
    }
    return {};
}

QImage GstInferenceSample::get_qimage() const {
    if (map_successful()) {
        gst_sample_ref(sample_);
        auto img = QImage((unsigned char *) data(), width_, height_, gst_video_format_to_qimage_format(format_),
                          std::function_ptr<void(void *)>([this](void *data) {
                              gst_sample_unref(sample_);
                          }));
        return img;
    }
    return {};
}

at::Tensor GstInferenceSample::get_tensor() const {
    if (map_successful()) {
        int channels;
        switch (gst_video_format_to_cv_mat_type(format_)) {
            case CV_8UC4:
                channels = 4;
                break;
            case CV_8UC3:
                channels = 3;
                break;
            case CV_8UC2:
                channels = 2;
                break;
            case CV_8UC1:
                channels = 1;
                break;
            default:  // not implemented
                AT_ASSERT(false, "Unable to convert to tensor");
        }
        gst_sample_ref(sample_);
        // TODO: strides
        auto tensor = at::from_blob((unsigned char *) data(), {height_, width_, channels}, {},
                                    std::function_ptr<void(void *)>([this](void *data) {
                                        gst_sample_unref(sample_);
                                    }),
                                    at::TensorOptions().dtype(at::kChar));
        return tensor;
    }
    return {};
}
