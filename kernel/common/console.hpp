#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#if defined(__aarch64__)
#include <arch/aarch64/drivers/pl011_uart.hpp>
#endif

class console {
public:
    static void initialize();
    static void put_character(char c);
    static void put_string(const char* str);

private:
#if defined(__aarch64__)
    static inline PL011_UART* m_uart{nullptr};
#endif
};

#endif // CONSOLE_HPP