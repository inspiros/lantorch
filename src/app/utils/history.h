#pragma once

#include <optional>

#include "qt/ConstantSizedMap"

template<typename T, typename TimestampT = unsigned int>
class History : public ConstantSizedMap<TimestampT, T> {
public:
    explicit History(int max_size = 10) : ConstantSizedMap<TimestampT, T>(max_size) {
        Q_ASSERT_X(max_size > 0, "History::History", "max_size must be positive");
    }

    using iterator = typename ConstantSizedMap<TimestampT, T>::iterator;
    using const_iterator = typename ConstantSizedMap<TimestampT, T>::const_iterator;
    using key_iterator = typename ConstantSizedMap<TimestampT, T>::key_iterator;
    using const_key_value_iterator = typename ConstantSizedMap<TimestampT, T>::const_key_value_iterator;
    using key_value_iterator = typename ConstantSizedMap<TimestampT, T>::key_value_iterator;

    struct Record {
        const TimestampT timestamp;
        const T value;

    private:
        static Record from_iterator(History::key_value_iterator it) {
            return {it->first, it->second};
        }

        friend History;
    };

    inline std::optional<const TimestampT> latest_timestamp() {
        auto locker = this->locker();
        if (this->empty())
            return {};
        return this->lastKey();
    }

    inline std::optional<const TimestampT> oldest_timestamp() {
        auto locker = this->locker();
        if (this->empty())
            return {};
        return this->firstKey();
    }

    inline std::optional<const TimestampT> latest_value() {
        auto locker = this->locker();
        if (this->empty())
            return {};
        return this->last();
    }

    inline std::optional<const TimestampT> oldest_value() {
        auto locker = this->locker();
        if (this->empty())
            return {};
        return this->first();
    }

    inline std::optional<Record> latest_record() {
        auto locker = this->locker();
        if (this->empty())
            return {};
        auto it = this->keyValueEnd() - 1;
        return {it->first, it->second};
    }

    inline std::optional<Record> oldest_record() {
        auto locker = this->locker();
        if (this->empty())
            return {};
        auto it = this->keyValueBegin();
        return {it->first, it->second};
    }

    inline std::optional<const T> at(const TimestampT &key) {
        auto locker = this->locker();
        auto it = this->find(key);
        if (it == this->end())
            return {};
        return *it;
    }

    inline std::optional<Record> record_at(const TimestampT &key) {
        auto locker = this->locker();
        auto it = this->find(key);
        if (it == this->end())
            return {};
        return Record::from_iterator(it);
    }

    inline std::optional<Record> latest_record_before(const TimestampT &key,
                                                      const std::optional<TimestampT> &max_lateness = {}) {
        Q_ASSERT_X(!max_lateness.has_value() || max_lateness.value() >= 0,
                   "History::latest_record_before", "max_lateness must be non-negative");
        auto locker = this->locker();
        if (this->empty())
            return {};
        key_value_iterator it = this->keyValueBegin();
        if (it->first > key)
            return {};
        key_value_iterator prev_it = it;
        it++;
        for (; prev_it != this->keyValueEnd(); it++, prev_it++) {
            if (prev_it->first <= key && (it == this->keyValueEnd() || it->first > key)) {
                if (!max_lateness.has_value() || key - prev_it->first <= max_lateness.value())
                    return Record::from_iterator(prev_it);
                break;
            }
        }
        return {};
    }
};
