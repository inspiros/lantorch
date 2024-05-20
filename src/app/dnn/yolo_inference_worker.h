#pragma once

#include "gst/gst_inference_qthread.h"

#include "dnn/ultralytics/yolo.h"

#include "../utils/time_meter.h"
#include "../utils/debug_mode.h"

#include <QDebug>

class YoloInferenceWorker : public GstInferenceWorker {
Q_OBJECT  // Q_OBJECT does not allow template classes
    using Yolo = ultralytics::Yolo<INFERENCE_ENGINE_LibTorch>;
    Yolo model_;
    std::string model_filepath_;
    std::string classes_filepath_;

    time_meter<std::chrono::high_resolution_clock> time_meter_;

public:
    ultralytics::YoloOptions options;
    at::Device device;
    at::ScalarType dtype;
    bool verbose;

    YoloInferenceWorker(GstElement *app_sink,
                        std::string_view model_filepath,
                        std::string_view classes_filepath,
                        ultralytics::YoloOptions options = {},
                        at::Device device = at::kCPU,
                        at::ScalarType dtype = at::kFloat,
                        bool verbose = false)
            : GstInferenceWorker(app_sink),
              model_filepath_(model_filepath),
              classes_filepath_(classes_filepath),
              options(options),
              device(device),
              dtype(dtype),
              verbose(verbose) {}

    inline Yolo model() const {
        return model_;
    }

protected:
    void setup() override {
        try {
            model_ = Yolo(model_filepath_, classes_filepath_, options);
            model_.to(device, dtype);
            model_.eval();
        } catch (const c10::Error &e) {
            std::cerr << "error loading the model\n";
            throw e;
        }
    }

    std::optional<GstInferenceSample> forward(const GstInferenceSample &sample) override {
        AutoDebugMode m(verbose);
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

    void cleanup() override {
    }

signals:

    void new_result(unsigned int frame_id,
                    const std::vector<Detection> &detections);

    void new_sample_and_result(unsigned int frame_id,
                               const GstInferenceSample &sample,
                               const std::vector<Detection> &detections);
};
