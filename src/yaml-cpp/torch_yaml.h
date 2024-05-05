#pragma once

#include <yaml-cpp/yaml.h>

#include "utils/hash.h"

#undef slots

#include <ATen/Device.h>
#include <ATen/ScalarType.h>

#define slots Q_SLOTS

namespace YAML {
// at::Device
    template<>
    struct convert<at::Device> {
        static Node encode(const at::Device &rhs) {
            return Node(rhs.str());
        }

        static bool decode(const Node &node, at::Device &rhs) {
            if (!node.IsScalar())
                return false;
            rhs = at::Device(node.Scalar());
            return true;
        }
    };

    template<typename S>
    struct as_if<at::Device, S> {
        explicit as_if(const Node &node_) : node(node_) {}

        const Node &node;

        at::Device operator()(const S &fallback) const {
            if (!node.m_pNode)
                return fallback;

            at::Device t = at::kCPU;
            if (convert<at::Device>::decode(node, t))
                return t;
            return fallback;
        }
    };

    template<>
    struct as_if<at::Device, void> {
        explicit as_if(const Node &node_) : node(node_) {}

        const Node &node;

        at::Device operator()() const {
            if (!node.m_pNode)
                throw TypedBadConversion<at::Device>(node.Mark());

            at::Device t = at::kCPU;
            if (convert<at::Device>::decode(node, t))
                return t;
            throw TypedBadConversion<at::Device>(node.Mark());
        }
    };

// at::ScalarType
    template<>
    struct convert<at::ScalarType> {
        static Node encode(const at::ScalarType &rhs) {
            return Node(toString(rhs));
        }

        static bool decode(const Node &node, at::ScalarType &rhs) {
            if (!node.IsScalar())
                return false;
            switch (hash(node.Scalar())) {
                case hash("Byte"):
                case hash("byte"):
                case hash("torch.byte"):
                    rhs = at::ScalarType::Byte;
                    break;
                case hash("Char"):
                case hash("char"):
                case hash("torch.char"):
                    rhs = at::ScalarType::Char;
                    break;
                case hash("Short"):
                case hash("short"):
                case hash("torch.short"):
                    rhs = at::ScalarType::Short;
                    break;
                case hash("Int"):
                case hash("int"):
                case hash("torch.int"):
                    rhs = at::ScalarType::Int;
                    break;
                case hash("Long"):
                case hash("long"):
                case hash("torch.long"):
                    rhs = at::ScalarType::Long;
                    break;
                case hash("Half"):
                case hash("half"):
                case hash("torch.half"):
                    rhs = at::ScalarType::Half;
                    break;
                case hash("Float"):
                case hash("float"):
                case hash("torch.float"):
                    rhs = at::ScalarType::Float;
                    break;
                case hash("Double"):
                case hash("double"):
                case hash("torch.double"):
                    rhs = at::ScalarType::Double;
                    break;
                case hash("ComplexHalf"):
                case hash("complex32"):
                case hash("torch.complex32"):
                    rhs = at::ScalarType::ComplexHalf;
                    break;
                case hash("ComplexFloat"):
                case hash("complex64"):
                case hash("torch.complex64"):
                    rhs = at::ScalarType::ComplexFloat;
                    break;
                case hash("ComplexDouble"):
                case hash("complex124"):
                case hash("torch.complex124"):
                    rhs = at::ScalarType::ComplexDouble;
                    break;
                case hash("Bool"):
                case hash("bool"):
                case hash("torch.bool"):
                    rhs = at::ScalarType::Bool;
                    break;
                case hash("QInt8"):
                case hash("qint8"):
                case hash("torch.qint8"):
                    rhs = at::ScalarType::QInt8;
                    break;
                case hash("QUInt8"):
                case hash("quint8"):
                case hash("torch.quint8"):
                    rhs = at::ScalarType::QUInt8;
                    break;
                case hash("QInt32"):
                case hash("qint32"):
                case hash("torch.qint32"):
                    rhs = at::ScalarType::QInt32;
                    break;
                case hash("BFloat16"):
                case hash("bfloat16"):
                case hash("torch.bfloat16"):
                    rhs = at::ScalarType::BFloat16;
                    break;
                case hash("QUInt4x2"):
                case hash("quint4x2"):
                case hash("torch.quint4x2"):
                    rhs = at::ScalarType::QUInt4x2;
                    break;
                case hash("QUInt2x4"):
                case hash("quint2x4"):
                case hash("torch.quint2x4"):
                    rhs = at::ScalarType::QUInt2x4;
                    break;
                case hash("Bits1x8"):
                case hash("bits1x8"):
                case hash("torch.bits1x8"):
                    rhs = at::ScalarType::Bits1x8;
                    break;
                case hash("Bits2x4"):
                case hash("bits2x4"):
                case hash("torch.bits2x4"):
                    rhs = at::ScalarType::Bits2x4;
                    break;
                case hash("Bits4x2"):
                case hash("bits4x2"):
                case hash("torch.bits4x2"):
                    rhs = at::ScalarType::Bits4x2;
                    break;
                case hash("Bits8"):
                case hash("bits8"):
                case hash("torch.bits8"):
                    rhs = at::ScalarType::Bits8;
                    break;
                case hash("Bits16"):
                case hash("bits16"):
                case hash("torch.bits16"):
                    rhs = at::ScalarType::Bits16;
                    break;
                case hash("Float8_e5m2"):
                case hash("float8_e5m2"):
                case hash("torch.float8_e5m2"):
                    rhs = at::ScalarType::Float8_e5m2;
                    break;
                case hash("Float8_e4m3fn"):
                case hash("float8_e4m3fn"):
                case hash("torch.float8_e4m3fn"):
                    rhs = at::ScalarType::Float8_e4m3fn;
                    break;
                default:
                    return false;
            }
            return true;
        }
    };
} // end namespace YAML
