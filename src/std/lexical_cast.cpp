#include "lexical_cast.h"

#include <iostream>

namespace std {
    namespace {
        inline unsigned int get_prefix_base(const std::string &str, bool &contains_prefix) {
            contains_prefix = false;
            if (str.size() < 2)
                return 0;
            if (str[0] != '0')
                return 10;
            if (str[1] == 'x' || str[1] == 'X') {
                contains_prefix = true;
                return 16;
            } else if (str[1] == 'o' || str[1] == 'O') {
                contains_prefix = true;
                return 8;
            } else if (::isodigit(str[1])) {
                return 8;
            } else if (str[1] == 'b' || str[1] == 'B') {
                contains_prefix = true;
                return 2;
            } else if (::isdigit(str[1]))
                return 10;
            return 0;
        }

        inline bool contains_digit(const std::string &str, unsigned int &base) {
            bool has_prefix;
            base = get_prefix_base(str, has_prefix);
            auto start = str.begin(), end = str.end();
            if (has_prefix)
                std::advance(start, 2);
            switch (base) {
                case 10:
                    return std::all_of(start, end, ::isdigit);
                case 16:
                    return std::all_of(start, end, ::isxdigit);
                case 8:
                    return std::all_of(start, end, ::isodigit);
                case 2:
                    return std::all_of(start, end, ::isbdigit);
                default:
                    break;
            }
            return false;
        }
    }

    bool lexical_cast(const std::string &str, int &num) {
        if (str.empty())
            return false;
        int sign = 1;
        bool contains_sign = false;
        switch (str[0]) {
            case '-':
                sign = -1;
            case '+':
                contains_sign = true;
            default:
                break;
        }
        unsigned int base;
        if (contains_digit(str, base)) {
            num = sign * std::stoi(str.substr(contains_sign ? 1 : 0), nullptr, (int) base);
            return true;
        }
        return false;
    }
}
