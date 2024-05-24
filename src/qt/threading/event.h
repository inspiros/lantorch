#pragma once

#include <QDeadlineTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

class Event {
    QMutex mutex_;
    bool flag_ = false;
    QWaitCondition cond_;

public:
    inline bool isSet() {
        QMutexLocker locker(&mutex_);
        return flag_;
    }

    inline bool is_set() {
        return isSet();
    }

    inline void set() {
        {
            QMutexLocker locker(&mutex_);
            flag_ = true;
        }
        cond_.wakeAll();
    }

    inline void clear() {
        QMutexLocker locker(&mutex_);
        flag_ = false;
    }

    inline bool wait(QDeadlineTimer deadline = QDeadlineTimer(QDeadlineTimer::Forever)) {
        QMutexLocker locker(&mutex_);
        while (!flag_) {
            if (!cond_.wait(&mutex_, deadline))
                return flag_;
        }
        return true;
    }

    inline bool wait(unsigned long time) {
        QMutexLocker locker(&mutex_);
        while (!flag_) {
            if (!cond_.wait(&mutex_, time))
                return flag_;
        }
        return true;
    }
};
