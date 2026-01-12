#pragma once
#include "format.hpp"

namespace std {
    template <typename... Args>
    void println(const char* fmt, Args&&... args) {
        detail::console_writer w{};

        format_to(w, fmt, std::forward<Args>(args)...);
        console::put_character('\n');
    };
}; // namespace std