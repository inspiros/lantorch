#pragma once

#include <QCoreApplication>
#include <QTime>
#include <QTimer>

/// Delay using time polling loop.
/// \param ms
/// \param process_maxtime
inline void qdelay(int ms, int process_maxtime = 100) {
    if (ms) {
        auto dieTime = QTime::currentTime().addMSecs(ms);
        while (QTime::currentTime() < dieTime)
            QCoreApplication::processEvents(QEventLoop::AllEvents, process_maxtime);
    }
}

/// Delay using QEventLoop, which does not spike CPU like qdelay.
/// \param ms
inline void qsmart_delay(int ms) {
    QEventLoop loop;
    QTimer t;
    QTimer::connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
    t.start(ms);
    loop.exec();
}
