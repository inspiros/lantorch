#include "dynamic_update_inference_worker.h"

DynamicUpdateInferenceWorker::DynamicUpdateInferenceWorker(
        GstElement *app_sink, GstElement *app_src, GstVideoFormat format, QObject *parent)
        : GstInferenceWorker(app_sink, app_src, format, parent) {}

DynamicUpdateInferenceWorker::DynamicUpdateInferenceWorker(
        GstElement *app_sink, GstVideoFormat format, QObject *parent)
        : GstInferenceWorker(app_sink, format, parent) {}

void DynamicUpdateInferenceWorker::update() {
    QMutexLocker locker(&update_queue_mutex_);
    if (!update_queue_.empty()) {
        while (!update_queue_.empty())
            update_queue_.dequeue()();
    }
}

void DynamicUpdateInferenceWorker::update_later(const std::function<void(void)> &update_func) {
    QMutexLocker locker(&update_queue_mutex_);
    update_queue_.enqueue(update_func);
}
