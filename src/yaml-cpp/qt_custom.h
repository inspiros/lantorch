#pragma once

#include "std/hash.h"

#include <yaml-cpp/yaml.h>

#include "qt.h"
#include "qt/ColorPalette"
#include "qt/overlay_graphics_items/DetectionBoundingBox"

namespace YAML {
// ColorPalette
    template<>
    struct convert<ColorPalette> {
        static Node encode(const ColorPalette &rhs) {
            if (rhs.name().size())
                return Node(rhs.name().toStdString());

            Node node(NodeType::Sequence);
                    foreach (QColor value, rhs.colors()) {
                    node.push_back(value.rgb());
                }
            return node;
        }

        static bool decode(const Node &node, ColorPalette &rhs) {
            if (node.IsScalar()) {
                rhs = ColorPalette(QString::fromStdString(node.Scalar()));
                return true;
            } else if (node.IsSequence()) {
                QList<QColor> colors;
                const_iterator it = node.begin();
                while (it != node.end()) {
                    colors.push_back(it->as<QColor>());
                    ++it;
                }
                rhs = ColorPalette(colors);
                return true;
            }
            return false;
        }
    };

// DetectionBoundingBoxOptions::Shape
    template<>
    struct convert<DetectionBoundingBoxOptions::Shape> {
        static Node encode(const DetectionBoundingBoxOptions::Shape &rhs) {
            std::string str;
            switch (rhs) {
                case DetectionBoundingBoxOptions::Shape::Rect:
                    str = "Rect";
                    break;
                case DetectionBoundingBoxOptions::Shape::RoundedRect:
                    str = "RoundedRect";
                    break;
                case DetectionBoundingBoxOptions::Shape::Ellipse:
                    str = "Ellipse";
                    break;
            }
            return Node(str);
        }

        static bool decode(const Node &node, DetectionBoundingBoxOptions::Shape &rhs) {
            if (!node.IsScalar())
                return false;

            using hasher = std::static_hash<std::string_view>;
            switch (hasher::call(node.Scalar())) {
                case hasher::call("Rect"):
                case hasher::call("rect"):
                case hasher::call("Rectangle"):
                case hasher::call("rectangle"):
                    rhs = DetectionBoundingBoxOptions::Shape::Rect;
                    break;
                case hasher::call("RoundedRect"):
                case hasher::call("roundedrect"):
                case hasher::call("RoundedRectangle"):
                case hasher::call("roundedrectangle"):
                    rhs = DetectionBoundingBoxOptions::Shape::RoundedRect;
                    break;
                case hasher::call("Ellipse"):
                case hasher::call("ellipse"):
                    rhs = DetectionBoundingBoxOptions::Shape::Ellipse;
                    break;
                default:
                    return false;
            }
            return true;
        }
    };

// DetectionBoundingBoxOptions::ShowFlag
    template<>
    struct convert<DetectionBoundingBoxOptions::ShowFlag> {
        static Node encode(const DetectionBoundingBoxOptions::ShowFlag &rhs) {
            std::string str;
            switch (rhs) {
                case DetectionBoundingBoxOptions::ShowFlag::Always:
                    str = "Always";
                    break;
                case DetectionBoundingBoxOptions::ShowFlag::OnFocus:
                    str = "OnFocus";
                    break;
                case DetectionBoundingBoxOptions::ShowFlag::Never:
                    str = "Never";
                    break;
            }
            return Node(str);
        }

        static bool decode(const Node &node, DetectionBoundingBoxOptions::ShowFlag &rhs) {
            if (!node.IsScalar())
                return false;

            using hasher = std::static_hash<std::string_view>;
            switch (hasher::call(node.Scalar())) {
                case hasher::call("Always"):
                case hasher::call("always"):
                    rhs = DetectionBoundingBoxOptions::ShowFlag::Always;
                    break;
                case hasher::call("OnFocus"):
                case hasher::call("onfocus"):
                    rhs = DetectionBoundingBoxOptions::ShowFlag::OnFocus;
                    break;
                case hasher::call("Never"):
                case hasher::call("never"):
                    rhs = DetectionBoundingBoxOptions::ShowFlag::Never;
                    break;
                default:
                    return false;
            }
            return true;
        }
    };
} // end namespace YAML
