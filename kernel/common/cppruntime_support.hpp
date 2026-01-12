#pragma once
#include "std/stdint.hpp"

typedef void (*func_ptr)();

// Constructor / Destructor support
extern "C" func_ptr __init_array_start[], __init_array_end[] __attribute__((weak));
extern "C" func_ptr __fini_array_start[], __fini_array_end[] __attribute__((weak));

struct atexit_func_entry_t {
    void (*destructor_func)(void*);
    void* obj_ptr;
    void* dso_handle;
};

[[noreturn]] void panic(const char* msg);

extern "C" void _main();
extern "C" int  __cxa_atexit(void (*f)(void*), void* p, void* d);
extern "C" void __cxa_finalize(const void* dso);
extern "C" void _atexit();

void* operator new(std::size_t size);
void  operator delete(void* ptr);

void* operator new[](std::size_t size);
void  operator delete[](void* ptr);