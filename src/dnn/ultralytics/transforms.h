#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn/dnn.hpp>

#undef slots

#include <ATen/ATen.h>

#define slots Q_SLOTS

#include "indexing.h"
#include "../return_types.h"

namespace ultralytics {
    namespace transforms {
        // ---------------------
        // Pre-processing
        // ---------------------
        namespace functional {
            cv::Mat letterbox(
                    const cv::Mat &input,
                    const cv::Size &output_size,
                    bool align_center = true,
                    const cv::Scalar &value = 0,
                    int interpolation = cv::INTER_AREA,
                    bool copy = false);
        }

        class [[maybe_unused]] LetterBox {
            cv::Size output_size;
            bool align_center = true;
            cv::Scalar value = 0;
            bool copy = false;

        public:
            explicit LetterBox(const cv::Size &output_size,
                               bool align_center = true,
                               const cv::Scalar &value = 0,
                               bool copy = false)
                    : output_size(output_size), align_center(align_center), value(value), copy(copy) {}

            inline cv::Mat forward(const cv::Mat &input) const {
                return functional::letterbox(input, output_size, align_center, value, copy);
            }

            inline cv::Mat operator()(const cv::Mat &input) const {
                return forward(input);
            }
        };

        namespace functional {
            C10_ALWAYS_INLINE cv::Mat to_blob(
                    const cv::Mat &input,
                    const cv::Size &output_size,
                    double scale_factor = 1. / 255.,
                    const cv::Scalar &mean = {},
                    bool swapRB = false,
                    bool crop = false) {
                cv::Mat output;
                cv::dnn::blobFromImage(input, output, scale_factor, output_size, mean, swapRB, crop);
                return output;
            }
        }  // namespace functional

        class [[maybe_unused]] ToBlob {
            cv::Size output_size;
            double scale_factor = 1. / 255.;
            cv::Scalar mean = {};
            bool swapRB = false;
            bool crop = false;

        public:
            explicit ToBlob(const cv::Size &output_size,
                            double scale_factor = 1. / 255.,
                            const cv::Scalar &mean = {},
                            bool swapRB = false,
                            bool crop = false)
                    : output_size(output_size), scale_factor(scale_factor), mean(mean), swapRB(swapRB), crop(crop) {}

            inline cv::Mat forward(const cv::Mat &input) const {
                return functional::to_blob(input, output_size, scale_factor, mean, swapRB, crop);
            }

            inline cv::Mat operator()(const cv::Mat &input) const {
                return forward(input);
            }
        };

        // ---------------------
        // Post-processing
        // ---------------------
        namespace functional {
            at::Tensor rescale_bboxes(
                    const at::Tensor &prediction,
                    const cv::Size &input_size,
                    const cv::Size &letterbox_size,
                    bool align_center = true);

            at::Tensor &rescale_bboxes_(
                    at::Tensor &prediction,
                    const cv::Size &input_size,
                    const cv::Size &letterbox_size,
                    bool align_center = true);

            std::vector<cv::Rect2d> rescale_bboxes(
                    at::ArrayRef<cv::Rect2d> bboxes,
                    const cv::Size &input_size,
                    const cv::Size &letterbox_size,
                    bool align_center = true);

            std::vector<cv::Rect2d> &rescale_bboxes_(
                    std::vector<cv::Rect2d> &bboxes,
                    const cv::Size &input_size,
                    const cv::Size &letterbox_size,
                    bool align_center = true);

            std::vector<Detection> rescale_bboxes(
                    at::ArrayRef<Detection> detections,
                    const cv::Size &input_size,
                    const cv::Size &letterbox_size,
                    bool align_center = true);

            std::vector<Detection> &rescale_bboxes_(
                    std::vector<Detection> &detections,
                    const cv::Size &input_size,
                    const cv::Size &letterbox_size,
                    bool align_center = true);

            std::vector<Detection> to_detection_list(
                    const at::Tensor &prediction);

            std::vector<Detection> to_detection_list(
                    const at::Tensor &prediction,
                    at::ArrayRef<std::string> class_names);
        }  // namespace functional
    }
}
