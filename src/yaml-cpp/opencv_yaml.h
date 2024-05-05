#pragma once

#include <yaml-cpp/yaml.h>

#include <opencv2/core.hpp>

namespace YAML {
// cv::Size
    template<typename T>
    struct convert<cv::Size_<T>> {
        static Node encode(const cv::Size_<T> &rhs) {
            Node node(NodeType::Sequence);
            node.push_back(rhs.width);
            node.push_back(rhs.height);
            return node;
        }

        static bool decode(const Node &node, cv::Size_<T> &rhs) {
            if (!node.IsSequence())
                return false;

            rhs.width = node[0].as<T>();
            rhs.height = node[1].as<T>();
            return true;
        }
    };
} // end namespace YAML
