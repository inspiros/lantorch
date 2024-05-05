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

            auto suppressed = suppressed_t.data_ptr<uint8_t>();
            auto keep = keep_t.data_ptr<int64_t>();
            auto order = order_t.data_ptr<int64_t>();
            auto x1 = x1_t.data_ptr<float>();
            auto y1 = y1_t.data_ptr<float>();
            auto x2 = x2_t.data_ptr<float>();
            auto y2 = y2_t.data_ptr<float>();
            auto areas = areas_t.data_ptr<float>();

            int64_t num_to_keep = 0;

            for (int64_t _i = 0; _i < ndets; _i++) {
                auto i = order[_i];
                if (suppressed[i] == 1)
                    continue;
                keep[num_to_keep++] = i;
                auto ix1 = x1[i];
                auto iy1 = y1[i];
                auto ix2 = x2[i];
                auto iy2 = y2[i];
                auto iarea = areas[i];

                for (int64_t _j = _i + 1; _j < ndets; _j++) {
                    auto j = order[_j];
                    if (suppressed[j] == 1)
                        continue;
                    auto xx1 = std::max(ix1, x1[j]);
                    auto yy1 = std::max(iy1, y1[j]);
                    auto xx2 = std::min(ix2, x2[j]);
                    auto yy2 = std::min(iy2, y2[j]);

                    auto w = std::max(static_cast<float>(0), xx2 - xx1);
                    auto h = std::max(static_cast<float>(0), yy2 - yy1);
                    auto inter = w * h;
                    auto ovr = inter / (iarea + areas[j] - inter);
                    if (ovr > iou_threshold)
                        suppressed[j] = 1;
                }
            }
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
                x = at::cat({box, conf, j.toType(at::kFloat), mask}, 1);
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
