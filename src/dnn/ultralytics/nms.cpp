#include "nms.h"
#include "indexing.h"

namespace ultralytics {
    namespace ops {
        namespace {
            C10_ALWAYS_INLINE at::Tensor xyxy2xywh(const at::Tensor &xyxy) {
                auto xywh = at::empty_like(xyxy);
                xywh.index_put_(
                        {at::indexing::Ellipsis, 0},
                        (xyxy.index({at::indexing::Ellipsis, 0}) + xyxy.index({at::indexing::Ellipsis, 2})).div_(2));
                xywh.index_put_(
                        {at::indexing::Ellipsis, 1},
                        (xyxy.index({at::indexing::Ellipsis, 1}) + xyxy.index({at::indexing::Ellipsis, 3})).div_(2));
                xywh.index_put_({at::indexing::Ellipsis, 2},
                                xyxy.index({at::indexing::Ellipsis, 2}) - xyxy.index({at::indexing::Ellipsis, 0}));
                xywh.index_put_({at::indexing::Ellipsis, 3},
                                xyxy.index({at::indexing::Ellipsis, 3}) - xyxy.index({at::indexing::Ellipsis, 1}));
                xywh.index_put_({at::indexing::Ellipsis, indexing::BboxXYSlice},
                                xyxy.index({at::indexing::Ellipsis, indexing::BboxXYSlice}));
                return xywh;
            }

            C10_ALWAYS_INLINE at::Tensor &xyxy2xywh_(at::Tensor &pred) {
                auto width = pred.index({at::indexing::Ellipsis, 2}) - pred.index({at::indexing::Ellipsis, 0});
                auto height = pred.index({at::indexing::Ellipsis, 3}) - pred.index({at::indexing::Ellipsis, 1});
                pred.index_put_(
                        {at::indexing::Ellipsis, 0},
                        (pred.index({at::indexing::Ellipsis, 0}) + pred.index({at::indexing::Ellipsis, 2})).div_(2));
                pred.index_put_(
                        {at::indexing::Ellipsis, 1},
                        (pred.index({at::indexing::Ellipsis, 1}) + pred.index({at::indexing::Ellipsis, 3})).div_(2));
                pred.index_put_({at::indexing::Ellipsis, 2}, width);
                pred.index_put_({at::indexing::Ellipsis, 3}, height);
                return pred;
            }

            C10_ALWAYS_INLINE at::Tensor xywh2xyxy(const at::Tensor &xywh) {
                auto xyxy = at::empty_like(xywh);
                auto dw = xywh.index({at::indexing::Ellipsis, 2}).div(2);
                auto dh = xywh.index({at::indexing::Ellipsis, 3}).div(2);
                xyxy.index_put_({at::indexing::Ellipsis, 0}, xywh.index({at::indexing::Ellipsis, 0}) - dw);
                xyxy.index_put_({at::indexing::Ellipsis, 1}, xywh.index({at::indexing::Ellipsis, 1}) - dh);
                xyxy.index_put_({at::indexing::Ellipsis, 2}, xywh.index({at::indexing::Ellipsis, 0}) + dw);
                xyxy.index_put_({at::indexing::Ellipsis, 3}, xywh.index({at::indexing::Ellipsis, 1}) + dh);
                xyxy.index_put_({at::indexing::Ellipsis, indexing::BboxXYSlice},
                                xywh.index({at::indexing::Ellipsis, indexing::BboxXYSlice}));
                return xyxy;
            }

            C10_ALWAYS_INLINE at::Tensor &xywh2xyxy_(at::Tensor &pred) {
                auto dw = pred.index({at::indexing::Ellipsis, 2}).div(2);
                auto dh = pred.index({at::indexing::Ellipsis, 3}).div(2);
                pred.index_put_({at::indexing::Ellipsis, 2}, pred.index({at::indexing::Ellipsis, 0}) + dw);
                pred.index_put_({at::indexing::Ellipsis, 3}, pred.index({at::indexing::Ellipsis, 1}) + dh);
                pred.index({at::indexing::Ellipsis, 0}).add_(-dw);
                pred.index({at::indexing::Ellipsis, 1}).add_(-dh);
                return pred;
            }

            template<typename scalar_t>
            C10_ALWAYS_INLINE int64_t nms_kernel(int64_t ndets,
                                                 uint8_t *suppressed,
                                                 int64_t *keep,
                                                 const int64_t *order,
                                                 scalar_t *x1,
                                                 scalar_t *y1,
                                                 scalar_t *x2,
                                                 scalar_t *y2,
                                                 scalar_t *areas,
                                                 scalar_t iou_threshold) {
                int64_t num_to_keep = 0;

                for (int64_t _i = 0; _i < ndets; _i++) {
                    auto i = order[_i];
                    if (suppressed[i] == 1)
                        continue;
                    keep[num_to_keep++] = i;
                    scalar_t ix1 = x1[i];
                    scalar_t iy1 = y1[i];
                    scalar_t ix2 = x2[i];
                    scalar_t iy2 = y2[i];
                    scalar_t iarea = areas[i];

                    for (int64_t _j = _i + 1; _j < ndets; _j++) {
                        auto j = order[_j];
                        if (suppressed[j] == 1)
                            continue;
                        scalar_t xx1 = std::max(ix1, x1[j]);
                        scalar_t yy1 = std::max(iy1, y1[j]);
                        scalar_t xx2 = std::min(ix2, x2[j]);
                        scalar_t yy2 = std::min(iy2, y2[j]);

                        scalar_t w = std::max(static_cast<scalar_t>(0), xx2 - xx1);
                        scalar_t h = std::max(static_cast<scalar_t>(0), yy2 - yy1);
                        scalar_t inter = w * h;
                        scalar_t ovr = inter / (iarea + areas[j] - inter);
                        if (ovr > iou_threshold)
                            suppressed[j] = 1;
                    }
                }
                return num_to_keep;
            }
        }

        // Reference: https://github.com/pytorch/vision/blob/main/torchvision/csrc/ops/cpu/nms_kernel.cpp
        at::Tensor nms(
                const at::Tensor &bboxes,
                const at::Tensor &scores,
                double iou_threshold) {
            if (bboxes.numel() == 0)
                return at::empty({0}, bboxes.options().dtype(at::kLong));

            auto x1_t = bboxes.select(1, 0).contiguous();
            auto y1_t = bboxes.select(1, 1).contiguous();
            auto x2_t = bboxes.select(1, 2).contiguous();
            auto y2_t = bboxes.select(1, 3).contiguous();

            at::Tensor areas_t = (x2_t - x1_t) * (y2_t - y1_t);

            auto order_t = std::get<1>(
                    scores.sort(/*stable=*/true, /*dim=*/0, /* descending=*/true));

            auto ndets = bboxes.size(0);
            at::Tensor suppressed_t = at::zeros({ndets}, bboxes.options().dtype(at::kByte));
            at::Tensor keep_t = at::zeros({ndets}, bboxes.options().dtype(at::kLong));

            int64_t num_to_keep;
            AT_DISPATCH_FLOATING_TYPES_AND2(at::kHalf, at::kBFloat16, bboxes.scalar_type(), "nms", [&]() {
                num_to_keep = nms_kernel(ndets,
                                         suppressed_t.data_ptr<uint8_t>(),
                                         keep_t.data_ptr<int64_t>(),
                                         order_t.data_ptr<int64_t>(),
                                         x1_t.data_ptr<scalar_t>(),
                                         y1_t.data_ptr<scalar_t>(),
                                         x2_t.data_ptr<scalar_t>(),
                                         y2_t.data_ptr<scalar_t>(),
                                         areas_t.data_ptr<scalar_t>(),
                                         static_cast<scalar_t>(iou_threshold));
            });
            return keep_t.narrow(0, 0, num_to_keep);
        }

        at::Tensor non_max_suppression(
                const at::Tensor &prediction,
                double conf_threshold,
                double iou_threshold,
                int64_t max_det) {
            auto bs = prediction.size(0);
            auto nc = prediction.size(1) - 4;
            auto nm = prediction.size(1) - nc - 4;
            auto mi = 4 + nc;
            auto xc = prediction.index({at::indexing::Slice(), at::indexing::Slice(4, mi)}).amax(1) > conf_threshold;

            auto output = prediction.transpose(-1, -2);
            xywh2xyxy_(output);

            std::vector<at::Tensor> outputs;
            outputs.reserve(bs);
            for (int i = 0; i < bs; i++) {
                outputs.push_back(at::zeros({0, 6 + nm}, output.options()));
            }

            for (int xi = 0; xi < output.size(0); xi++) {
                auto x = output[xi];
                x = x.index({xc[xi]});
                auto x_split = x.split({4, nc, nm}, 1);
                auto box = x_split[0], cls = x_split[1], mask = x_split[2];
                auto [conf, j] = cls.max(1, true);
                x = at::cat({box, conf, j.toType(prediction.scalar_type()), mask}, 1);
                x = x.index({conf.view(-1) > conf_threshold});
                int n = x.size(0);
                if (!n) { continue; }

                // NMS
                auto c = x.index({at::indexing::Slice(), at::indexing::Slice{5, 6}}) * 7680;
                auto boxes = x.index({at::indexing::Slice(), indexing::BboxXYSlice}) + c;
                auto scores = x.index({at::indexing::Slice(), 4});
                auto i = nms(boxes, scores, iou_threshold);
                i = i.index({at::indexing::Slice(at::indexing::None, max_det)});
                outputs[xi] = x.index({i});
            }
            return at::stack(outputs);
        }
    }
}
