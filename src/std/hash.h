#pragma once

#include <string>

namespace std {
    namespace detail {
        inline void hash_combine_impl(std::size_t &seed) {}

        template<typename T, typename... Rest>
        inline void hash_combine_impl(std::size_t &seed, const T &v, Rest... rest) {
            std::hash < T > hasher;
            seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            hash_combine_impl(seed, rest...);
        }
    }

    template<typename T, typename... Rest>
    inline std::size_t hash_combine(std::size_t seed, const T &v, Rest... rest) {
        auto hash = seed;
        detail::hash_combine_impl(seed, std::forward<const T &>(v), std::forward<Rest>(rest)...);
        return hash;
    }

    template<class T1, class T2>
    struct hash<std::pair<T1, T2>> {
        std::size_t operator()(const std::pair<T1, T2> &p) const {
            return hash_combine(0, p.first, p.second);
        }
    };

    template<typename T>
    struct static_hash : __hash_enum<T> {
    };

    template<>
    struct static_hash<std::string_view> {
        static constexpr std::size_t call(const std::string_view &str) {
            std::size_t hash = 5385;
            for (const auto &c: str)
                hash = ((hash << 5) + hash) + c;
            return hash;
        }
    };
}
