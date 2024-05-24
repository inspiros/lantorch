#pragma once

#include "dynamic_update_inference_worker.h"

#include "dnn/ultralytics/yolo.h"

#include "../utils/time_meter.h"

class YoloInferenceWorker : public DynamicUpdateInferenceWorker {
Q_OBJECT  // Q_OBJECT does not allow template classes
    using Yolo = ultralytics::Yolo<INFERENCE_ENGINE_LibTorch>;
    Yolo model_;
    at::Device device_;
    at::ScalarType dtype_;
    ultralytics::YoloOptions options_;
    bool verbose_;

    time_meter<std::chrono::high_resolution_clock> time_meter_;

public:
    explicit YoloInferenceWorker(GstElement *app_sink,
                                 at::Device device = at::kCPU,
                                 at::ScalarType dtype = at::kFloat,
                                 bool verbose = false);

    explicit YoloInferenceWorker(GstElement *app_sink,
                                 ultralytics::YoloOptions options = {},
                                 at::Device device = at::kCPU,
                                 at::ScalarType dtype = at::kFloat,
                                 bool verbose = false);

    YoloInferenceWorker(GstElement *app_sink,
                        const std::string &model_filepath,
                        const std::string &classes_filepath = "",
                        ultralytics::YoloOptions options = {},
                        at::Device device = at::kCPU,
                        at::ScalarType dtype = at::kFloat,
                        bool verbose = false);

    inline Yolo model() const noexcept {
        return model_;
    }

    inline ultralytics::YoloOptions options() const noexcept {
        return options_;
    }

protected:
    std::optional<GstInferenceSample> forward(const GstInferenceSample &sample) override;

signals:

    void new_result(unsigned int frame_id,
                    const std::vector<Detection> &detections);

    void new_sample_and_result(unsigned int frame_id,
                               const GstInferenceSample &sample,
                               const std::vector<Detection> &detections);

    void error(const char *what);

public slots:

    void update_model_later(const std::string &model_filepath,
                            const std::string &classes_filepath = "",
                            std::optional<at::Device> device = {},
                            std::optional<at::ScalarType> dtype = {},
                            std::optional<ultralytics::YoloOptions> options = {});

    void update_options_later(std::optional<at::Device> device = {},
                              std::optional<at::ScalarType> dtype = {},
                              std::optional<ultralytics::YoloOptions> options = {});
};
