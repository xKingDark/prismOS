#include "pl011_uart.hpp"

#define UARTFR_TXFF (1 << 5) // Transmit FIFO full

PL011_UART::PL011_UART(const std::uintptr_t base) {
    m_dr = reinterpret_cast<volatile std::uint32_t*>(base + 0x00);
    m_fr = reinterpret_cast<volatile std::uint32_t*>(base + 0x18);
};

void PL011_UART::put_character(const char c) const {
    // Wait until FIFO not full
    while (*m_fr & UARTFR_TXFF) {};
    *m_dr = c;
};

void PL011_UART::put_string(const char* str) const {
    while (*str) {
        if (*str == '\n')
            this->put_character('\r'); // CRLF for terminals

        this->put_character(*str++);
    };
};