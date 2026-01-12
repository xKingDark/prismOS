#pragma once
#include "common/console.hpp"
#include "string.hpp"
#include "utility.hpp"

namespace std {
    namespace detail {
        // Functor to write to a std::string
        struct string_writer {
            std::string& str;
            void         operator()(const char c) const {
                str.append(c);
            };
            void operator()(const char* s) const {
                str.append(s);
            };
        };

        // Functor to write directly to console
        struct console_writer {
            void operator()(const char c) const {
                console::put_character(c);
            };
            void operator()(const char* s) const {
                console::put_string(s);
            };
        };

        // Base Overloads for types
        template <typename Writer>
        void format_arg(Writer& out, char c) {
            out(c);
        };

        template <typename Writer>
        void format_arg(Writer& out, const char* s) {
            out(s ? s : "(null)");
        };

        template <typename Writer>
        void format_arg(Writer& out, const std::string& s) {
            out(s.c_str());
        };

        template <typename Writer>
        void format_arg(Writer& out, const bool b) {
            out(b ? "true" : "false");
        };

        // Integers
        template <typename Writer, typename T>
        void format_arg(Writer& out, T val, std::enable_if_t<std::is_integral_v<T>, int> = 0) {
            if (val == 0) {
                out('0');
                return;
            };

            // Handle Signed Negative
            if constexpr (std::is_signed<T>::value) {
                if (val < 0) {
                    out('-');
                    // Cast to unsigned equivalent to handle INT_MIN safely
                    using U   = std::make_unsigned_t<T>;
                    auto uval = static_cast<U>(0 - val); // 0 - (negative) = positive

                    char buf[24];
                    int  i = 0;

                    while (uval) {
                        buf[i++]  = '0' + (uval % 10);
                        uval     /= 10;
                    };
                    while (i) {
                        out(buf[--i]);
                    };
                    return;
                };
            };

            // Handle Positive / Unsigned
            char buf[24];
            int  i = 0;
            while (val) {
                buf[i++]  = '0' + (val % 10);
                val      /= 10;
            };

            while (i > 0) {
                out(buf[--i]);
            };
        };

        // Pointers -> Hex
        template <typename Writer>
        void format_arg(Writer& out, void* ptr) {
            out("0x");
            auto p = reinterpret_cast<uintptr_t>(ptr);
            if (p == 0) {
                out('0');
                return;
            };

            char buf[20];
            int  i = 0;
            while (p) {
                const int digit  = p % 16;
                buf[i++]         = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
                p               /= 16;
            };

            while (i > 0) {
                out(buf[--i]);
            };
        };
    }; // namespace detail

    // Recursive Format Loop
    template <typename Writer>
    void format_to_impl(Writer& out, const char* fmt) {
        while (*fmt) {
            out(*fmt++);
        };
    };

    template <typename Writer, typename T, typename... Args>
    void format_to_impl(Writer& out, const char* fmt, T&& val, Args&&... args) {
        while (*fmt) {
            if (*fmt == '{') {
                const char* next = fmt + 1;
                if (*next == '{') { // Escape {{
                    out('{');
                    fmt += 2;
                    continue;
                };

                if (*next == '}') { // Replace {}
                    detail::format_arg(out, std::forward<T>(val));
                    fmt += 2;
                    format_to_impl(out, fmt, std::forward<Args>(args)...);
                    return;
                };
            }
            else if (*fmt == '}') {
                if (*(fmt + 1) == '}') { // Escape }}
                    out('}');
                    fmt += 2;
                    continue;
                };
            };

            out(*fmt++);
        };
    };

    // Public API
    template <typename Writer, typename... Args>
    void format_to(Writer&& w, const char* fmt, Args&&... args) {
        format_to_impl(w, fmt, std::forward<Args>(args)...);
    };

    template <typename... Args>
    std::string format(const char* fmt, Args&&... args) {
        std::string           res{};
        detail::string_writer w{res};

        format_to(w, fmt, std::forward<Args>(args)...);
        return res;
    };
}; // namespace std