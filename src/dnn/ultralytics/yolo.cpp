#include "yolo.h"
#include "nms.h"
#include "transforms.h"

#include <fstream>
#include <string>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

namespace ultralytics {
    namespace {
        C10_ALWAYS_INLINE YoloVersion _deduce_yolo_version(const cv::Mat &output_blob) {
            // yolov5 has an output_blob of shape (batch_size, 25200, 85) (Num classes + bbox[x,y,w,h] + confidence_[c])
            // yolov8 has an output_blob of shape (batch_size, 84,  8400) (Num classes + bbox[x,y,w,h])
            return output_blob.size[2] > output_blob.size[1] ? Yolov8 : Yolov5;
        }

        C10_ALWAYS_INLINE YoloVersion _deduce_yolo_version(const at::Tensor &output) {
            return output.size(-1) > output.size(-2) ? Yolov8 : Yolov5;
        }
    }

    YoloBase::YoloBase(YoloOptions options)
            : options_(options) {}

    void YoloBase::load_classes(const std::string &classes_filepath) {
        std::ifstream inputFile(classes_filepath);
        if (inputFile.is_open()) {
            classes_.clear();
            std::string line;
            while (std::getline(inputFile, line))
                classes_.push_back(line);
            inputFile.close();
        }
    }

    YoloVersion YoloBase::version() const noexcept {
        return version_;
    }

    YoloOptions YoloBase::options() const noexcept {
        return options_;
    }

    int YoloBase::num_classes() const noexcept {
        return classes_.size();
    }

    at::ArrayRef<std::string> YoloBase::classes_names() const noexcept {
        return classes_;
    }

    std::string_view YoloBase::classes_name(int class_id) const {
        return classes_.at(class_id);
    }

    // ---------------------
    // OpenCV
    // ---------------------
    YoloOpenCV::Yolo(YoloOptions options)
            : YoloBase(options) {}

    YoloOpenCV::Yolo(const std::string &model_filepath,
                     YoloOptions options,
                     int backend,
                     int target)
            : YoloBase(options) {
        load_onnx(model_filepath);
        to(backend, target);
        num_classes_ = static_cast<int>(classes_.size());
    }

    YoloOpenCV::Yolo(
            const std::string &model_filepath,
            const std::string &classes_filepath,
            YoloOptions options,
            int backend,
            int target)
            : Yolo(model_filepath, options, backend, target) {
        load_classes(classes_filepath);
    }

    YoloOpenCV::Yolo(const std::string &model_filepath,
                     int num_classes,
                     YoloOptions options,
                     int backend,
                     int target)
            : Yolo(model_filepath, options, backend, target) {
        num_classes_ = num_classes;
    }

    void YoloOpenCV::load_onnx(const std::string &filename) {
        OpenCVModule::load_onnx(filename);
        version_ = Yolo_UNKNOWN;
    }

    void YoloOpenCV::load_classes(const std::string &classes_filepath) {
        YoloBase::load_classes(classes_filepath);
        num_classes_ = static_cast<int>(classes_.size());
    }

    int YoloOpenCV::num_classes() const noexcept {
        return num_classes_;
    }

    std::vector<Detection> YoloOpenCV::forward(const cv::Mat &input) {
        cv::Mat scaled_input = input;
        int max_dim = std::max(input.cols, input.rows);
        scaled_input = transforms::functional::letterbox(
                scaled_input, cv::Size(max_dim, max_dim), options_.align_center(), cv::Scalar(117, 117, 117));
        auto input_blob = transforms::functional::to_blob(
                scaled_input, options_.input_shape(), 1. / 255., cv::Scalar(), false);
        net.setInput(input_blob);

        std::vector<cv::Mat> output_blobs(1);
        net.forward(output_blobs, net.getUnconnectedOutLayersNames());
        if (version_ == Yolo_UNKNOWN)
            version_ = _deduce_yolo_version(output_blobs[0]);
        bool is_yolov8 = version_ == Yolov8;

        int ndets, dimensions;
        if (is_yolov8) {  // Check if the shape[2] is more than shape[1] (is_yolov8)
            ndets = output_blobs[0].size[2];
            dimensions = output_blobs[0].size[1];

            output_blobs[0] = output_blobs[0].reshape(1, dimensions);
            cv::transpose(output_blobs[0], output_blobs[0]);
        } else {
            ndets = output_blobs[0].size[1];
            dimensions = output_blobs[0].size[2];
        }
        auto data_ptr = (float *) output_blobs[0].data;

        std::vector<int> class_ids;
        std::vector<float> confidences;
        std::vector<cv::Rect2d> bboxes;
        for (int i = 0; i < ndets; ++i) {
            if (is_yolov8) {
                auto classes_scores_ptr = data_ptr + 4;

                cv::Mat scores(1, num_classes_, CV_32FC1, classes_scores_ptr);
                cv::Point class_id;
                double max_class_score;
                cv::minMaxLoc(scores, nullptr, &max_class_score, nullptr, &class_id);
                if (max_class_score > options_.score_threshold()) {
                    float x_c = data_ptr[0], y_c = data_ptr[1], w = data_ptr[2], h = data_ptr[3];

                    confidences.push_back((float) max_class_score);
                    class_ids.push_back(class_id.x);
                    bboxes.emplace_back(x_c - 0.5 * w, y_c - 0.5 * h, w, h);
                }
            } else {  // yolov5
                float confidence = data_ptr[4];

                if (confidence >= options_.confidence_threshold()) {
                    auto classes_scores_ptr = data_ptr + 5;

                    cv::Mat scores(1, num_classes_, CV_32FC1, classes_scores_ptr);
                    cv::Point class_id;
                    double max_class_score;
                    minMaxLoc(scores, nullptr, &max_class_score, nullptr, &class_id);
                    if (max_class_score > options_.score_threshold()) {
                        float x_c = data_ptr[0], y_c = data_ptr[1], w = data_ptr[2], h = data_ptr[3];

                        confidences.push_back(confidence);
                        class_ids.push_back(class_id.x);
                        bboxes.emplace_back(x_c - 0.5 * w, y_c - 0.5 * h, w, h);
                    }
                }
            }
            data_ptr += dimensions;
        }

        std::vector<int> keep_indices;
        cv::dnn::NMSBoxes(bboxes, confidences,
                          is_yolov8 ? options_.score_threshold() : options_.confidence_threshold(),
                          options_.nms_threshold(), keep_indices);

        std::vector<Detection> detections;
        for (auto idx: keep_indices) {
            Detection det;
            det.label_id = class_ids[idx];
            det.label = classes_name(det.label_id);
            det.confidence = confidences[idx];
            det.bbox = bboxes[idx];

            detections.push_back(det);
        }
        transforms::functional::rescale_bboxes_(detections, input.size(), options_.input_shape(),
                                                options_.align_center());
        return detections;
    }

    // ---------------------
    // LibTorch
    // ---------------------
    YoloLibTorch::Yolo(YoloOptions options)
            : YoloBase(options) {}

    YoloLibTorch::Yolo(const std::string &model_filepath,
                       YoloOptions options,
                       c10::optional<at::Device> device)
            : YoloBase(options) {
        load(model_filepath, device);
    }

    YoloLibTorch::Yolo(const std::string &model_filepath,
                       const std::string &classes_filepath,
                       YoloOptions options,
                       c10::optional<at::Device> device)
            : YoloBase(options) {
        load(model_filepath, device);
        load_classes(classes_filepath);
    }

    void YoloLibTorch::load(const std::string &filename, c10::optional<at::Device> device) {
        LibTorchModule::load(filename, device);
        version_ = Yolo_UNKNOWN;
    }

    at::Tensor YoloLibTorch::forward(const at::Tensor &input) {
        TORCH_CHECK_VALUE(input.size(-2) == options_.input_height() && input.size(-1) == options_.input_width(),
                          "input must has spatial size of ",
                          options_.input_shape(),
                          ". Got input.sizes()=",
                          input.sizes())
        // only inference, no preprocessing nor postprocessing
        std::vector<torch::jit::IValue> inputs{input};
        auto prediction = net.forward(inputs).toTensor();
        if (version_ == Yolo_UNKNOWN)
            version_ = _deduce_yolo_version(prediction);
        return prediction;
    }

    std::vector<Detection> YoloLibTorch::forward(const cv::Mat &input) {
        auto scaled_input = transforms::functional::letterbox(
                input, options_.input_shape(), options_.align_center(), cv::Scalar(117, 117, 117));

        auto input_tensor = at::from_blob(
                scaled_input.data, {scaled_input.rows, scaled_input.cols, scaled_input.channels()},
                at::TensorOptions(at::kByte));
        input_tensor = input_tensor.to(device_, dtype_).div_(255);
        input_tensor = input_tensor.permute({2, 0, 1}).unsqueeze_(0);
        std::vector<torch::jit::IValue> inputs{input_tensor};

        // inference
        auto prediction = net.forward(inputs).toTensor().cpu();
        if (version_ == Yolo_UNKNOWN)
            version_ = _deduce_yolo_version(prediction);
        TORCH_CHECK_NOT_IMPLEMENTED(version_ == Yolov8,
                                    "Post-processing for Yolov5 LibTorch is not implemented.")

        // nms
        prediction = ops::non_max_suppression(
                prediction, version_ == Yolov8 ? options_.score_threshold() : options_.confidence_threshold(),
                options_.nms_threshold())[0];
        transforms::functional::rescale_bboxes_(
                prediction, input.size(), scaled_input.size(), options_.align_center());
        return transforms::functional::to_detection_list(prediction, classes_);
    }
}  // namespace ultralytics
