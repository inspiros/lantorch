#pragma once

#include <chrono>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "std/hash.h"

#include "numeric_utils.h"
#include "stats_tracker.h"

/**
 * Time measure and duration stats tracking class.
 * It is not thread safe.
 *
 * @tparam C clock defined in std::chrono
 * @tparam M stats tracking momentum type
 * @tparam MF stats tracking momentum fraction type, must be integral
 */
template<typename C = std::chrono::system_clock,
        typename M = double, typename MF = int64_t,
        typename std::enable_if_t<std::is_same_v<C, std::chrono::system_clock> ||
                                  std::is_same_v<C, std::chrono::steady_clock> ||
                                  std::is_same_v<C, std::chrono::high_resolution_clock>> * = nullptr,
        typename std::enable_if_t<std::is_floating_point_v<M>> * = nullptr,
        typename std::enable_if_t<std::is_integral_v<MF>> * = nullptr>
class time_meter {
public:
    using clock_t = C;
    using time_point_t = typename clock_t::time_point;
    using rep_t = typename clock_t::rep;
    using period_t = typename clock_t::period;
    using duration_t = typename clock_t::duration;
    template<typename to_dur>
    using duration_cast_t = std::result_of_t<
            decltype(&std::chrono::duration_cast<to_dur, rep_t, period_t>)(duration_t)>;
    template<typename to_dur>
    using time_point_cast_t = std::result_of_t<
            decltype(&std::chrono::time_point_cast<to_dur, clock_t, period_t>)(time_point_t)>;

    using momentum_t = M;
    using momentum_frac_t = MF;

    explicit time_meter(std::optional<momentum_t> momentum = 0.1,
                        momentum_t precision = 1e-5,
                        std::size_t max_iter = 100) : duration_stats_(momentum, precision, max_iter) {}

    explicit time_meter(const std::optional<std::pair<momentum_frac_t, momentum_frac_t>> &momentum_frac)
            : duration_stats_(momentum_frac) {}

    inline void reserve(std::size_t n) {
        timestamps_.reserve(n);
    }

    inline void clear() noexcept {
        timestamps_.clear();
    }

    inline void reset() noexcept {
        clear();
    }

    inline time_point_t tick() {
        auto now = clock_t::now();
        timestamps_.push_back(now);
        return now;
    }

    [[nodiscard]] inline std::size_t n_ticks() const noexcept {
        return timestamps_.size();
    }

    [[nodiscard]] inline time_point_t timestamp(std::size_t ti) const {
        return timestamps_.at(ti);
    }

    template<typename to_dur>
    [[nodiscard]] inline time_point_cast_t<to_dur> timestamp_cast(std::size_t ti) const {
        return std::chrono::time_point_cast<to_dur>(timestamps_.at(ti));
    }

    [[nodiscard]] inline std::vector<time_point_t> timestamps() const noexcept {
        return timestamps_;
    }

    template<typename to_dur>
    [[nodiscard]] inline std::vector<time_point_cast_t<to_dur>> timestamps_cast() const {
        std::vector<time_point_cast_t<to_dur>> res;
        res.reserve(n_ticks());
        for (const auto &t: timestamps_)
            res.push_back(std::chrono::time_point_cast<to_dur>(t));
        return res;
    }

    [[nodiscard]] inline duration_t duration(std::size_t t0 = 0, std::optional<std::size_t> t1 = std::nullopt) {
        return timestamps_.at(t1.value_or(n_ticks() - 1)) - timestamps_.at(t0);
    }

    template<typename to_dur>
    [[nodiscard]] inline duration_cast_t<to_dur>
    duration_cast(std::size_t t0 = 0, std::optional<std::size_t> t1 = std::nullopt) {
        return std::chrono::duration_cast<to_dur>(timestamps_.at(t1.value_or(n_ticks() - 1)) - timestamps_.at(t0));
    }

    [[nodiscard]] inline std::vector<duration_t> durations() {
        std::vector<duration_t> res;
        res.reserve(n_ticks() - 1);
        for (std::size_t i = 1; i < n_ticks(); i++)
            res.push_back(duration(i, i - 1));
        return res;
    }

    template<typename to_dur>
    [[nodiscard]] inline std::vector<duration_cast_t<to_dur>> durations_cast() {
        std::vector<duration_cast_t<to_dur>> res;
        res.reserve(n_ticks() - 1);
        for (std::size_t i = 1; i < n_ticks(); i++)
            res.push_back(std::chrono::duration_cast<to_dur>(timestamps_.at(i) - timestamps_.at(i - 1)));
        return res;
    }

    typedef class duration_stats {
    private:
        /**
         * This is a special stats tracker that has fractional momentum.
         * The reason is that we can't do moving average with floating point
         * momentum on std::chrono::duration without messing up its type.
         */
        typedef class duration_stats_tracker : public stats_tracker<duration_t, nullptr_t, std::size_t> {
            using parent_t = stats_tracker<duration_t, nullptr_t, std::size_t>;
        public:
            using data_t = typename parent_t::data_t;
            using count_t = typename parent_t::count_t;

            explicit duration_stats_tracker(std::optional<momentum_t> momentum = 0.1,
                                            momentum_t precision = 1e-5,
                                            std::size_t max_iter = 100)
                    : parent_t(std::nullopt) {
                set_momentum(momentum, precision, max_iter);
            }

            explicit duration_stats_tracker(
                    const std::optional<std::pair<momentum_frac_t, momentum_frac_t>> &momentum_frac)
                    : parent_t(std::nullopt) {
                set_momentum_frac(momentum_frac);
            }

            explicit duration_stats_tracker(const duration_t &d,
                                            std::optional<momentum_t> momentum = 0.1,
                                            momentum_t precision = 1e-5,
                                            std::size_t max_iter = 100)
                    : parent_t(std::nullopt) {
                set_momentum(momentum, precision, max_iter);
                update(d);
            }

            explicit duration_stats_tracker(
                    const duration_t &d,
                    const std::optional<std::pair<momentum_frac_t, momentum_frac_t>> &momentum_frac)
                    : parent_t(std::nullopt) {
                set_momentum_frac(momentum_frac);
                update(d);
            }

            [[nodiscard]] inline std::optional<momentum_t> momentum() const {
                return (momentum_t) momentum_frac_->first / (momentum_t) momentum_frac_->second;
            }

            [[nodiscard]] inline std::optional<std::pair<momentum_frac_t, momentum_frac_t>>
            momentum_frac() const noexcept {
                return momentum_frac_;
            }

            inline void set_momentum(std::optional<momentum_t> momentum,
                                     momentum_t precision = 1e-5,
                                     std::size_t max_iter = 100) {
                if (momentum.has_value()) {
                    momentum_frac_ = as_fraction<momentum_t, momentum_frac_t>(momentum.value(),
                                                                              precision,
                                                                              max_iter);
                } else {
                    momentum_frac_.reset();
                }
            }

            inline void set_momentum_frac(
                    const std::optional<std::pair<momentum_frac_t, momentum_frac_t>> &momentum_frac) {
                if (momentum_frac.has_value() &&
                    (momentum_frac->first < static_cast<momentum_frac_t>(0) ||
                     momentum_frac->first > momentum_frac->second))
                    throw std::range_error("momentum must be in range [0, 1]");
                momentum_frac_ = momentum_frac;
            }

            inline void update(const duration_t &d) {
                if (!this->mean_.has_value()) {
                    this->mean_ = d;
                    if (!momentum_frac_.has_value())
                        this->count_ = 1;
                } else {
                    if (!momentum_frac_.has_value()) {
                        momentum_frac_t count = ++this->count_;
                        this->mean_ = ((count - static_cast<momentum_frac_t>(1)) * this->mean_.value() + d) / count;
                    } else {
                        momentum_frac_t m_num, m_denom;
                        std::tie(m_num, m_denom) = momentum_frac_.value();
                        this->mean_ = ((m_denom - m_num) * this->mean_.value() + m_num * d) / m_denom;
                    }
                }
            }

        private:
            using parent_t::set_momentum;
            std::optional<std::pair<momentum_frac_t, momentum_frac_t>> momentum_frac_;
        } duration_stats_tracker_t;

    public:
        duration_stats() = default;

        explicit inline duration_stats(std::optional<momentum_t> momentum,
                                       momentum_t precision = 1e-5,
                                       std::size_t max_iter = 100) {
            set_momentum(momentum, precision, max_iter);
        }

        explicit inline duration_stats(
                const std::optional<std::pair<momentum_frac_t, momentum_frac_t>> &momentum_frac) {
            set_momentum_frac(momentum_frac);
        }

        inline std::optional<momentum_t> momentum() const {
            if (momentum_frac_.has_value())
                return (momentum_t) momentum_frac_->first / (momentum_t) momentum_frac_->second;
            else
                return {};
        }

        inline std::optional<std::pair<momentum_frac_t, momentum_frac_t>> momentum_frac() const noexcept {
            return momentum_frac_;
        }

        inline void set_momentum(std::optional<momentum_t> momentum,
                                 momentum_t precision = 1e-5,
                                 std::size_t max_iter = 100) {
            if (momentum.has_value()) {
                if (momentum.value() < static_cast<momentum_t>(0) || momentum.value() > static_cast<momentum_t>(1))
                    throw std::range_error("momentum must be in range [0, 1]");
                momentum_frac_ = as_fraction<momentum_t, momentum_frac_t>(momentum.value(), precision, max_iter);
            } else
                momentum_frac_.reset();
            for (auto &[_, tracker]: trackers_)
                tracker.set_momentum_frac(momentum_frac_);
        }

        inline void set_momentum_frac(
                const std::optional<std::pair<momentum_frac_t, momentum_frac_t>> &momentum_frac) {
            if (momentum_frac.has_value() &&
                (momentum_frac->first < static_cast<momentum_frac_t>(0) ||
                 momentum_frac->first > momentum_frac->second))
                throw std::range_error("momentum must be in range [0, 1]");
            momentum_frac_ = momentum_frac;
        }

        [[nodiscard]] inline std::optional<duration_t> mean(const std::pair<std::size_t, std::size_t> &ts) const {
            return trackers_.at(ts).mean();
        }

        [[nodiscard]] inline std::optional<duration_t> mean(std::size_t t0, std::size_t t1) const {
            return mean(std::make_pair(t0, t1));
        }

        inline void update(const std::pair<std::size_t, std::size_t> &ts, const duration_t &dur) {
            if (trackers_.find(ts) == trackers_.end()) {
                trackers_[ts] = std::move(duration_stats_tracker_t(dur, momentum_frac_));
            } else {
                trackers_.at(ts).update(dur);
            }
        }

        inline void update(std::size_t t0, std::size_t t1, const duration_t &dur) {
            update(std::make_pair(t0, t1), dur);
        }

        inline void clear() {
            trackers_.clear();
        }

        inline std::size_t erase(const std::pair<std::size_t, std::size_t> &ts) {
            return trackers_.erase(ts);
        }

        inline std::size_t erase(std::size_t t0, std::size_t t1) {
            return erase(std::make_pair(t0, t1));
        }

        inline void reset(const std::pair<std::size_t, std::size_t> &ts) {
            trackers_.at(ts).reset();
        }

        inline void reset(std::size_t t0, std::size_t t1) {
            reset(std::make_pair(t0, t1));
        }

    private:
        std::optional<std::pair<momentum_frac_t, momentum_frac_t>> momentum_frac_;
        std::unordered_map<std::pair<std::size_t, std::size_t>, duration_stats_tracker_t,
                std::hash<std::pair<std::size_t, std::size_t>>> trackers_;
    } duration_stats_t;

    inline const duration_stats_t &duration_stats() const noexcept {
        return duration_stats_;
    }

    inline std::optional<momentum_t> momentum() const {
        return duration_stats_.momentum();
    }

    inline std::optional<std::pair<momentum_frac_t, momentum_frac_t>> momentum_frac() const noexcept {
        return duration_stats_.momentum_frac();
    }

    inline void set_momentum(momentum_t momentum,
                             momentum_t precision = 1e-5,
                             std::size_t max_iter = 100) {
        duration_stats_.set_momentum(momentum, precision, max_iter);
    }

    inline void set_momentum_frac(const std::optional<std::pair<momentum_frac_t, momentum_frac_t>> &momentum_frac) {
        duration_stats_.set_momentum_frac(momentum_frac);
    }

    inline void duration_stats_update(std::size_t t0 = 0, std::optional<std::size_t> t1 = std::nullopt) {
        duration_stats_.update(t0, t1.value_or(n_ticks() - 1), duration(t0, t1));
    }

    inline std::optional<duration_t> mean_duration(std::size_t t0 = 0, std::optional<std::size_t> t1 = std::nullopt) {
        return duration_stats_.mean(t0, t1.value_or(n_ticks() - 1));
    }

    template<typename to_dur>
    inline std::optional<duration_cast_t<to_dur>>
    mean_duration_cast(std::size_t t0 = 0, std::optional<std::size_t> t1 = std::nullopt) {
        auto dur_mean = mean_duration(t0, t1);
        if (dur_mean.has_value())
            return std::chrono::duration_cast<to_dur>(dur_mean.value());
        return {};
    }

    inline void clear_duration_stats() {
        duration_stats_.clear();
    }

    inline std::size_t erase_duration_stats(std::size_t t0, std::optional<std::size_t> t1 = std::nullopt) {
        return duration_stats_.erase(t0, t1.value_or(n_ticks() - 1));
    }

private:
    std::vector<time_point_t> timestamps_;
    duration_stats_t duration_stats_;
};
