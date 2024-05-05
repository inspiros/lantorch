#pragma once

#undef slots
#include <ATen/ATen.h>
#define slots Q_SLOTS

namespace ultralytics {
    namespace ops {
        at::Tensor nms(
                const at::Tensor &bboxes,
                const at::Tensor &scores,
                double iou_threshold);

        at::Tensor non_max_suppression(
                const at::Tensor &prediction,
                double conf_threshold = 0.25,
                double iou_threshold = 0.45,
                int64_t max_det = 100);
    }
}
