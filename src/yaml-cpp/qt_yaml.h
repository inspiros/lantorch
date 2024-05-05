/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ***** END LICENSE BLOCK ***** */

/*
 * Copyright (c) 2014, Filip Brcic <brcha@gna.org>. All rights reserved.
 *
 * This file is part of lusim
 */

// Copied and modified from https://gist.github.com/brcha/d392b2fe5f1e427cc8a6

#pragma once

#include <yaml-cpp/yaml.h>

#include "utils/hash.h"

#include <QColor>
#include <QString>
#include <QMap>
#include <QVector>
#include <QList>
#include <QPair>
#include <QSize>
#include <QSizeF>
#include <QThread>

namespace YAML {
// QColor
    template<>
    struct convert<QColor> {
        static Node encode(const QColor &rhs) {
            return Node(rhs.name(rhs.alpha() ? QColor::HexArgb : QColor::HexRgb));
        }

        static bool decode(const Node &node, QColor &rhs) {
            if (node.IsSequence() && (node.size() > 0 && node.size() < 5)) {
                if (node.size() < 3) {
                    rhs.setRed(node[0].as<int>());
                    rhs.setGreen(node[0].as<int>());
                    rhs.setBlue(node[0].as<int>());
                    if (node.size() == 2)
                        rhs.setAlpha(node[1].as<int>());
                } else {
                    rhs.setRed(node[0].as<int>());
                    rhs.setGreen(node[1].as<int>());
                    rhs.setBlue(node[2].as<int>());
                    if (node.size() == 4)
                        rhs.setAlpha(node[3].as<int>());
                }
                return true;
            } else if (node.IsScalar()) {
                rhs = QColor(QString::fromStdString(node.Scalar()));
                return true;
            }
            return false;
        }
    };

// QSize
    template<>
    struct convert<QSize> {
        static Node encode(const QSize &rhs) {
            Node node(NodeType::Sequence);
            node.push_back(rhs.width());
            node.push_back(rhs.height());
            return node;
        }

        static bool decode(const Node &node, QSize &rhs) {
            if (!node.IsSequence())
                return false;

            rhs.setWidth(node[0].as<int>());
            rhs.setHeight(node[1].as<int>());
            return true;
        }
    };

// QSizeF
    template<>
    struct convert<QSizeF> {
        static Node encode(const QSizeF &rhs) {
            Node node(NodeType::Sequence);
            node.push_back(rhs.width());
            node.push_back(rhs.height());
            return node;
        }

        static bool decode(const Node &node, QSizeF &rhs) {
            if (!node.IsSequence())
                return false;

            rhs.setWidth(node[0].as<qreal>());
            rhs.setHeight(node[1].as<qreal>());
            return true;
        }
    };

// QString
    template<>
    struct convert<QString> {
        static Node encode(const QString &rhs) {
            return Node(rhs.toStdString());
        }

        static bool decode(const Node &node, QString &rhs) {
            if (!node.IsScalar())
                return false;
            rhs = QString::fromStdString(node.Scalar());
            return true;
        }
    };

// QMap
    template<typename Key, typename Value>
    struct convert<QMap<Key, Value> > {
        static Node encode(const QMap<Key, Value> &rhs) {
            Node node(NodeType::Map);
            auto it = rhs.constBegin();
            while (it != rhs.constEnd()) {
                node.force_insert(it.key(), it.value());
                ++it;
            }
            return node;
        }

        static bool decode(const Node &node, QMap<Key, Value> &rhs) {
            if (!node.IsMap())
                return false;

            rhs.clear();
            const_iterator it = node.begin();
            while (it != node.end()) {
                rhs[it->first.as<Key>()] = it->second.as<Value>();
                ++it;
            }
            return true;
        }
    };

// QVector
    template<typename T>
    struct convert<QVector<T> > {
        static Node encode(const QVector<T> &rhs) {
            Node node(NodeType::Sequence);
                    foreach (T value, rhs) {
                    node.push_back(value);
                }
            return node;
        }

        static bool decode(const Node &node, QVector<T> &rhs) {
            if (!node.IsSequence())
                return false;

            rhs.clear();
            const_iterator it = node.begin();
            while (it != node.end()) {
                rhs.push_back(it->as<T>());
                ++it;
            }
            return true;
        }
    };

// QList
    template<typename T>
    struct convert<QList<T> > {
        static Node encode(const QList<T> &rhs) {
            Node node(NodeType::Sequence);
                    foreach (T value, rhs) {
                    node.push_back(value);
                }
            return node;
        }

        static bool decode(const Node &node, QList<T> &rhs) {
            if (!node.IsSequence())
                return false;

            rhs.clear();
            const_iterator it = node.begin();
            while (it != node.end()) {
                rhs.push_back(it->as<T>());
                ++it;
            }
            return true;
        }
    };

// QPair
    template<typename T, typename U>
    struct convert<QPair<T, U> > {
        static Node encode(const QPair<T, U> &rhs) {
            Node node(NodeType::Sequence);
            node.push_back(rhs.first);
            node.push_back(rhs.second);
            return node;
        }

        static bool decode(const Node &node, QPair<T, U> &rhs) {
            if (!node.IsSequence())
                return false;
            if (node.size() != 2)
                return false;

            rhs.first = node[0].as<T>();
            rhs.second = node[1].as<U>();
            return true;
        }
    };

// QStringList
    template<>
    struct convert<QStringList> {
        static Node encode(const QStringList &rhs) {
            Node node(NodeType::Sequence);
            for (const auto &value: rhs) {
                node.push_back(value);
            }
            return node;
        }

        static bool decode(const Node &node, QStringList &rhs) {
            if (!node.IsSequence())
                return false;

            rhs.clear();
            const_iterator it = node.begin();
            while (it != node.end()) {
                rhs.push_back(it->as<QString>());
                ++it;
            }
            return true;
        }
    };

// TODO: Add the rest of the container classes
// QLinkedList, QStack, QQueue, QSet, QMultiMap, QHash, QMultiHash, ...

// QThread::Priority
    template<>
    struct convert<QThread::Priority> {
        static Node encode(const QThread::Priority &rhs) {
            std::string str;
            switch (rhs) {
                case QThread::IdlePriority:
                    str = "IdlePriority";
                    break;
                case QThread::LowestPriority:
                    str = "LowestPriority";
                    break;
                case QThread::LowPriority:
                    str = "LowPriority";
                    break;
                case QThread::NormalPriority:
                    str = "NormalPriority";
                    break;
                case QThread::HighPriority:
                    str = "HighPriority";
                    break;
                case QThread::HighestPriority:
                    str = "HighestPriority";
                    break;
                case QThread::TimeCriticalPriority:
                    str = "TimeCriticalPriority";
                    break;
                case QThread::InheritPriority:
                    str = "InheritPriority";
                    break;
            }
            return Node(str);
        }

        static bool decode(const Node &node, QThread::Priority &rhs) {
            if (!node.IsScalar())
                return false;

            switch (hash(node.Scalar())) {
                case hash("IdlePriority"):
                    rhs = QThread::IdlePriority;
                    break;
                case hash("LowestPriority"):
                    rhs = QThread::LowestPriority;
                    break;
                case hash("LowPriority"):
                    rhs = QThread::LowPriority;
                    break;
                case hash("NormalPriority"):
                    rhs = QThread::NormalPriority;
                    break;
                case hash("HighPriority"):
                    rhs = QThread::HighPriority;
                    break;
                case hash("HighestPriority"):
                    rhs = QThread::HighestPriority;
                    break;
                case hash("TimeCriticalPriority"):
                    rhs = QThread::TimeCriticalPriority;
                    break;
                case hash("InheritPriority"):
                    rhs = QThread::InheritPriority;
                    break;
                default:
                    return false;
            }
            return true;
        }
    };
} // end namespace YAML
