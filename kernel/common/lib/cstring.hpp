#pragma once
#include <common/std/stdint.hpp>

extern "C" int         strcmp(const char* a, const char* b);
extern "C" int         strncmp(const char* a, const char* b, std::size_t n);
extern "C" char*       strstr(const char* haystack, const char* needle);
extern "C" std::size_t strlen(const char* str);
