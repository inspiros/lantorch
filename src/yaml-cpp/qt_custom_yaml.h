#pragma once

#include <yaml-cpp/yaml.h>

#include "utils/hash.h"

#include "qt_yaml.h"
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

            switch (hash(node.Scalar())) {
                case hash("Rect"):
                case hash("rect"):
                case hash("Rectangle"):
                case hash("rectangle"):
                    rhs = DetectionBoundingBoxOptions::Shape::Rect;
                    break;
                case hash("RoundedRect"):
                case hash("roundedrect"):
                case hash("RoundedRectangle"):
                case hash("roundedrectangle"):
                    rhs = DetectionBoundingBoxOptions::Shape::RoundedRect;
                    break;
                case hash("Ellipse"):
                case hash("ellipse"):
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

            switch (hash(node.Scalar())) {
                case hash("Always"):
                case hash("always"):
                    rhs = DetectionBoundingBoxOptions::ShowFlag::Always;
                    break;
                case hash("OnFocus"):
                case hash("onfocus"):
                    rhs = DetectionBoundingBoxOptions::ShowFlag::OnFocus;
                    break;
                case hash("Never"):
                case hash("never"):
                    rhs = DetectionBoundingBoxOptions::ShowFlag::Never;
                    break;
                default:
                    return false;
            }
            return true;
        }
    };
} // end namespace YAML
