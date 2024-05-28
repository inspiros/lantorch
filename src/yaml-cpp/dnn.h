#pragma once

#include <yaml-cpp/yaml.h>
#include "yaml-cpp/opencv.h"

#include "dnn/return_types.h"

namespace YAML {
// Classification
    template<>
    struct convert<Classification> {
        static Node encode(const Classification &rhs) {
            Node node(NodeType::Map);
            node["id"] = rhs.label_id;
            if (!rhs.label.empty())
                node["label"] = rhs.label;
            node["probability"] = rhs.probability;
            return node;
        }

        static bool decode(const Node &node, Classification &rhs) {
            if (!node.IsMap())
                return false;
            rhs.label_id = node["id"].as<int>();
            rhs.label = node["label"].as<std::string>("");
            rhs.probability = node["probability"].as<float>();
            return true;
        }
    };

// Detection
    template<>
    struct convert<Detection> {
        static Node encode(const Detection &rhs) {
            Node node(NodeType::Map);
            node["id"] = rhs.label_id;
            if (!rhs.label.empty())
                node["label"] = rhs.label;
            node["confidence"] = rhs.confidence;
            node["bbox"] = convert<cv::Rect2d>::encode(rhs.bbox);
            return node;
        }

        static bool decode(const Node &node, Detection &rhs) {
            if (!node.IsMap())
                return false;
            rhs.label_id = node["id"].as<int>();
            rhs.label = node["label"].as<std::string>("");
            rhs.confidence = node["confidence"].as<float>();
            rhs.bbox = node["bbox"].as<cv::Rect2d>();
            return true;
        }
    };
}
