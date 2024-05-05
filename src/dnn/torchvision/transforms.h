/// This file contains C++ equivalences of the classes defined in torchvision.transforms.
/// Only a limited number of transforms are implemented for our use, porting the whole
/// library is not necessary for this project.
#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#ifdef WITH_PILLOW_RESIZE

#include <PillowResize/PillowResize.hpp>

#endif

#undef slots

#include <ATen/ATen.h>

#define slots Q_SLOTS

namespace torchvision {
    namespace transforms {
        // ---------------------
        // Compose
        // ---------------------
        namespace detail {
            template<typename op_Ts, typename T, std::size_t... Is>
            C10_ALWAYS_INLINE auto compose_impl(const op_Ts &ops, T &&self, std::index_sequence<Is...>) {
                if constexpr (sizeof...(Is) == 1)
                    return std::get<std::tuple_size_v<op_Ts> - 1>(ops).forward(self);
                else {
                    auto res = std::get<std::tuple_size_v<op_Ts> - sizeof...(Is)>(ops).forward(self);
                    return compose_impl(ops, std::forward<decltype(res)>(res),
                                        std::make_index_sequence<sizeof...(Is) - 1>());
                }
            }

            template<typename T, typename op_T, typename ... op_Ts>
            C10_ALWAYS_INLINE auto static_compose_impl(T &&self) {
                if constexpr (sizeof...(op_Ts) == 0)
                    return op_T::forward(self);
                else {
                    auto res = op_T::forward(self);
                    return static_compose_impl<decltype(res), op_Ts...>(std::forward<decltype(res)>(res));
                }
            }
        }  // namespace detail

/// \brief
/// Compose multiple sequential transformations.
///
/// See torchvision.transforms.Compose.
        template<typename... Ts>
        struct Compose {
            std::tuple<Ts...> ops;

            explicit Compose(const std::tuple<Ts...> &ops) : ops(ops) {}

            explicit Compose(Ts &&... ops) : ops(std::make_tuple(ops...)) {}

            template<typename T>
            inline auto forward(T &&input) const {
                return detail::compose_impl(ops, std::forward<T>(input),
                                            std::make_index_sequence<sizeof...(Ts)>());
            }

            template<typename T>
            inline auto operator()(T &&input) const {
                return forward(input);
            }
        };

/// \brief
/// Static variant of Compose.
        template<typename... Ts>
        struct StaticCompose {
            template<typename T>
            inline static auto forward(T &&input) {
                return detail::static_compose_impl<T, Ts...>(std::forward<T>(input));
            }

            template<typename T>
            inline auto operator()(T &&input) const {
                return forward(input);
            }
        };

        // ---------------------
        // Resize & ToTensor
        // ---------------------
/// Supported resize backends
        enum ResizeBackend {
            RESIZE_BACKEND_OpenCV = 0,
            RESIZE_BACKEND_Pillow = 1,
            RESIZE_BACKEND_Torch = 2,
        };

        // Pillow is used as the default backend if available to replicate
        // Python api. Otherwise, OpenCV is used.
#ifdef WITH_PILLOW_RESIZE
        constexpr auto DEFAULT_RESIZE_BACKEND = ResizeBackend::RESIZE_BACKEND_Pillow;
#else
        constexpr auto DEFAULT_RESIZE_BACKEND = ResizeBackend::RESIZE_BACKEND_OpenCV;
#endif

/// Supported interpolation methods for resize ops.
        enum InterpolationMethods {
            INTERPOLATION_NEAREST = 0,
            INTERPOLATION_NEAREST_EXACT = 1,
            INTERPOLATION_LINEAR = 2,
            INTERPOLATION_CUBIC = 3,
        };

        namespace detail {
            inline constexpr InterpolationMethods _interpolation_method_from_string(
                    std::string_view interpolation) {
                if (interpolation == "nearest")
                    return INTERPOLATION_NEAREST;
                else if (interpolation == "nearest-exact")
                    return INTERPOLATION_NEAREST_EXACT;
                else if (interpolation == "bilinear" || interpolation == "linear")
                    return INTERPOLATION_LINEAR;
                else if (interpolation == "bicubic" || interpolation == "cubic")
                    return INTERPOLATION_CUBIC;
                else {
                    TORCH_CHECK(false,
                                "interpolation must be either nearest | nearest-exact | bilinear | bicubic. "
                                "Got interpolation=",
                                interpolation)
                }
            }

            inline constexpr cv::InterpolationFlags _cv_interpolation_flags(
                    InterpolationMethods method) {
                switch (method) {
                    case INTERPOLATION_NEAREST:
                        return cv::INTER_NEAREST;
                    case INTERPOLATION_NEAREST_EXACT:
                        return cv::INTER_NEAREST_EXACT;
                    case INTERPOLATION_LINEAR:
                        return cv::INTER_LINEAR;
                    case INTERPOLATION_CUBIC:
                        return cv::INTER_CUBIC;
                    default:
                        TORCH_INTERNAL_ASSERT(false, "");
                }
            }

            template<InterpolationMethods method>
            inline constexpr cv::InterpolationFlags _cv_interpolation_flags() {
                if constexpr (method == INTERPOLATION_NEAREST)
                    return cv::INTER_NEAREST;
                else if constexpr (method == INTERPOLATION_NEAREST_EXACT)
                    return cv::INTER_NEAREST_EXACT;
                else if constexpr (method == INTERPOLATION_LINEAR)
                    return cv::INTER_LINEAR;
                else if constexpr (method == INTERPOLATION_CUBIC)
                    return cv::INTER_CUBIC;
            }

#ifdef WITH_PILLOW_RESIZE

            inline constexpr PillowResize::InterpolationMethods _pillow_interpolation_methods(
                    InterpolationMethods method) {
                switch (method) {
                    case INTERPOLATION_NEAREST:
                    case INTERPOLATION_NEAREST_EXACT:
                        return PillowResize::INTERPOLATION_NEAREST;
                    case INTERPOLATION_LINEAR:
                        return PillowResize::INTERPOLATION_BILINEAR;
                    case INTERPOLATION_CUBIC:
                        return PillowResize::INTERPOLATION_BICUBIC;
                }
            }

            template<InterpolationMethods method>
            inline constexpr PillowResize::InterpolationMethods _pillow_interpolation_methods() {
                if constexpr (method == INTERPOLATION_NEAREST || method == INTERPOLATION_NEAREST_EXACT)
                    return PillowResize::INTERPOLATION_NEAREST;
                else if constexpr (method == INTERPOLATION_LINEAR)
                    return PillowResize::INTERPOLATION_BILINEAR;
                else if constexpr (method == INTERPOLATION_CUBIC)
                    return PillowResize::INTERPOLATION_BICUBIC;
            }

#endif
        }

        namespace functional {
            C10_ALWAYS_INLINE at::Tensor resize(
                    const at::Tensor &src,
                    const cv::Size &size,
                    InterpolationMethods interpolation = INTERPOLATION_LINEAR,
                    bool antialias = false,
                    bool align_corners = false) {
                src.unsqueeze_(0);
                switch (interpolation) {
                    case INTERPOLATION_NEAREST:
                        return at::upsample_nearest2d(src.unsqueeze(0), {size.height, size.width}).squeeze(0);
                    case INTERPOLATION_NEAREST_EXACT:
                        return at::_upsample_nearest_exact2d(src.unsqueeze(0), {size.height, size.width}).squeeze(0);
                    case INTERPOLATION_LINEAR:
                        if (antialias)
                            return at::_upsample_bilinear2d_aa(src.unsqueeze(0), {size.height, size.width},
                                                               align_corners).squeeze(0);
                        else
                            return at::upsample_bilinear2d(src.unsqueeze(0), {size.height, size.width},
                                                           align_corners).squeeze(0);
                    case INTERPOLATION_CUBIC:
                        if (antialias)
                            return at::_upsample_bicubic2d_aa(src.unsqueeze(0), {size.height, size.width},
                                                              align_corners).squeeze(0);
                        else
                            return at::upsample_bicubic2d(src.unsqueeze(0), {size.height, size.width},
                                                          align_corners).squeeze(0);
                    default:
                        break;
                }
            }

            template<int64_t width, int64_t height,
                    InterpolationMethods interpolation = INTERPOLATION_LINEAR,
                    bool antialias = false,
                    bool align_corners = false>
            C10_ALWAYS_INLINE at::Tensor resize(
                    const at::Tensor &src) {
                if constexpr (interpolation == INTERPOLATION_NEAREST) {
                    return at::upsample_nearest2d(src.unsqueeze(0), {height, width}).squeeze(0);
                } else if constexpr (interpolation == INTERPOLATION_NEAREST_EXACT) {
                    return at::_upsample_nearest_exact2d(src.unsqueeze(0), {height, width}).squeeze(0);
                } else if constexpr (interpolation == INTERPOLATION_LINEAR) {
                    if constexpr (antialias)
                        return at::_upsample_bilinear2d_aa(src.unsqueeze(0), {height, width}, align_corners).squeeze(0);
                    else
                        return at::upsample_bilinear2d(src.unsqueeze(0), {height, width}, align_corners).squeeze(0);
                } else if constexpr (interpolation == INTERPOLATION_CUBIC) {
                    if constexpr (antialias)
                        return at::_upsample_bicubic2d_aa(src.unsqueeze(0), {height, width}, align_corners).squeeze(0);
                    else
                        return at::upsample_bicubic2d(src.unsqueeze(0), {height, width}, align_corners).squeeze(0);
                }
            }

            template<ResizeBackend backend = DEFAULT_RESIZE_BACKEND>
            C10_ALWAYS_INLINE cv::Mat resize(
                    const cv::Mat &src,
                    const cv::Size &size,
                    InterpolationMethods interpolation = INTERPOLATION_LINEAR) {
                if constexpr (backend == RESIZE_BACKEND_OpenCV) {
                    cv::Mat dst;
                    cv::resize(src, dst, size, 0, 0, detail::_cv_interpolation_flags(interpolation));
                    return dst;
                } else if constexpr (backend == RESIZE_BACKEND_Pillow) {
#ifdef WITH_PILLOW_RESIZE
                    return PillowResize::resize(src, size,
                                                detail::_pillow_interpolation_methods(interpolation));
#else
                    TORCH_CHECK_VALUE(false,
                                      "Library was not compiled with PillowResize.")
#endif
                } else {
                    TORCH_CHECK_VALUE(false,
                                      "only RESIZE_BACKEND_OpenCV or RESIZE_BACKEND_Pillow backend can be used to resize cv::Mat.")
                }
            }

            template<int64_t width, int64_t height,
                    InterpolationMethods interpolation = INTERPOLATION_LINEAR,
                    ResizeBackend backend = DEFAULT_RESIZE_BACKEND>
            C10_ALWAYS_INLINE cv::Mat resize(
                    const cv::Mat &src) {
                if constexpr (backend == RESIZE_BACKEND_OpenCV) {
                    cv::Mat dst;
                    cv::resize(src, dst, cv::Size(width, height), 0, 0,
                               detail::_cv_interpolation_flags<interpolation>());
                    return dst;
                } else if constexpr (backend == RESIZE_BACKEND_Pillow) {
#ifdef WITH_PILLOW_RESIZE
                    return PillowResize::resize(src, cv::Size(width, height),
                                                detail::_pillow_interpolation_methods<interpolation>());
#else
                    TORCH_CHECK_VALUE(false,
                                      "Library was not compiled with PillowResize.")
#endif
                } else {
                    TORCH_CHECK_VALUE(false,
                                      "only RESIZE_BACKEND_OpenCV or RESIZE_BACKEND_Pillow backend can be used to resize cv::Mat.")
                }
            }

            C10_ALWAYS_INLINE at::Tensor to_tensor(
                    const at::Tensor &tensor) {
                at::Tensor output;
                if (tensor.dtype() == at::kByte)
                    output = tensor.to(at::kFloat).div_(255.0);
                else
                    output = tensor;
                output = output.permute({2, 0, 1});
                return output.contiguous();
            }

            template<bool resize = true>
            C10_ALWAYS_INLINE at::Tensor resize_to_tensor(
                    const at::Tensor &tensor,
                    const cv::Size &size,
                    InterpolationMethods interpolation = INTERPOLATION_LINEAR,
                    bool antialias = false,
                    bool align_corners = false) {
                at::Tensor output;
                if (tensor.dtype() == at::kByte)
                    output = tensor.to(at::kFloat).div_(255.0);
                else
                    output = tensor;
                output = output.permute({2, 0, 1});
                if constexpr (resize)
                    return functional::resize(output, size, interpolation, antialias, align_corners);
                else
                    return output.contiguous();
            }

            template<int64_t width, int64_t height,
                    InterpolationMethods interpolation = INTERPOLATION_LINEAR,
                    bool antialias = false,
                    bool align_corners = false,
                    bool resize = true>
            C10_ALWAYS_INLINE at::Tensor resize_to_tensor(
                    const at::Tensor &tensor) {
                at::Tensor output;
                if (tensor.dtype() == at::kByte)
                    output = tensor.to(at::kFloat).div_(255.0);
                else
                    output = tensor;
                output = output.permute({2, 0, 1});
                if constexpr (resize)
                    return functional::resize<width, height, interpolation, antialias, align_corners>(output);
                else
                    return output.contiguous();
            }
        }  // namespace functional

/// \brief
/// Convert image to tensor.
///
/// See torchvision.transforms.ToTensor.
        class [[maybe_unused]] ToTensor {
        public:
            [[nodiscard]] inline at::Tensor forward(const at::Tensor &pic) const {
                return pic;
            }

            inline at::Tensor operator()(const at::Tensor &pic) const {
                return forward(pic);
            }

            [[nodiscard]] inline at::Tensor forward(const cv::Mat &pic) const {
                auto pic_c = pic.step > pic.cols ? pic.clone() : pic;
                auto tensor = at::from_blob(
                        pic_c.data, {pic_c.rows, pic_c.cols, pic_c.channels()},
                        at::TensorOptions(at::kByte));
                return functional::to_tensor(tensor);
            }

            inline at::Tensor operator()(const cv::Mat &pic) const {
                return forward(pic);
            }
        };

/// \brief
/// Static variant of ToTensor.
        class [[maybe_unused]] StaticToTensor {
        public:
            static inline at::Tensor forward(const at::Tensor &pic) {
                return pic;
            }

            static inline at::Tensor forward(const cv::Mat &pic) {
                auto pic_c = pic.step > pic.cols ? pic.clone() : pic;
                auto tensor = at::from_blob(
                        pic_c.data, {pic_c.rows, pic_c.cols, pic_c.channels()},
                        at::TensorOptions(at::kByte));
                return functional::to_tensor(tensor);
            }
        };

/// \brief
/// Resize and ToTensor composed together as it may save some steps.
///
/// See torchvision.transforms.Resize and torchvision.transforms.ToTensor.
/// \tparam preferable_backend: Preferable resize backend to be used.
        template<ResizeBackend preferable_backend = DEFAULT_RESIZE_BACKEND>
        class [[maybe_unused]] ResizeToTensor {
        public:
            cv::Size size;
            InterpolationMethods interpolation;
            bool antialias;
            bool align_corners;

            ResizeToTensor(at::IntArrayRef size,
                           std::string_view interpolation = "bilinear",
                           bool antialias = false,
                           bool align_corners = false)
                    : interpolation(detail::_interpolation_method_from_string(interpolation)),
                      antialias(antialias),
                      align_corners(align_corners) {
                TORCH_CHECK(!size.empty(), "size is empty")
                int width = size[0];
                int height = size.size() > 1 ? size[1] : size[0];
                this->size = cv::Size(width, height);
            }

            ResizeToTensor(at::IntArrayRef size,
                           InterpolationMethods interpolation = INTERPOLATION_LINEAR,
                           bool antialias = false,
                           bool align_corners = false)
                    : interpolation(interpolation),
                      antialias(antialias),
                      align_corners(align_corners) {
                TORCH_CHECK(!size.empty(), "size is empty")
                int width = size[0];
                int height = size.size() > 1 ? size[1] : size[0];
                this->size = cv::Size(width, height);
            }

            [[nodiscard]] inline at::Tensor forward(const at::Tensor &pic) const {
                TORCH_CHECK(pic.ndimension() == 3,
                            "pic must be 3D tensor. Got pic.ndimension()=",
                            pic.ndimension())
                TORCH_CHECK(pic.dtype() == at::kByte || at::is_floating_point(pic),
                            "pic data type must be kByte. Got pic.dtype()=",
                            pic.dtype())
                return functional::resize_to_tensor(pic, size, interpolation, antialias, align_corners);
            }

            inline at::Tensor operator()(const at::Tensor &pic) const {
                return forward(pic);
            }

            [[nodiscard]] inline at::Tensor forward(const cv::Mat &pic) const {
                cv::Mat scaled_pic;
                if constexpr (preferable_backend != RESIZE_BACKEND_Torch) {
                    scaled_pic = functional::resize<preferable_backend>(pic, size, interpolation);
                } else {  // make sure pic is contiguous
                    scaled_pic = pic.step > pic.cols ? pic.clone() : pic;
                }
                auto tensor = at::from_blob(
                        scaled_pic.data, {scaled_pic.rows, scaled_pic.cols, scaled_pic.channels()},
                        at::TensorOptions(at::kByte));
                return functional::resize_to_tensor<preferable_backend == RESIZE_BACKEND_Torch>(
                        tensor, size, interpolation, antialias, align_corners);
            }

            inline at::Tensor operator()(const cv::Mat &pic) const {
                return forward(pic);
            }

            inline at::Tensor forward(const cv::Mat &pic,
                                      const std::function<void(void *)> &deleter) const {
                cv::Mat scaled_pic;
                if constexpr (preferable_backend != RESIZE_BACKEND_Torch) {
                    scaled_pic = functional::resize<preferable_backend>(pic, size, interpolation);
                } else {  // make sure pic is contiguous
                    scaled_pic = pic.step > pic.cols ? pic.clone() : pic;
                }
                auto tensor = at::from_blob(
                        scaled_pic.data, {scaled_pic.rows, scaled_pic.cols, scaled_pic.channels()},
                        [&deleter](void *data) {
                            deleter(data);
                        },
                        at::TensorOptions(at::kByte));
                return functional::resize_to_tensor<preferable_backend == RESIZE_BACKEND_Torch>(
                        tensor, size, interpolation, antialias, align_corners);
            }

            inline at::Tensor operator()(const cv::Mat &pic,
                                         const std::function<void(void *)> &deleter) const {
                return forward(pic, deleter);
            }
        };

/// \brief
/// Static variant of ResizeToTensor.
///
/// \tparam preferable_backend: Preferable resize backend to be used.
        template<int64_t width, int64_t height,
                InterpolationMethods interpolation = INTERPOLATION_LINEAR,
                bool antialias = false,
                bool align_corners = false,
                ResizeBackend preferable_backend = DEFAULT_RESIZE_BACKEND>
        class [[maybe_unused]] StaticResizeToTensor {
        public:
            static inline at::Tensor forward(const at::Tensor &pic) {
                TORCH_CHECK(pic.ndimension() == 3,
                            "pic must be 3D tensor. Got pic.ndimension()=",
                            pic.ndimension())
                TORCH_CHECK(pic.dtype() == at::kByte || at::is_floating_point(pic),
                            "pic data type must be kByte. Got pic.dtype()=",
                            pic.dtype())
                return functional::resize_to_tensor<width, height, interpolation, antialias, align_corners>(pic);
            }

            static inline at::Tensor forward(const cv::Mat &pic) {
                cv::Mat scaled_pic;
                if constexpr (preferable_backend != RESIZE_BACKEND_Torch) {
                    scaled_pic = functional::resize<width, height, interpolation, preferable_backend>(pic);
                } else {  // make sure pic is contiguous
                    scaled_pic = pic.step > pic.cols ? pic.clone() : pic;
                }
                auto tensor = at::from_blob(
                        scaled_pic.data, {scaled_pic.rows, scaled_pic.cols, scaled_pic.channels()},
                        at::TensorOptions(at::kByte));
                return functional::resize_to_tensor<width, height, interpolation, antialias, align_corners,
                        preferable_backend == RESIZE_BACKEND_Torch>(tensor);
            }

            static inline at::Tensor forward(const cv::Mat &pic,
                                             const std::function<void(void *)> &deleter) {
                cv::Mat scaled_pic;
                if constexpr (preferable_backend != RESIZE_BACKEND_Torch) {
                    scaled_pic = functional::resize<width, height, interpolation, preferable_backend>(pic);
                } else {  // make sure pic is contiguous
                    scaled_pic = pic.step > pic.cols ? pic.clone() : pic;
                }
                auto tensor = at::from_blob(
                        scaled_pic.data, {scaled_pic.rows, scaled_pic.cols, scaled_pic.channels()},
                        [&deleter](void *data) {
                            deleter(data);
                        },
                        at::TensorOptions(at::kByte));
                return functional::resize_to_tensor<width, height, interpolation, antialias, align_corners,
                        preferable_backend == RESIZE_BACKEND_Torch>(tensor);
            }
        };

        // ---------------------
        // Normalize
        // ---------------------
        namespace functional {
            C10_ALWAYS_INLINE at::Tensor normalize(at::Tensor &tensor,
                                                   const std::vector<double> &mean,
                                                   const std::vector<double> &std,
                                                   bool inplace = false) {
                TORCH_CHECK_TYPE(tensor.is_floating_point(),
                                 "Input tensor should be a float tensor. Got ",
                                 tensor.dtype())
                TORCH_CHECK_VALUE(tensor.ndimension() >= 3,
                                  "Expected tensor to be a tensor image of size (..., C, H, W). Got tensor.size() = ",
                                  tensor.sizes())
                auto tensor_t = inplace ? tensor : tensor.clone();
                auto mean_t = at::tensor(mean, tensor.options());
                auto std_t = at::tensor(std, tensor.options());
                TORCH_CHECK_VALUE(!std_t.eq(0).any().item<bool>(),
                                  "std evaluated to zero after conversion to ",
                                  tensor.dtype(),
                                  ", leading to division by zero.")
                if (mean_t.ndimension() == 1)
                    mean_t = mean_t.view({-1, 1, 1});
                if (std_t.ndimension() == 1)
                    std_t = std_t.view({-1, 1, 1});
                return tensor_t.sub_(mean_t).div_(std_t);
            }

#if __cplusplus >= 202002L

            template<double mean0, double mean1, double mean2,
                    double std0, double std1, double std2,
                    bool inplace = false>
            C10_ALWAYS_INLINE std::conditional_t<inplace, at::Tensor &, at::Tensor> normalize(
                    std::conditional_t<inplace, at::Tensor, const at::Tensor> &tensor) {
                TORCH_CHECK_TYPE(tensor.is_floating_point(),
                                 "Input tensor should be a float tensor. Got ",
                                 tensor.dtype())
                TORCH_CHECK_VALUE(tensor.ndimension() >= 3,
                                  "Expected tensor to be a tensor image of size (..., C, H, W). Got tensor.size() = ",
                                  tensor.sizes())
                auto tensor_t = inplace ? tensor : tensor.clone();
                auto mean_t = at::tensor({mean0, mean1, mean2}, tensor.options());
                auto std_t = at::tensor({std0, std1, std2}, tensor.options());
                TORCH_CHECK_VALUE(!std_t.eq(0).any().template item<bool>(),
                                  "std evaluated to zero after conversion to ",
                                  tensor.dtype(),
                                  ", leading to division by zero.")
                if (mean_t.ndimension() == 1)
                    mean_t = mean_t.view({-1, 1, 1});
                if (std_t.ndimension() == 1)
                    std_t = std_t.view({-1, 1, 1});
                tensor_t.sub_(mean_t).div_(std_t);
                if constexpr (inplace)
                    return tensor;
                else
                    return tensor_t;
            }

#endif
        }  // namespace functional

/// \brief
/// Normalize image with mean and standard deviation.
///
/// See torchvision.transforms.Normalize.
        class [[maybe_unused]] Normalize {
            std::vector<double> mean, std;
            bool inplace;

        public:
            Normalize(at::ArrayRef<double> mean = {0.485, 0.456, 0.406},
                      at::ArrayRef<double> std = {0.229, 0.224, 0.225},
                      bool inplace = false) : inplace(inplace) {
                this->mean = std::vector<double>(mean.size());
                std::copy(mean.begin(), mean.end(), this->mean.begin());
                this->std = std::vector<double>(std.size());
                std::copy(std.begin(), std.end(), this->std.begin());
            }

            [[nodiscard]] inline at::Tensor forward(at::Tensor &tensor) const {
                return functional::normalize(tensor, mean, std, inplace);
            }

            inline at::Tensor operator()(at::Tensor &tensor) const {
                return functional::normalize(tensor, mean, std, inplace);
            }
        };

#if __cplusplus >= 202002L

/// \brief
/// Static variant of Normalize.
///
/// See torchvision.transforms.Normalize.
        template<double mean0 = 0.485, double mean1 = 0.456, double mean2 = 0.406,
                double std0 = 0.229, double std1 = 0.224, double std2 = 0.225,
                bool inplace = false>
        class [[maybe_unused]] StaticNormalize {
        public:
            static inline at::Tensor forward(at::Tensor &tensor) {
                return functional::normalize<mean0, mean1, mean2, std0, std1, std2, inplace>(tensor);
            }
        };

#endif
    }  // namespace transforms
}  // namespace torchvision
