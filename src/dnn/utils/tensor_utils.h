#pragma once

#undef slots
#include <ATen/ATen.h>
#define slots Q_SLOTS

#include <QImage>

namespace at {
    /**
     * Convert a QImage to Tensor.
     * Warning: This function is not optimized
     * Ref: https://www.cnblogs.com/cheungxiongwei/p/11836869.html
     *
     * @param q_img
     * @param options
     * @param non_blocking
     * @param copy
     * @param memory_format
     * @return
     */
    at::Tensor from_qimage(const QImage &q_img,
                           at::TensorOptions options = {},
                           bool non_blocking = false,
                           bool copy = false,
                           c10::optional<at::MemoryFormat> memory_format = c10::nullopt) {
        int width = q_img.width();
        int height = q_img.height();
        int channels = q_img.depth() / 8;

        // create tensor
        auto tensor = at::empty({1, height, width, channels},
                                at::TensorOptions(at::kByte));
        auto tensor_accessor = tensor.accessor<uint8_t, 4>();
        bool isOk = false;

        // conversion step must take place in cpu
        if (channels == 1) {
#pragma omp simd
            for (int w = 0; w < width; ++w) {
                for (int h = 0; h < height; ++h) {
                    auto pixel = q_img.pixel(w, h);
                    tensor_accessor[0][h][w][0] = qGray(pixel);
                }
            }
        } else if (channels == 3) {
#pragma omp simd
            for (int w = 0; w < width; ++w) {
                for (int h = 0; h < height; ++h) {
                    auto pixel = q_img.pixel(w, h);
                    tensor_accessor[0][h][w][0] = qRed(pixel);
                    tensor_accessor[0][h][w][1] = qGreen(pixel);
                    tensor_accessor[0][h][w][2] = qBlue(pixel);
                }
            }
        } else if (channels == 4) {
#pragma omp simd
            for (int w = 0; w < width; ++w) {
                for (int h = 0; h < height; ++h) {
                    auto pixel = q_img.pixel(w, h);
                    tensor_accessor[0][h][w][0] = qRed(pixel);
                    tensor_accessor[0][h][w][1] = qGreen(pixel);
                    tensor_accessor[0][h][w][2] = qBlue(pixel);
                    tensor_accessor[0][h][w][3] = qAlpha(pixel);
                }
            }
        } else {
            TORCH_CHECK_VALUE(false,
                              "Can only convert QImage with channels of 1 | 3 | 4. "
                              "Got channels=",
                              channels)
        }
        return tensor.to(options, non_blocking, copy, memory_format);
    }
}
