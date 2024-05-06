#pragma once

#include "std/hash.h"

#include <yaml-cpp/yaml.h>

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

            using hasher = std::static_hash<std::string_view>;
            switch (hasher::call(node.Scalar())) {
                case hasher::call("Byte"):
                case hasher::call("byte"):
                case hasher::call("torch.byte"):
                    rhs = at::ScalarType::Byte;
                    break;
                case hasher::call("Char"):
                case hasher::call("char"):
                case hasher::call("torch.char"):
                    rhs = at::ScalarType::Char;
                    break;
                case hasher::call("Short"):
                case hasher::call("short"):
                case hasher::call("torch.short"):
                    rhs = at::ScalarType::Short;
                    break;
                case hasher::call("Int"):
                case hasher::call("int"):
                case hasher::call("torch.int"):
                    rhs = at::ScalarType::Int;
                    break;
                case hasher::call("Long"):
                case hasher::call("long"):
                case hasher::call("torch.long"):
                    rhs = at::ScalarType::Long;
                    break;
                case hasher::call("Half"):
                case hasher::call("half"):
                case hasher::call("torch.half"):
                    rhs = at::ScalarType::Half;
                    break;
                case hasher::call("Float"):
                case hasher::call("float"):
                case hasher::call("torch.float"):
                    rhs = at::ScalarType::Float;
                    break;
                case hasher::call("Double"):
                case hasher::call("double"):
                case hasher::call("torch.double"):
                    rhs = at::ScalarType::Double;
                    break;
                case hasher::call("ComplexHalf"):
                case hasher::call("complex32"):
                case hasher::call("torch.complex32"):
                    rhs = at::ScalarType::ComplexHalf;
                    break;
                case hasher::call("ComplexFloat"):
                case hasher::call("complex64"):
                case hasher::call("torch.complex64"):
                    rhs = at::ScalarType::ComplexFloat;
                    break;
                case hasher::call("ComplexDouble"):
                case hasher::call("complex124"):
                case hasher::call("torch.complex124"):
                    rhs = at::ScalarType::ComplexDouble;
                    break;
                case hasher::call("Bool"):
                case hasher::call("bool"):
                case hasher::call("torch.bool"):
                    rhs = at::ScalarType::Bool;
                    break;
                case hasher::call("QInt8"):
                case hasher::call("qint8"):
                case hasher::call("torch.qint8"):
                    rhs = at::ScalarType::QInt8;
                    break;
                case hasher::call("QUInt8"):
                case hasher::call("quint8"):
                case hasher::call("torch.quint8"):
                    rhs = at::ScalarType::QUInt8;
                    break;
                case hasher::call("QInt32"):
                case hasher::call("qint32"):
                case hasher::call("torch.qint32"):
                    rhs = at::ScalarType::QInt32;
                    break;
                case hasher::call("BFloat16"):
                case hasher::call("bfloat16"):
                case hasher::call("torch.bfloat16"):
                    rhs = at::ScalarType::BFloat16;
                    break;
                case hasher::call("QUInt4x2"):
                case hasher::call("quint4x2"):
                case hasher::call("torch.quint4x2"):
                    rhs = at::ScalarType::QUInt4x2;
                    break;
                case hasher::call("QUInt2x4"):
                case hasher::call("quint2x4"):
                case hasher::call("torch.quint2x4"):
                    rhs = at::ScalarType::QUInt2x4;
                    break;
                case hasher::call("Bits1x8"):
                case hasher::call("bits1x8"):
                case hasher::call("torch.bits1x8"):
                    rhs = at::ScalarType::Bits1x8;
                    break;
                case hasher::call("Bits2x4"):
                case hasher::call("bits2x4"):
                case hasher::call("torch.bits2x4"):
                    rhs = at::ScalarType::Bits2x4;
                    break;
                case hasher::call("Bits4x2"):
                case hasher::call("bits4x2"):
                case hasher::call("torch.bits4x2"):
                    rhs = at::ScalarType::Bits4x2;
                    break;
                case hasher::call("Bits8"):
                case hasher::call("bits8"):
                case hasher::call("torch.bits8"):
                    rhs = at::ScalarType::Bits8;
                    break;
                case hasher::call("Bits16"):
                case hasher::call("bits16"):
                case hasher::call("torch.bits16"):
                    rhs = at::ScalarType::Bits16;
                    break;
                case hasher::call("Float8_e5m2"):
                case hasher::call("float8_e5m2"):
                case hasher::call("torch.float8_e5m2"):
                    rhs = at::ScalarType::Float8_e5m2;
                    break;
                case hasher::call("Float8_e4m3fn"):
                case hasher::call("float8_e4m3fn"):
                case hasher::call("torch.float8_e4m3fn"):
                    rhs = at::ScalarType::Float8_e4m3fn;
                    break;
                default:
                    return false;
            }
            return true;
        }
    };
} // end namespace YAML
