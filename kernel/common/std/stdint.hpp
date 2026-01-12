#pragma once

namespace std {
    // Signed integers
    using int8_t  = signed char;
    using int16_t = short;
    using int32_t = int;
    using int64_t = long long;

    // Unsigned integers
    using uint8_t  = unsigned char;
    using uint16_t = unsigned short;
    using uint32_t = unsigned int;
    using uint64_t = unsigned long long;

    // Pointer-sized integers
    using intptr_t  = long;
    using uintptr_t = unsigned long;

    // Size types
    using size_t    = uintptr_t;
    using ptrdiff_t = intptr_t;
}; // namespace std