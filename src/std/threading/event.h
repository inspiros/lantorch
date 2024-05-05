#pragma once

#include <iostream>

#include <condition_variable>
#include <mutex>
#include <thread>

namespace std {
/// Reimplementation of threading.Event in Python
/// Reference: https://codereview.stackexchange.com/questions/250647/c-reimplementation-of-pythons-threading-event
    class event {
        std::mutex lock_;
        bool flag_ = false;
        std::condition_variable cond_;

    public:
        inline bool isSet() {
            std::lock_guard<std::mutex> lock_guard(lock_);
            return flag_;
        }

        inline bool is_set() {
            return isSet();
        }

        inline void set() {
            {
                std::lock_guard<std::mutex> lock_guard(lock_);
                flag_ = true;
            }
            cond_.notify_all();
        }

        inline void clear() {
            std::lock_guard<std::mutex> lock_guard(lock_);
            flag_ = false;
        }

        inline void wait() {
            std::unique_lock<std::mutex> set_trigger_unique_lock(lock_);
            cond_.wait(set_trigger_unique_lock, [&]() { return flag_; });
        }

        template<typename _Rep, typename _Period>
        inline bool wait_for(const chrono::duration<_Rep, _Period> &timeout) {
            std::unique_lock<std::mutex> set_trigger_unique_lock(lock_);
            return cond_.wait_for(set_trigger_unique_lock, timeout, [&]() { return flag_; });
        }
    };
}
