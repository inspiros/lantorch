#pragma once

#include <stdexcept>

namespace std {
    class not_implemented_error : public std::logic_error {
        std::string _text;

        not_implemented_error(const char *message, const char *function)
                : std::logic_error("Function not yet implemented") {
            _text = message;
            _text += " : ";
            _text += function;
        }

    public:
        not_implemented_error() : not_implemented_error("Not Implemented", __FUNCTION__) {}

        explicit not_implemented_error(const char *message) : not_implemented_error(message, __FUNCTION__) {}

        [[nodiscard]] const char *what() const noexcept override {
            return _text.c_str();
        }
    };
}
