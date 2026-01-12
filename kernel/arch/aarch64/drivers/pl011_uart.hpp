#pragma once
#include "common/std/stdint.hpp"

class PL011_UART {
public:
    explicit PL011_UART(std::uintptr_t base);

    void put_character(char c) const;
    void put_string(const char* str) const;

private:
    volatile std::uint32_t* m_dr; // Data register
    volatile std::uint32_t* m_fr; // Flag register
};