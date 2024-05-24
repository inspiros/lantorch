#include "yolo_inference_worker.h"

#include "../utils/debug_mode.h"

#include <QDebug>

YoloInferenceWorker::YoloInferenceWorker(GstElement *app_sink,
                                         at::Device device,
                                         at::ScalarType dtype,
                                         bool verbose)
        : DynamicUpdateInferenceWorker(app_sink),
          device_(device),
          dtype_(dtype),
          verbose_(verbose) {}

YoloInferenceWorker::YoloInferenceWorker(GstElement *app_sink,
                                         ultralytics::YoloOptions options,
                                         at::Device device,
                                         at::ScalarType dtype,
                                         bool verbose)
        : YoloInferenceWorker(app_sink, device, dtype, verbose) {
    update_options_later({}, {}, options);
}

YoloInferenceWorker::YoloInferenceWorker(GstElement *app_sink,
                                         const std::string &model_filepath,
                                         const std::string &classes_filepath,
                                         ultralytics::YoloOptions options,
                                         at::Device device,
                                         at::ScalarType dtype,
                                         bool verbose)
        : YoloInferenceWorker(app_sink, device, dtype, verbose) {
    update_model_later(model_filepath, classes_filepath, {}, {}, options);
}

std::optional<GstInferenceSample> YoloInferenceWorker::forward(const GstInferenceSample &sample) {
    AutoDebugMode m(verbose_);
    at::NoGradGuard g;

    DEBUG_ONLY([&]() {
        time_meter_.reset();
    })
    auto frame_id = sample.frame_id();
    auto img = sample.get_image();

    DEBUG_ONLY([&]() {
        time_meter_.tick();
    })
    // inference
    auto detections = model_.forward(img);
    DEBUG_ONLY([&]() {
        time_meter_.tick();
    })

    emit new_sample_and_result(frame_id, sample, detections);
    emit new_result(frame_id, detections);

    DEBUG_ONLY([&]() {
        time_meter_.duration_stats_update();
        auto result_str = c10::str(
                "[Detection] frame_id=", frame_id,
                ", n_detections=", detections.size(),
                "\n\ttotal_elapsed_time=", std::setw(5),
                (float) time_meter_.duration_cast<std::chrono::microseconds>().count() / 1000, "ms/",
                (float) time_meter_.mean_duration_cast<std::chrono::microseconds>()->count() / 1000, "ms",
                " (infer:", time_meter_.duration_cast<std::chrono::microseconds>(0, 1).count(), "µs)\n");
        for (auto &det: detections) {
            result_str = c10::str(
                    result_str, " ", det.label_id,
                    " (", model_.classes_name(det.label_id), ") - ",
                    det.confidence, " - ", det.bbox, "\n");
        }
        qInfo().noquote() << QString::fromStdString(result_str);
    })
    return std::nullopt;
}

void YoloInferenceWorker::update_model_later(const std::string &model_filepath,
                                             const std::string &classes_filepath,
                                             std::optional<at::Device> device,
                                             std::optional<at::ScalarType> dtype,
                                             std::optional<ultralytics::YoloOptions> options) {
    // we may load the model only once in setup(), but let's do it in update()
    update_later([this, model_filepath, classes_filepath, device, dtype, options]() {
        if (device.has_value())
            device_ = device.value();
        if (dtype.has_value())
            dtype_ = dtype.value();
        try {
            model_.load(model_filepath, device_);
            model_.to(device_, dtype_);
            model_.eval();
        } catch (const c10::Error &e) {
            std::cerr << "error loading the model\n";
            emit error(e.what());
        }
        if (!classes_filepath.empty())
            model_.load_classes(classes_filepath);
        if (options.has_value()) {
            options_ = options.value();
            model_.set_options(options_);
        }
    });
}

void YoloInferenceWorker::update_options_later(std::optional<at::Device> device,
                                               std::optional<at::ScalarType> dtype,
                                               std::optional<ultralytics::YoloOptions> options) {
    update_later([this, device, dtype, options]() {
        if (device.has_value())
            device_ = device.value();
        if (dtype.has_value())
            dtype_ = dtype.value();
        if (device.has_value() || dtype.has_value())
            model_.to(device_, dtype_);
        if (options.has_value()) {
            options_ = options.value();
            model_.set_options(options_);
        }
    });
}
