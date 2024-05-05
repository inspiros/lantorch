#pragma once

#include <optional>
#include <thread>

#include <gst/gst.h>
#include <gst/app/app.h>
#include <gst/video/video-format.h>

#include <opencv2/core.hpp>

#include "std/threading/event.h"
#include "std/exception.h"

#include "gst_inference_sample.h"

class GstInferenceThread {
    GstElement *app_sink_ = nullptr;
    GstElement *app_src_ = nullptr;
    std::array<gulong, 3> app_src_cb_handlers_ = {0, 0, 0};
    GstVideoFormat format_ = GST_VIDEO_FORMAT_RGB;
    unsigned long num_processed_samples_ = 0;

    std::recursive_mutex mutex_;
    std::thread thread_;
    std::event app_sink_set_event_{};
    std::event app_src_set_event_{};
    std::event app_src_need_data_event_{};
    std::event started_event_{};
    std::event unpaused_event_{};
    std::event stopped_event_{};
    GstClockTime pull_sample_timeout_ = 100000000;

public:
    explicit GstInferenceThread(
            GstElement *app_sink, GstElement *app_src, GstVideoFormat format = GST_VIDEO_FORMAT_RGB);

    explicit GstInferenceThread(
            GstElement *app_sink, GstVideoFormat format = GST_VIDEO_FORMAT_RGB);

    GstInferenceThread(const GstInferenceThread &) = delete;

    GstInferenceThread(GstInferenceThread &&other) noexcept;

    GstInferenceThread &operator=(const GstInferenceThread &) = delete;

    GstInferenceThread &operator=(GstInferenceThread &&other) = delete;

    ~GstInferenceThread();

    void set_app_sink(GstElement *app_sink);

    void set_app_src(GstElement *app_src);

    void set_format(GstVideoFormat format);

    void set_pull_sample_timeout(GstClockTime timeout);

    [[nodiscard]] unsigned long num_processed_samples() const;

    bool has_sink();

    bool has_src();

    bool is_started();

    bool is_paused();

    bool is_stopped();

    void start();

    void stop();

    void terminate();

    bool should_abort();

private:
    void run();

    void connect_app_src_cb();

    void disconnect_app_src_cb();

protected:
    // these methods are to be implemented by subclasses
    virtual std::optional<GstInferenceSample> forward(const GstInferenceSample &sample) {
        throw std::not_implemented_error();
    }

    virtual void setup() {
    }

    virtual void cleanup() {
    }
};
