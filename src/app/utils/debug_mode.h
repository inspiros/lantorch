#pragma once

struct DebugState {
    static DebugState &get_tls_state();

    static void set_tls_state(DebugState state);

    [[nodiscard]] inline bool debug_state() const noexcept {
        return debug_state_;
    }

    inline void set_debug_state(bool state) {
        debug_state_ = state;
    }

private:
    bool debug_state_ = true;
};

struct DebugMode {
    static bool is_enabled();

    static void set_enabled(bool mode);
};

struct AutoDebugMode {
    explicit AutoDebugMode(bool enabled) : prev_mode(DebugMode::is_enabled()) {
        DebugMode::set_enabled(enabled);
    }

    ~AutoDebugMode() {
        DebugMode::set_enabled(prev_mode);
    }

    bool prev_mode;
};

struct NoDebugGuard : public AutoDebugMode {
    NoDebugGuard() : AutoDebugMode(false) {}
};

#define DEBUG_ONLY(...)        \
if (DebugMode::is_enabled()) { \
    __VA_ARGS__();             \
}
