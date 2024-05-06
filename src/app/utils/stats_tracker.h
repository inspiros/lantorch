#pragma once

#include <optional>
#include <stdexcept>

/**
 * Online updating stats tracker.
 * Momentum can be explicitly set to std::nullopt.
 *
 * @tparam T data type
 * @tparam M momentum type
 * @tparam I count type
 */
template<typename T, typename M = T, typename I = std::size_t>
class stats_tracker {
protected:
    I count_ = 0;
    std::optional<M> momentum_;
    std::optional<T> mean_;

    static constexpr std::optional<M> default_momentum() {
        if constexpr (std::is_convertible_v<double, M>)
            return 0.1;
        else
            return std::nullopt;
    }

public:
    using data_t [[maybe_unused]] = T;
    using momentum_t [[maybe_unused]] = M;
    using count_t [[maybe_unused]] = I;

    explicit stats_tracker(std::optional<momentum_t> momentum = default_momentum()) {
        set_momentum(momentum);
    }

    explicit stats_tracker(const data_t &x_t, std::optional<momentum_t> momentum = default_momentum()) {
        set_momentum(momentum);
        update(x_t);
    }

    [[nodiscard]] inline std::optional<momentum_t> momentum() const noexcept {
        return momentum_;
    }

    inline void set_momentum(std::optional<momentum_t> momentum) {
        if (momentum.has_value()) {
            if constexpr (std::is_convertible_v<double, momentum_t>) {
                if (momentum.value() < static_cast<momentum_t>(0.) || momentum.value() > static_cast<momentum_t>(1.))
                    throw std::range_error("momentum must be in range [0, 1]");
            }
            count_ = 0;
        }
        momentum_ = momentum;
    }

    [[nodiscard]] inline std::optional<data_t> mean() const noexcept {
        return mean_;
    }

    inline void update(const data_t &x_t) {
        if (!mean_.has_value()) {
            mean_ = x_t;
            if (!momentum_.has_value())
                this->count_ = 1;
        } else {
            if constexpr (std::is_convertible_v<double, momentum_t>) {
                momentum_t m = momentum_.value_or(static_cast<momentum_t>(++count_));
                mean_ = (data_t) ((static_cast<momentum_t>(1.) - m) * mean_.value() + m * x_t);
            } else {
                static_assert(std::is_convertible_v<double, momentum_t>,
                              "momentum type is not convertible from floating type, "
                              "please extend this class and provide your own update rule.");
            }
        }
    }

    inline void reset() {
        count_ = 0;
        mean_.reset();
    }
};
