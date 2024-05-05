#pragma once

#include <fstream>

#include <opencv2/core.hpp>

namespace cv {
    [[maybe_unused]] inline void matwrite(
            const cv::String &filename,
            const cv::String &name,
            const cv::Mat &mat) {
        cv::FileStorage fs(filename, cv::FileStorage::WRITE);
        fs << name << mat;
        fs.release();
    }

    [[maybe_unused]] inline void matwrite(
            cv::FileStorage &fs,
            const cv::String &name,
            const cv::Mat &mat) {
        fs << name << mat;
    }

    [[maybe_unused]] inline cv::Mat matread(
            const cv::String &filename,
            const cv::String &name) {
        cv::FileStorage fs(filename, cv::FileStorage::READ);
        cv::Mat mat;
        fs[name] >> mat;
        fs.release();
        return mat;
    }

    [[maybe_unused]] inline cv::Mat matread(
            const cv::FileStorage &fs,
            const cv::String &name) {
        cv::Mat mat;
        fs[name] >> mat;
        return mat;
    }
}
