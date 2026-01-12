#include "cstring.hpp"
#include "common/std/stdint.hpp"

extern "C" int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        ++a;
        ++b;
    };

    return static_cast<unsigned char>(*a) - static_cast<unsigned char>(*b);
};

extern "C" int strncmp(const char* a, const char* b, std::size_t n) {
    for (; n > 0; --n, ++a, ++b) {
        const auto ca = static_cast<unsigned char>(*a);
        const auto cb = static_cast<unsigned char>(*b);

        if (ca != cb)
            return ca - cb;

        if (ca == '\0')
            return 0;
    };

    return 0;
};

extern "C" char* strstr(const char* haystack, const char* needle) {
    if (*needle == '\0')
        return const_cast<char*>(haystack);

    for (const char* h = haystack; *h != '\0'; ++h) {
        const char* nh = h;
        const char* nn = needle;
        while (*nn && *nh && *nn == *nh) {
            ++nn;
            ++nh;
        };

        if (*nn == '\0')
            return const_cast<char*>(h);
    };

    return nullptr;
};

extern "C" std::size_t strlen(const char* str) {
    std::size_t length = 0;
    while (*str != '\0') {
        length++;
        str++;
    };

    return length;
};