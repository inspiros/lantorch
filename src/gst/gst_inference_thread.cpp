#include "gst_inference_thread.h"

#include "gst_buffer.h"

GstInferenceThread::GstInferenceThread(GstElement *app_sink, GstElement *app_src, GstVideoFormat format)
        : format_(format) {
    set_app_sink(app_sink);
    set_app_src(app_src);
    thread_ = std::thread(&GstInferenceThread::run, this);
    unpaused_event_.set();
}

GstInferenceThread::GstInferenceThread(GstElement *app_sink, GstVideoFormat format)
        : GstInferenceThread(app_sink, NULL, format) {}

GstInferenceThread::GstInferenceThread(GstInferenceThread &&other) noexcept
        : app_sink_(other.app_sink_),
          app_src_(other.app_src_),
          format_(other.format_),
          num_processed_samples_(other.num_processed_samples_),
          thread_(std::move(other.thread_)),
          started_event_{},
          stopped_event_{} {
    if (other.stopped_event_.isSet())
        stopped_event_.set();
    if (other.app_sink_set_event_.isSet())
        app_sink_set_event_.set();
    if (other.app_src_set_event_.isSet()) {
        app_src_set_event_.set();
        other.disconnect_app_src_cb();
        connect_app_src_cb();
    }
    if (other.unpaused_event_.isSet())
        unpaused_event_.set();
    if (other.started_event_.isSet())
        started_event_.set();
}

GstInferenceThread::~GstInferenceThread() {
    if (!is_stopped())
        stop();
    if (thread_.joinable())
        thread_.join();
}

void GstInferenceThread::set_app_sink(GstElement *app_sink) {
    if (app_sink && !GST_IS_APP_SINK(app_sink))
        g_error("sink is not an appsink");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    app_sink_ = app_sink;
    if (app_sink_)
        app_sink_set_event_.set();
    else
        app_sink_set_event_.clear();
}

void GstInferenceThread::set_app_src(GstElement *app_src) {
    if (app_src && !GST_IS_APP_SRC(app_src))
        g_error("element is not an appsrc");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    disconnect_app_src_cb();
    app_src_ = app_src;
    if (app_src_)
        app_src_set_event_.set();
    else
        app_src_set_event_.clear();
    connect_app_src_cb();
}

void GstInferenceThread::connect_app_src_cb() {
    if (app_src_set_event_.isSet()) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        gboolean app_src_emit_signals;
        g_object_get(G_OBJECT(app_src_), "emit-signals", &app_src_emit_signals, NULL);
        if (app_src_emit_signals) {
            app_src_cb_handlers_[0] = g_signal_connect(
                    app_src_, "need-data", G_CALLBACK(std::function_ptr<void(GstElement *, guint, gpointer)>(
                    [this](GstElement *pipeline, guint size, gpointer user_data) {
                        app_src_need_data_event_.set();
                    })), NULL);
            app_src_cb_handlers_[1] = g_signal_connect(
                    app_src_, "enough-data", G_CALLBACK(std::function_ptr<void(GstElement *, guint, gpointer)>(
                    [this](GstElement *pipeline, guint size, gpointer user_data) {
                        app_src_need_data_event_.clear();
                    })), NULL);
        }
    }
}

void GstInferenceThread::disconnect_app_src_cb() {
    if (app_src_set_event_.isSet()) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        for (auto &app_src_cb_handler: app_src_cb_handlers_) {
            if (app_src_cb_handler) {
                g_signal_handler_disconnect(app_src_, app_src_cb_handler);
                app_src_cb_handler = 0;
            }
        }
    }
}

void GstInferenceThread::set_format(GstVideoFormat format) {
    format_ = format;
}

void GstInferenceThread::set_pull_sample_timeout(GstClockTime timeout) {
    pull_sample_timeout_ = timeout;
}

unsigned long GstInferenceThread::num_processed_samples() const {
    return num_processed_samples_;
}

bool GstInferenceThread::has_sink() {
    return app_sink_set_event_.isSet();
}

bool GstInferenceThread::has_src() {
    return app_src_set_event_.isSet();
}

bool GstInferenceThread::is_started() {
    return started_event_.isSet();
}

bool GstInferenceThread::is_paused() {
    return !unpaused_event_.isSet();
}

bool GstInferenceThread::is_stopped() {
    return stopped_event_.isSet();
}

void GstInferenceThread::start() {
    started_event_.set();
}

void GstInferenceThread::stop() {
    stopped_event_.set();
    unpaused_event_.set();
    app_sink_set_event_.set();
    disconnect_app_src_cb();
    app_src_need_data_event_.set();
    started_event_.set();
}

void GstInferenceThread::terminate() {
    stop();
}

bool GstInferenceThread::should_abort() {
    return is_stopped();
}

void GstInferenceThread::run() {
    started_event_.wait();
    if (!should_abort())
        setup();
    while (true) {
        unpaused_event_.wait();
        app_sink_set_event_.wait();
        if (app_src_set_event_.isSet() && app_src_cb_handlers_[0])
            app_src_need_data_event_.wait();
        if (should_abort())
            break;
        update();
        GstSample *sample = NULL;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            sample = gst_app_sink_try_pull_sample(GST_APP_SINK(app_sink_), pull_sample_timeout_);
        }
        if (sample) {
            auto infer_sample = GstInferenceSample(sample).to(format_);
            if (!infer_sample.map_successful())
                continue;
            auto out_infer_sample = forward(infer_sample);
            num_processed_samples_++;
            if (app_src_set_event_.isSet()) {
                if (!out_infer_sample.has_value()) {
                    g_printerr("forward doesn't return anything to push to appsrc\n");
                    continue;
                }
                if (out_infer_sample->caps())
                    gst_app_src_set_caps(GST_APP_SRC(app_src_), out_infer_sample->caps());
                GstBuffer *out_buf = out_infer_sample->buffer();
                if (out_buf) {
                    if (gst_buffer_is_writable(out_buf))
                        gst_buffer_copy_into(out_buf, infer_sample.buffer(), GST_BUFFER_COPY_METADATA, 0, 0);
                    out_buf->dts = GST_CLOCK_TIME_NONE;
                    {
                        std::lock_guard<std::recursive_mutex> lock(mutex_);
                        if (gst_app_src_push_sample(GST_APP_SRC(app_src_), out_infer_sample->sample()) != GST_FLOW_OK)
                            g_printerr("Failed to push sample to appsrc\n");
                    }
                }
            }
        } else if (gst_app_sink_is_eos(GST_APP_SINK(app_sink_))) {
            unpaused_event_.clear();
            if (app_src_set_event_.isSet()) {
                std::lock_guard<std::recursive_mutex> lock(mutex_);
                gst_app_src_end_of_stream(GST_APP_SRC(app_src_));
            }
        }
    }
    cleanup();
}
