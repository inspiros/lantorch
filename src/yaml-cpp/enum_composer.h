#pragma once

#include "std/lexical_cast.h"

#include <yaml-cpp/yaml.h>

#include "node.h"

namespace YAML {
    template<typename T>
    struct EnumComposer {
        EnumComposer() = default;

        explicit EnumComposer(int v) {
            set_value(v);
        }

        inline void set_value(int v) {
            val = v;
        }

        inline void add_value(int v) {
            val |= v;
        }

        inline EnumComposer &operator=(int v) {
            set_value(v);
            return *this;
        }

        inline EnumComposer &operator=(const EnumComposer &other) {
            set_value(other.val);
            return *this;
        }

        inline EnumComposer &operator|=(int v) {
            val |= v;
            return *this;
        }

        inline EnumComposer &operator|=(const EnumComposer &other) {
            val |= other.val;
            return *this;
        }

        inline bool operator==(int v) {
            return val == v;
        }

        inline bool operator==(const EnumComposer &other) {
            return val == other.val;
        }

        [[nodiscard]] inline int value() const noexcept {
            return val;
        }

    private:
        int val = 0;
    };

    template<typename T>
    struct convert<EnumComposer<T>> {
        static Node encode(const EnumComposer<T> &rhs) {
            return Node(std::to_string(rhs.value()));
        }

        static bool decode(const Node &node, EnumComposer<T> &rhs) {
            if (node.IsSequence()) {
                for (const auto &n: node)
                    rhs |= get_value(n);
                return true;
            } else if (node.IsScalar()) {
                if (node.Scalar().find('|') != std::string::npos) {
                    auto s = node.Scalar();
                    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

                    std::size_t last = 0, next;
                    Node n;
                    while ((next = s.find('|', last)) != std::string::npos) {
                        call_private::Assign(n, s.substr(last, next - last).c_str());
                        rhs |= get_value(n);
                        last = next + 1;
                    }
                    call_private::Assign(n, s.substr(last).c_str());
                    rhs |= get_value(n);
                } else
                    rhs = get_value(node);
                return true;
            }
            return false;
        }

    private:
        static inline int get_value(const Node &node) {
            int res;
            if (std::lexical_cast(node.Scalar(), res)) {
                return res;
            } else {
                return (int) node.as<T>();
            }
        }
    };
}
