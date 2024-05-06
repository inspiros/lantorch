#pragma once

#include <cmath>
#include <limits>
#include <type_traits>
#include <utility>
#include <valarray>

/// \private
/// Returns the smallest floating point value larger than the min probability (0.0),
/// which equals to eps.
template<class T, typename std::enable_if_t<!std::is_integral_v<T>> * = nullptr>
[[maybe_unused]] constexpr T lb_probability() {
    return std::numeric_limits<T>::epsilon();
}

/// \private
/// Returns 0 if T is an integral type.
template<class T, typename std::enable_if_t<std::is_integral_v<T>> * = nullptr>
[[maybe_unused]] constexpr T lb_probability() {
    return T(0);
}

/// \private
/// Returns the largest floating point value smaller than the max probability (1.0),
/// which equals to 1.0 - eps.
template<class T, typename std::enable_if_t<!std::is_integral_v<T>> * = nullptr>
[[maybe_unused]] constexpr T ub_probability() {
    return T(1) - std::numeric_limits<T>::epsilon();
}

/// \private
/// Returns 1 if T is an integral type.
template<class T, typename std::enable_if_t<std::is_integral_v<T>> * = nullptr>
[[maybe_unused]] constexpr T ub_probability() {
    return T(1);
}

/**
 * Convert floating point number to integral fraction.
 *
 * @tparam T
 * @tparam R
 * @param number
 * @param precision
 * @param max_iter
 * @refitem https://stackoverflow.com/a/64828741
 */
template<class T, class R = long,
        typename std::enable_if_t<std::is_floating_point_v<T> &&
                                  std::is_integral_v<R>> * = nullptr>
std::pair<R, R> as_fraction(T number, T precision = 1e-5, std::size_t max_iter = 100) {
    R sign = !std::signbit(number);
    number = number * sign;
    T new_number, whole_part;
    T decimal_part = number - (R) number;

    std::valarray<T> vec_1{T((R) number), 1}, vec_2{1, 0}, tmp;
    std::size_t iter = 0;
    while (decimal_part > precision && iter < max_iter) {
        new_number = static_cast<T>(1) / decimal_part;
        whole_part = (R) new_number;

        tmp = vec_1;
        vec_1 = whole_part * vec_1 + vec_2;
        vec_2 = tmp;

        decimal_part = new_number - whole_part;
        iter++;
    }
    return std::make_pair(sign * vec_1[0], vec_1[1]);
}
