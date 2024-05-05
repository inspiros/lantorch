#include "transforms.h"

#include <opencv2/imgproc.hpp>

#include <c10/macros/Macros.h>

namespace ultralytics {
    namespace transforms {
        // ---------------------
        // Pre-processing
        // ---------------------
        namespace functional {
            namespace {
                C10_ALWAYS_INLINE double generate_scale(
                        const cv::Size &input_size,
                        const cv::Size &output_size) {
                    return std::min(static_cast<double>(output_size.height) / static_cast<double>(input_size.height),
                                    static_cast<double>(output_size.width) / static_cast<double>(input_size.width));
                }
            }

            cv::Mat letterbox(
                    const cv::Mat &input,
                    const cv::Size &output_size,
                    bool align_center,
                    const cv::Scalar &value,
                    int interpolation,
                    bool copy) {
                if (input.rows == output_size.width && input.cols == output_size.height)
                    return copy ? input.clone() : input;
                auto resize_scale = generate_scale(input.size(), output_size);
                int new_shape_w = std::round(input.cols * resize_scale);
                int new_shape_h = std::round(input.rows * resize_scale);

                int top, right, bottom, left;
                if (align_center) {
                    double pad_w = (output_size.width - new_shape_w) / 2.;
                    double pad_h = (output_size.height - new_shape_h) / 2.;
                    top = std::round(pad_h - 0.1);
                    bottom = std::round(pad_h + 0.1);
                    left = std::round(pad_w - 0.1);
                    right = std::round(pad_w + 0.1);
                } else {
                    top = left = 0;
                    bottom = output_size.height - input.rows;
                    right = output_size.width - input.cols;
                }

                cv::Mat output;
                cv::resize(input, output,
                           cv::Size(new_shape_w, new_shape_h),
                           0, 0, interpolation);
                cv::copyMakeBorder(output, output, top, bottom, left, right,
                                   cv::BORDER_CONSTANT, value);
                return output;
            }
        }  // namespace functional

        // ---------------------
        // Post-processing
        // ---------------------
        namespace functional {
            at::Tensor rescale_bboxes(
                    const at::Tensor &prediction,
                    const cv::Size &input_size,
                    const cv::Size &letterbox_size,
                    bool align_center) {
                auto rescaled_prediction = at::empty_like(prediction);
                auto gain = generate_scale(input_size, letterbox_size);
                if (align_center) {
                    auto pad_x = std::round((double) (letterbox_size.width - input_size.width * gain) / 2. - 0.1);
                    auto pad_y = std::round((double) (letterbox_size.height - input_size.height * gain) / 2. - 0.1);
                    rescaled_prediction.index_put_({at::indexing::Ellipsis, indexing::BboxXSlice},
                                               prediction.index({at::indexing::Ellipsis, indexing::BboxXSlice}) - pad_x);
                    rescaled_prediction.index_put_({at::indexing::Ellipsis, indexing::BboxYSlice},
                                               prediction.index({at::indexing::Ellipsis, indexing::BboxYSlice}) - pad_y);
                }
                rescaled_prediction.index_put_(
                        {at::indexing::Ellipsis, indexing::BboxXYSlice},
                        prediction.index({at::indexing::Ellipsis, indexing::BboxXYSlice}).div(gain));
                rescaled_prediction.index_put_(
                        {at::indexing::Ellipsis, at::indexing::Slice(4, at::indexing::None)},
                        prediction.index({at::indexing::Ellipsis, at::indexing::Slice(4, at::indexing::None)}));
                return rescaled_prediction;
            }

            at::Tensor &rescale_bboxes_(
                    at::Tensor &prediction,
                    const cv::Size &input_size,
                    const cv::Size &letterbox_size,
                    bool align_center) {
                auto gain = generate_scale(input_size, letterbox_size);
                if (align_center) {
                    auto pad_x = std::round((double) (letterbox_size.width - input_size.width * gain) / 2. - 0.1);
                    auto pad_y = std::round((double) (letterbox_size.height - input_size.height * gain) / 2. - 0.1);
                    prediction.index({at::indexing::Ellipsis, indexing::BboxXSlice}).add_(-pad_x);
                    prediction.index({at::indexing::Ellipsis, indexing::BboxYSlice}).add_(-pad_y);
                }
                prediction.index({at::indexing::Ellipsis, indexing::BboxXYSlice}).div_(gain);
                return prediction;
            }

            std::vector<cv::Rect2d> rescale_bboxes(
                    at::ArrayRef<cv::Rect2d> bboxes,
                    const cv::Size &input_size,
                    const cv::Size &letterbox_size,
                    bool align_center) {
                std::vector<cv::Rect2d> rescaled_bboxes;
                rescaled_bboxes.reserve(bboxes.size());
                auto gain = generate_scale(input_size, letterbox_size);
                auto pad_x = align_center ?
                             std::round((double) (letterbox_size.width - input_size.width * gain) / 2. - 0.1) : 0;
                auto pad_y = align_center ?
                             std::round((double) (letterbox_size.height - input_size.height * gain) / 2. - 0.1) : 0;

                for (const auto &bbox: bboxes) {
                    cv::Rect2d rescaled_bbox = cv::Rect2d((bbox.x - pad_x) / gain, (bbox.y - pad_y) / gain,
                                                          bbox.width / gain, bbox.height / gain);
                    rescaled_bboxes.push_back(rescaled_bbox);
                }
                return rescaled_bboxes;
            }

            std::vector<cv::Rect2d> &rescale_bboxes_(
                    std::vector<cv::Rect2d> &bboxes,
                    const cv::Size &input_size,
                    const cv::Size &letterbox_size,
                    bool align_center) {
                auto gain = generate_scale(input_size, letterbox_size);
                auto pad_x = align_center ?
                             std::round((double) (letterbox_size.width - input_size.width * gain) / 2. - 0.1) : 0;
                auto pad_y = align_center ?
                             std::round((double) (letterbox_size.height - input_size.height * gain) / 2. - 0.1) : 0;

                for (auto &bbox: bboxes) {
                    bbox.x = (bbox.x - pad_x) / gain;
                    bbox.y = (bbox.y - pad_y) / gain;
                    bbox.width /= gain;
                    bbox.height /= gain;
                }
                return bboxes;
            }

            std::vector<Detection> rescale_bboxes(
                    at::ArrayRef<Detection> detections,
                    const cv::Size &input_size,
                    const cv::Size &letterbox_size,
                    bool align_center) {
                std::vector<Detection> rescaled_detections;
                rescaled_detections.reserve(detections.size());
                auto gain = generate_scale(input_size, letterbox_size);
                auto pad_x = align_center ?
                             std::round((double) (letterbox_size.width - input_size.width * gain) / 2. - 0.1) : 0;
                auto pad_y = align_center ?
                             std::round((double) (letterbox_size.height - input_size.height * gain) / 2. - 0.1) : 0;

                for (const auto &det: detections) {
                    Detection rescaled_det = det;
                    rescaled_det.bbox = cv::Rect2d((det.bbox.x - pad_x) / gain, (det.bbox.y - pad_y) / gain,
                                                   det.bbox.width / gain, det.bbox.height / gain);
                    rescaled_detections.push_back(rescaled_det);
                }
                return rescaled_detections;
            }

            std::vector<Detection> &rescale_bboxes_(
                    std::vector<Detection> &detections,
                    const cv::Size &input_size,
                    const cv::Size &letterbox_size,
                    bool align_center) {
                auto gain = generate_scale(input_size, letterbox_size);
                auto pad_x = align_center ?
                             std::round((double) (letterbox_size.width - input_size.width * gain) / 2. - 0.1) : 0;
                auto pad_y = align_center ?
                             std::round((double) (letterbox_size.height - input_size.height * gain) / 2. - 0.1) : 0;

                for (auto &det: detections) {
                    det.bbox.x = (det.bbox.x - pad_x) / gain;
                    det.bbox.y = (det.bbox.y - pad_y) / gain;
                    det.bbox.width /= gain;
                    det.bbox.height /= gain;
                }
                return detections;
            }

            std::vector<Detection> to_detection_list(
                    const at::Tensor &prediction) {
                bool batched = prediction.ndimension() == 3;
                TORCH_CHECK_VALUE(prediction.ndimension() == 2 || (batched && prediction.size(0) == 1),
                                  "prediction must be 2D or single-batched 3D prediction. "
                                  "Got prediction.ndimension=",
                                  prediction.ndimension())
                auto prediction_c = prediction.cpu();
                if (batched)
                    prediction_c.squeeze_(0);

                std::vector<Detection> detections;
                detections.reserve(prediction_c.size(0));
                auto tensor_accessor = prediction_c.accessor<float, 2>();
                for (int idx = 0; idx < prediction_c.size(0); idx++) {
                    Detection det;
                    double x1 = tensor_accessor[idx][0];
                    double y1 = tensor_accessor[idx][1];
                    double x2 = tensor_accessor[idx][2];
                    double y2 = tensor_accessor[idx][3];
                    det.label_id = static_cast<int>(tensor_accessor[idx][5]);
                    det.confidence = tensor_accessor[idx][4];
                    det.bbox = cv::Rect2d(x1, y1, x2 - x1, y2 - y1);

                    detections.push_back(det);
                }
                return detections;
            }

            std::vector<Detection> to_detection_list(
                    const at::Tensor &prediction,
                    at::ArrayRef<std::string> class_names) {
                bool batched = prediction.ndimension() == 3;
                TORCH_CHECK_VALUE(prediction.ndimension() == 2 || (batched && prediction.size(0) == 1),
                                  "prediction must be 2D or single-batched 3D prediction. "
                                  "Got prediction.ndimension=",
                                  prediction.ndimension())
                auto prediction_c = prediction.cpu();
                if (batched)
                    prediction_c.squeeze_(0);

                std::vector<Detection> detections;
                detections.reserve(prediction_c.size(0));
                auto tensor_accessor = prediction_c.accessor<float, 2>();
                for (int idx = 0; idx < prediction_c.size(0); idx++) {
                    Detection det;
                    double x1 = tensor_accessor[idx][0];
                    double y1 = tensor_accessor[idx][1];
                    double x2 = tensor_accessor[idx][2];
                    double y2 = tensor_accessor[idx][3];
                    det.label_id = static_cast<int>(tensor_accessor[idx][5]);
                    det.label = class_names.at(det.label_id);
                    det.confidence = tensor_accessor[idx][4];
                    det.bbox = cv::Rect2d(x1, y1, x2 - x1, y2 - y1);

                    detections.push_back(det);
                }
                return detections;
            }
        }
    }
}
