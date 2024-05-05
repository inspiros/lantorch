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

#include <QException>
#include <QImage>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QSharedPointer>
#include "qt/threading/Event"

class GstInferenceWorker : public QObject {
Q_OBJECT
protected:
    GstElement *app_sink_ = nullptr;
    GstElement *app_src_ = nullptr;
    std::array<gulong, 3> app_src_cb_handlers_ = {0, 0, 0};
    GstVideoFormat format_ = GST_VIDEO_FORMAT_RGB;
    unsigned long num_processed_samples_ = 0;

    QRecursiveMutex mutex_;
    Event app_sink_set_event_{};
    Event app_src_set_event_{};
    Event app_src_need_data_event_{};
    Event started_event_{};
    Event unpaused_event_{};
    Event stopped_event_{};
    GstClockTime pull_sample_timeout_ = 100000000;

public:
    explicit GstInferenceWorker(
            GstElement *app_sink, GstElement *app_src, GstVideoFormat format = GST_VIDEO_FORMAT_RGB, QObject *parent = nullptr);

    explicit GstInferenceWorker(
            GstElement *app_sink, GstVideoFormat format = GST_VIDEO_FORMAT_RGB, QObject *parent = nullptr);

    virtual ~GstInferenceWorker();

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

signals:

    void eos();

    void finished();

public slots:

    void on_started();

    void on_paused(bool state);

    void on_pause_toggled();

    void on_stopped();

private:
    void run();

    void connect_app_src_cb();

    void disconnect_app_src_cb();

protected:
    bool should_abort();

    // these methods are to be implemented by subclasses
    virtual std::optional<GstInferenceSample> forward(const GstInferenceSample &sample) {
        throw std::not_implemented_error();
    }

    virtual void setup() {
    }

    virtual void cleanup() {
    }
};

class GstInferenceWorkerAlreadyInitializedException : public QException {
public:
    void raise() const override { throw *this; }

    [[nodiscard]] GstInferenceWorkerAlreadyInitializedException *clone() const override {
        return new GstInferenceWorkerAlreadyInitializedException(*this);
    }
};

class GstInferenceQThread : public QThread {
Q_OBJECT
    QMutex mutex_;
    QSharedPointer<GstInferenceWorker> worker_ = nullptr;
    qint64 termination_deadline_ = 10000;

public:
    explicit GstInferenceQThread(QObject *parent = nullptr);

    explicit GstInferenceQThread(qint64 termination_deadline, QObject *parent = nullptr);

    ~GstInferenceQThread() override;

    template<typename Worker, typename... Args,
            typename std::enable_if<std::is_convertible_v<Worker *, GstInferenceWorker *>, bool>::type = true>
    QSharedPointer<Worker> init_worker(Args &&... args) {
        if (isWorkerInitialized())
            throw GstInferenceWorkerAlreadyInitializedException();

        auto *new_worker = new Worker(std::forward<Args>(args)...);
        {
            QMutexLocker locker(&mutex_);
            worker_ = QSharedPointer<GstInferenceWorker>(qobject_cast<GstInferenceWorker *>(new_worker));
            worker_->moveToThread(this);

            QObject::connect(this, SIGNAL(started()), worker_.data(), SLOT(on_started()));
            QObject::connect(this, SIGNAL(paused(bool)), worker_.data(), SLOT(on_paused(bool)),
                             Qt::DirectConnection);
            QObject::connect(this, SIGNAL(pause_toggled()), worker_.data(), SLOT(on_pause_toggled()),
                             Qt::DirectConnection);
            QObject::connect(this, SIGNAL(stopped()), worker_.data(), SLOT(on_stopped()),
                             Qt::DirectConnection);
            QObject::connect(worker_.data(), SIGNAL(finished()), this, SLOT(deleteWorker()));
        }

        return worker<Worker>();
    }

    template<class Worker,
            typename std::enable_if<std::is_convertible_v<Worker *, GstInferenceWorker *>, bool>::type = true>
    inline QSharedPointer<Worker> worker() {
        QMutexLocker locker(&mutex_);
        auto *ptr = qobject_cast<Worker *>(worker_.data());
        return QtSharedPointer::copyAndSetPointer(ptr, worker_);
    }

    bool isWorkerInitialized() noexcept;

    inline void pause(bool state = true) noexcept {
        emit paused(state);
    }

    inline void toggle_pause() noexcept {
        emit pause_toggled();
    }

    inline void stop() noexcept {
        emit stopped();
    }

private slots:

    void deleteWorker() noexcept;

signals:

    void paused(bool);

    void pause_toggled();

    void stopped();
};
