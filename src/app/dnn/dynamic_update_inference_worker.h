#pragma once

#include <QMutex>
#include <QMutexLocker>
#include <QQueue>

#include "gst/gst_inference_qthread.h"

/**
 * Base class for Inference Worker that can execute a submitted function
 * in update() by calling update_later(update_func).
 */
class DynamicUpdateInferenceWorker : public GstInferenceWorker {
Q_OBJECT
    QMutex update_queue_mutex_;
    QQueue<std::function<void(void)>> update_queue_;

public:
    explicit DynamicUpdateInferenceWorker(
            GstElement *app_sink, GstElement *app_src, GstVideoFormat format = GST_VIDEO_FORMAT_RGB,
            QObject *parent = nullptr);

    explicit DynamicUpdateInferenceWorker(
            GstElement *app_sink, GstVideoFormat format = GST_VIDEO_FORMAT_RGB, QObject *parent = nullptr);

protected:
    void update() override;

public slots:

    void update_later(const std::function<void(void)> &update_func);
};
