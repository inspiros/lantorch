#pragma once

#undef slots

#include <torch/script.h>

#define slots Q_SLOTS

#include "../inference_engine.h"
#include "../module.h"
#include "../return_types.h"

namespace ultralytics {
    enum YoloVersion {
        Yolo_UNKNOWN = 0,
        Yolov5,
        Yolov8,
    };

    struct YoloOptions {
    private:
        cv::Size input_shape_;
        float confidence_threshold_;
        float score_threshold_;
        float nms_threshold_;
        bool align_center_;

    public:
        YoloOptions()
                : input_shape_(640, 640),
                  confidence_threshold_(0.25),
                  score_threshold_(0.45),
                  nms_threshold_(0.5),
                  align_center_(true) {}

        explicit YoloOptions(const cv::Size &input_shape)
                : YoloOptions() {
            set_input_shape(input_shape);
        }

        [[nodiscard]] inline cv::Size input_shape() const noexcept {
            return input_shape_;
        }

        [[nodiscard]] inline int input_width() const noexcept {
            return input_shape_.width;
        }

        [[nodiscard]] inline int input_height() const noexcept {
            return input_shape_.height;
        }

        [[nodiscard]] inline float confidence_threshold() const noexcept {
            return confidence_threshold_;
        }

        [[nodiscard]] inline float score_threshold() const noexcept {
            return score_threshold_;
        }

        [[nodiscard]] inline float nms_threshold() const noexcept {
            return nms_threshold_;
        }

        [[nodiscard]] inline bool align_center() const noexcept {
            return align_center_;
        }

        [[nodiscard]] inline YoloOptions input_shape(const cv::Size &input_shape) const noexcept {
            auto r = *this;
            r.set_input_shape(input_shape);
            return r;
        }

        [[nodiscard]] inline YoloOptions confidence_threshold(float confidence_threshold) const noexcept {
            auto r = *this;
            r.set_confidence_threshold(confidence_threshold);
            return r;
        }

        [[nodiscard]] inline YoloOptions score_threshold(float score_threshold) const noexcept {
            auto r = *this;
            r.set_score_threshold(score_threshold);
            return r;
        }

        [[nodiscard]] inline YoloOptions nms_threshold(float nms_threshold) const noexcept {
            auto r = *this;
            r.set_nms_threshold(nms_threshold);
            return r;
        }

        [[nodiscard]] inline YoloOptions align_center(bool align_center) const noexcept {
            auto r = *this;
            r.set_align_center(align_center);
            return r;
        }

    private:
        inline void set_input_shape(const cv::Size &input_shape) & noexcept {
            input_shape_ = input_shape;
        }

        inline void set_confidence_threshold(float confidence_threshold) & noexcept {
            confidence_threshold_ = confidence_threshold;
        }

        inline void set_score_threshold(float score_threshold) & noexcept {
            score_threshold_ = score_threshold;
        }

        inline void set_nms_threshold(float nms_threshold) & noexcept {
            nms_threshold_ = nms_threshold;
        }

        inline void set_align_center(bool align_center) & noexcept {
            align_center_ = align_center;
        }
    };

    class YoloBase {
    protected:
        YoloVersion version_ = Yolo_UNKNOWN;
        YoloOptions options_;
        std::vector<std::string> classes_ = {
                "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck",
                "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench",
                "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra",
                "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis",
                "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard",
                "surfboard", "tennis racket", "bottle", "wine glass", "cup", "fork", "knife",
                "spoon", "bowl", "banana", "apple", "sandwich", "orange", "broccoli", "carrot",
                "hot dog", "pizza", "donut", "cake", "chair", "couch", "potted plant", "bed",
                "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard",
                "cell phone", "microwave", "oven", "toaster", "sink", "refrigerator", "book",
                "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush"};

        void load_classes(const std::string &classes_filepath);

    public:
        YoloBase() = default;

        explicit YoloBase(YoloOptions options);

        [[nodiscard]] YoloVersion version() const noexcept;

        [[nodiscard]] YoloOptions options() const noexcept;

        [[nodiscard]] int num_classes() const noexcept;

        [[nodiscard]] at::ArrayRef<std::string> classes_names() const noexcept;

        [[nodiscard]] std::string_view classes_name(int class_id) const;
    };

    /// Template class for Ultralytics's Yolo object detection (v5 or v8) model
    /// with different inference engines.
    template<InferenceEngine engine>
    class Yolo {
        static_assert(engine == INFERENCE_ENGINE_OpenCV || engine == INFERENCE_ENGINE_LibTorch,
                      "Yolo is only implemented for opencv or libtorch engine");
    };

    template<>
    class Yolo<INFERENCE_ENGINE_OpenCV> : public YoloBase, public OpenCVModule {
        int num_classes_{};

    public:
        explicit Yolo(YoloOptions options = {});

        explicit Yolo(const std::string &model_filepath,
                      YoloOptions options = {},
                      int backend = -1,
                      int target = -1);

        explicit Yolo(const std::string &model_filepath,
                      const std::string &classes_filepath,
                      YoloOptions options = {},
                      int backend = -1,
                      int target = -1);

        explicit Yolo(const std::string &model_filepath,
                      int num_classes,
                      YoloOptions options = {},
                      int backend = -1,
                      int target = -1);

        void load_onnx(const std::string &filename);

        void load_classes(const std::string &classes_filepath);

        [[nodiscard]] int num_classes() const noexcept;

        std::vector<Detection> forward(const cv::Mat &input);

        inline std::vector<Detection> operator()(const cv::Mat &input) {
            return forward(input);
        }
    };

    template<>
    class Yolo<INFERENCE_ENGINE_LibTorch> : public YoloBase, public LibTorchModule {
    public:
        explicit Yolo(YoloOptions options = {});

        explicit Yolo(const std::string &model_filepath,
                      YoloOptions options = {},
                      c10::optional<at::Device> device = c10::nullopt);

        explicit Yolo(const std::string &model_filepath,
                      const std::string &classes_filepath = "",
                      YoloOptions options = {},
                      c10::optional<at::Device> device = c10::nullopt);

        void load(const std::string &filename, c10::optional<at::Device> device = c10::nullopt);

        at::Tensor forward(const at::Tensor &input);

        std::vector<Detection> forward(const cv::Mat &input);

        inline at::Tensor operator()(const at::Tensor &input) {
            return forward(input);
        }

        inline std::vector<Detection> operator()(const cv::Mat &input) {
            return forward(input);
        }
    };

    // aliases
    using YoloOpenCV = Yolo<INFERENCE_ENGINE_OpenCV>;
    using YoloLibTorch = Yolo<INFERENCE_ENGINE_LibTorch>;
    using YoloOnnx = Yolo<INFERENCE_ENGINE_Onnxruntime>;
    using YoloTensorRT = Yolo<INFERENCE_ENGINE_TensorRT>;
}  // namespace ultralytics
