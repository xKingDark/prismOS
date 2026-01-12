#pragma once
#include <common/std/stdint.hpp>

extern "C" void set_heap(std::uint64_t mem_base, std::uint64_t mem_size);

extern "C" std::size_t total_heap_size();
extern "C" std::size_t used_heap();
extern "C" std::size_t free_heap();

extern "C" void* malloc(std::size_t size);
extern "C" void  free(void* ptr);
extern "C" void* realloc(void* ptr, std::size_t size);

extern "C" void* memset(void* dest, int c, std::size_t n);
extern "C" void* memcpy(void* dest, const void* src, std::size_t n);

namespace std {
    inline void* memset(void* dest, const int c, const std::size_t n) {
        return __builtin_memset(dest, c, n);
    };

    inline void* memcpy(void* dest, const void* src, const std::size_t n) {
        return __builtin_memcpy(dest, src, n);
    };
}; // namespace std