#include "debug_mode.h"

namespace {
    thread_local DebugState debug_state_tls;
}

DebugState &DebugState::get_tls_state() {
    return debug_state_tls;
}

void DebugState::set_tls_state(DebugState state) {
    debug_state_tls = state;
}

bool DebugMode::is_enabled() {
    return DebugState::get_tls_state().debug_state();
}

void DebugMode::set_enabled(bool mode) {
    DebugState::get_tls_state().set_debug_state(mode);
}
