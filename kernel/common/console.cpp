#include "console.hpp"
#if defined(__aarch64__)
void console::initialize() {
    static PL011_UART instance{ 0x09000000 };
    m_uart = &instance;
};

void console::put_character(const char c) {
    m_uart->put_character(c);
};

void console::put_string(const char* str) {
    m_uart->put_string(str);
};
#endif
