#pragma once

#include <yaml-cpp/yaml.h>

#include <opencv2/core.hpp>

namespace YAML {
// cv::Size
    template<typename T>
    struct convert<cv::Size_<T>> {
        static Node encode(const cv::Size_<T> &rhs) {
            Node node(NodeType::Map);
            node["width"] = rhs.width;
            node["height"] = rhs.height;
            return node;
        }

        static bool decode(const Node &node, cv::Size_<T> &rhs) {
            if (node.IsMap()) {
                rhs.width = node["width"].as<T>();
                rhs.height = node["height"].as<T>();
                return true;
            } else if (node.IsSequence()) {
                rhs.width = node[0].as<T>();
                rhs.height = node[1].as<T>();
                return true;
            }
            return false;
        }
    };

// cv::Rect
    template<typename T>
    struct convert<cv::Rect_<T>> {
        static Node encode(const cv::Rect_<T> &rhs) {
            Node node(NodeType::Map);
            node["x"] = rhs.x;
            node["y"] = rhs.y;
            node["width"] = rhs.width;
            node["height"] = rhs.height;
            return node;
        }

        static bool decode(const Node &node, cv::Rect_<T> &rhs) {
            if (node.IsMap()) {
                rhs = {node["x"].as<T>(), node["y"].as<T>(), node["width"].as<T>(), node["height"].as<T>()};
                return true;
            } else if (node.IsSequence()) {
                rhs = {node[0].as<T>(), node[1].as<T>(), node[2].as<T>(), node[3].as<T>()};
                return true;
            }
            return false;
        }
    };
} // end namespace YAML
