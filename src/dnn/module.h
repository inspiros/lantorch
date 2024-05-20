#pragma once

#include "inference_engine.h"

#include <istream>

#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <utility>

#undef slots
#include <torch/jit.h>
#define slots Q_SLOTS

/// Neural network module with different inference engines.
/// These classes are not intended to be used directly but
/// rather as base classes.
template<InferenceEngine engine>
class Module {
    // Not implemented
};

template<>
class Module<INFERENCE_ENGINE_OpenCV> {
protected:
    cv::dnn::Net net;

public:
    inline void load_onnx(const std::string &filename) {
        net = cv::dnn::readNetFromONNX(filename);
    }

    inline void load_torch(const std::string &filename) {
        net = cv::dnn::readNetFromTorch(filename);
    }

    inline void load_caffe(const std::string &filename) {
        net = cv::dnn::readNetFromCaffe(filename);
    }

    inline void load_darknet(const std::string &filename) {
        net = cv::dnn::readNetFromDarknet(filename);
    }

    inline void load_tflite(const std::string &filename) {
        net = cv::dnn::readNetFromTFLite(filename);
    }

    inline void load_tensorflow(const std::string &filename) {
        net = cv::dnn::readNetFromTensorflow(filename);
    }

    inline void to(int backend = -1, int target = -1) {
        if (backend >= 0)
            net.setPreferableBackend(backend);
        if (target >= 0)
            net.setPreferableTarget(target);
    }

    inline void cpu() {
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }

    inline void cuda() {
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    }
};

template<>
class Module<INFERENCE_ENGINE_LibTorch> {
protected:
    torch::jit::Module net;
    at::ScalarType dtype_ = at::kFloat;
    at::Device device_ = at::kCPU;

public:
    inline void load(const std::string &filename,
                     c10::optional<at::Device> device = c10::nullopt) {
        net = torch::jit::load(filename, device);
        device_ = device.value_or(at::kCPU);
    }

    inline void load(std::istream &in,
                     c10::optional<at::Device> device = c10::nullopt) {
        net = torch::jit::load(in, device);
        device_ = device.value_or(at::kCPU);
    }

    inline void to(at::Device device, at::ScalarType dtype, bool non_blocking = false) {
        net.to(device, dtype, non_blocking);
        device_ = device;
        dtype_ = dtype;
    }

    inline void to(at::ScalarType dtype, bool non_blocking = false) {
        net.to(dtype, non_blocking);
        this->dtype_ = dtype;
    }

    inline void to(at::Device device, bool non_blocking = false) {
        net.to(device, non_blocking);
        device_ = device;
    }

    inline void cpu(bool non_blocking = false) {
        net.to(at::kCPU, non_blocking);
        device_ = at::kCPU;
    }

    inline void cuda(bool non_blocking = false) {
        net.to(at::kCUDA, non_blocking);
        device_ = at::kCUDA;
    }

    inline void train(bool on = true) {
        net.train(on);
    }

    inline void eval() {
        net.eval();
    }

    inline bool is_training() const {
        return net.is_training();
    }

    inline at::Device device() const {
        return device_;
    }

    inline at::ScalarType dtype() const {
        return dtype_;
    }
};

using OpenCVModule = Module<INFERENCE_ENGINE_OpenCV>;
using LibTorchModule = Module<INFERENCE_ENGINE_LibTorch>;
using OnnxModule = Module<INFERENCE_ENGINE_Onnxruntime>;
using TensorRTModule = Module<INFERENCE_ENGINE_TensorRT>;
