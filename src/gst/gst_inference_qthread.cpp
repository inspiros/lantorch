#include "gst_inference_qthread.h"

#include <QTimer>

GstInferenceWorker::GstInferenceWorker(GstElement *app_sink, GstElement *app_src, GstVideoFormat format,
                                       QObject *parent)
        : QObject(parent), format_(format) {
    set_app_sink(app_sink);
    set_app_src(app_src);
    unpaused_event_.set();
}

GstInferenceWorker::GstInferenceWorker(GstElement *app_sink, GstVideoFormat format, QObject *parent)
        : GstInferenceWorker(app_sink, NULL, format, parent) {}

GstInferenceWorker::~GstInferenceWorker() {
    if (!is_stopped())
        on_stopped();
}

void GstInferenceWorker::set_app_sink(GstElement *app_sink) {
    if (app_sink && !GST_IS_APP_SINK(app_sink))
        g_error("sink is not an appsink");
    QMutexLocker lock(&mutex_);
    app_sink_ = app_sink;
    if (app_sink_)
        app_sink_set_event_.set();
    else
        app_sink_set_event_.clear();
}

void GstInferenceWorker::set_app_src(GstElement *app_src) {
    if (app_src && !GST_IS_APP_SRC(app_src))
        g_error("element is not an appsrc");
    QMutexLocker lock(&mutex_);
    disconnect_app_src_cb();
    app_src_ = app_src;
    if (app_src_)
        app_src_set_event_.set();
    else
        app_src_set_event_.clear();
    connect_app_src_cb();
}

void GstInferenceWorker::connect_app_src_cb() {
    if (app_src_set_event_.isSet()) {
        QMutexLocker lock(&mutex_);
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

void GstInferenceWorker::disconnect_app_src_cb() {
    if (app_src_set_event_.isSet()) {
        QMutexLocker lock(&mutex_);
        for (auto &app_src_cb_handler: app_src_cb_handlers_) {
            if (app_src_cb_handler) {
                g_signal_handler_disconnect(app_src_, app_src_cb_handler);
                app_src_cb_handler = 0;
            }
        }
    }
}

void GstInferenceWorker::set_format(GstVideoFormat format) {
    format_ = format;
}

void GstInferenceWorker::set_pull_sample_timeout(GstClockTime timeout) {
    pull_sample_timeout_ = timeout;
}

unsigned long GstInferenceWorker::num_processed_samples() const {
    return num_processed_samples_;
}

bool GstInferenceWorker::has_sink() {
    return app_sink_set_event_.isSet();
}

bool GstInferenceWorker::has_src() {
    return app_src_set_event_.isSet();
}

bool GstInferenceWorker::is_started() {
    return started_event_.isSet();
}

void GstInferenceWorker::on_started() {
    started_event_.set();
    QMetaObject::invokeMethod(qobject_cast<GstInferenceWorker *>(this), &GstInferenceWorker::run, Qt::DirectConnection);
}

bool GstInferenceWorker::is_paused() {
    return !unpaused_event_.isSet();
}

void GstInferenceWorker::on_paused(bool state) {
    if (state)
        unpaused_event_.clear();
    else
        unpaused_event_.set();
}

void GstInferenceWorker::on_pause_toggled() {
    if (!unpaused_event_.isSet())
        unpaused_event_.set();
    else
        unpaused_event_.clear();
}

bool GstInferenceWorker::is_stopped() {
    return stopped_event_.isSet();
}

void GstInferenceWorker::on_stopped() {
    stopped_event_.set();
    unpaused_event_.set();
    app_sink_set_event_.set();
    disconnect_app_src_cb();
    app_src_need_data_event_.set();
    started_event_.set();
    disconnect();
}

bool GstInferenceWorker::should_abort() {
    return is_stopped() || QThread::currentThread()->isInterruptionRequested();
}

void GstInferenceWorker::run() {
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
            QMutexLocker lock(&mutex_);
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
                        QMutexLocker lock(&mutex_);
                        if (gst_app_src_push_sample(GST_APP_SRC(app_src_), out_infer_sample->sample()) != GST_FLOW_OK)
                            g_printerr("Failed to push sample to appsrc\n");
                    }
                }
            }
        } else if (gst_app_sink_is_eos(GST_APP_SINK(app_sink_))) {
            unpaused_event_.clear();
            if (app_src_set_event_.isSet()) {
                QMutexLocker lock(&mutex_);
                gst_app_src_end_of_stream(GST_APP_SRC(app_src_));
                emit eos();
            }
        }
    }
    cleanup();
    emit finished();
}

GstInferenceQThread::GstInferenceQThread(QObject *parent) : QThread(parent) {}

GstInferenceQThread::GstInferenceQThread(qint64 termination_deadline, QObject *parent)
        : QThread(parent), termination_deadline_(termination_deadline) {}

GstInferenceQThread::~GstInferenceQThread() {
    stop();
    requestInterruption();
    quit();
    if (!wait(QDeadlineTimer(termination_deadline_)))
        terminate();
}

bool GstInferenceQThread::isWorkerInitialized() noexcept {
    QMutexLocker locker(&mutex_);
    return !worker_.isNull();
}

void GstInferenceQThread::deleteWorker() noexcept {
    QMutexLocker locker(&mutex_);
    worker_.reset();
}
