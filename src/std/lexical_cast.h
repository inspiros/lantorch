#pragma once

#include <algorithm>
#include <string>
#include <stdexcept>

extern inline int isbdigit(int c) {
    return c == '0' || c == '1';
}

extern inline int isodigit(int c) {
    return '0' <= c && c <= '7';
}

namespace std {
    bool lexical_cast(const std::string &str, int &num);
}
