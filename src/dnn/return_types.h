#pragma once

#include <opencv2/core/types.hpp>

#include <QMetaType>

/**
 * Classification result
 */
struct Classification {
    int label_id = -1;
    std::string label = "";
    float probability = 0.;
};

Q_DECLARE_METATYPE(Classification)

/**
 * Object detection result
 */
struct Detection {
    int label_id = -1;
    std::string label = "";
    float confidence = 0.;
    cv::Rect2d bbox{};
};

Q_DECLARE_METATYPE(Detection)
