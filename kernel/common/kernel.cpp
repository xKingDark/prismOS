#include "std/print.hpp"
#include "std/stdint.hpp"

#include "drivers/fdt.hpp"
#include "lib/memory.hpp"

extern "C" char __bss_start[], __bss_end[];
extern "C" void _initialize(void* dtb_ptr) {
    for (char* p = __bss_start; p < __bss_end; ++p)
        *p = 0;

    fdt::initialize(dtb_ptr);
    console::initialize();

    std::uint64_t mem_base, mem_size;
    if (fdt::get_memory(mem_base, mem_size)) {
        set_heap(mem_base, mem_size);
    }
    else {
        set_heap(0x40000000, 128 * 1024 * 1024);
    };
};