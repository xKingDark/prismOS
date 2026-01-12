#include "memory.hpp"

#include <common/cppruntime_support.hpp>
#include <common/std/format.hpp>
#include <common/std/print.hpp>

extern "C" void* memset(void* dest, const int c, const std::size_t n) {
    auto* p = static_cast<unsigned char*>(dest);
    for (std::size_t i = 0; i < n; ++i) p[i] = static_cast<unsigned char>(c);

    return dest;
};

extern "C" void* memcpy(void* dest, const void* src, const std::size_t n) {
    const auto d = static_cast<char*>(dest);
    const auto s = static_cast<const char*>(src);
    for (std::size_t i = 0; i < n; ++i) d[i] = s[i];

    return dest;
};

constexpr std::size_t ALLOC_ALIGN = 16; // AArch64: 16-byte stack/ABI alignment is safe
static std::size_t    align_up(const std::size_t x, const std::size_t a) {
    return (x + (a - 1)) & ~(a - 1);
};

struct FreeBlock {
    FreeBlock*  next;
    std::size_t size;
};

static FreeBlock* free_list = nullptr;

extern "C" char __text_start;
extern "C" char _image_end;
extern "C" char _heap_start;
extern "C" char _heap_end;

static char* heap_ptr = &_heap_start;
static char* heap_end = &_heap_end;

extern "C" void set_heap(const std::uint64_t mem_base, const std::uint64_t mem_size) {
    heap_end = reinterpret_cast<char*>(mem_base + mem_size);
    if (heap_end < heap_ptr)
        heap_end = heap_ptr;

    // Keep the bump pointer aligned
    heap_ptr =
        reinterpret_cast<char*>(align_up(reinterpret_cast<std::size_t>(heap_ptr), ALLOC_ALIGN));

    /*{
        // 1. Total Hardware RAM (Exactly what the bootloader told us)
        std::uint64_t total_ram_mb = mem_size / (1024 * 1024);

        // 2. Kernel "Reserved" space (From start of RAM to end of Kernel/Stack)
        // This includes that 512KB gap at the beginning!
        const std::uint64_t kernel_reserved_bytes =
    reinterpret_cast<std::uintptr_t>(&_image_end) - mem_base;

        // 3. Kernel Image Size (Just the code/data/bss)
        const std::uint64_t kernel_binary_size =
            reinterpret_cast<std::uintptr_t>(&_image_end) -
    reinterpret_cast<std::uintptr_t>(&__text_start);

        // 4. Actual Managed Heap Size
        const std::uint64_t managed_heap_bytes =
            reinterpret_cast<std::uintptr_t>(&_heap_end) -
    reinterpret_cast<std::uintptr_t>(&_heap_start);

        std::println("--- Hardware Memory Report ---");
        std::println("Total RAM:      {} MB", total_ram_mb);
        std::println("Phys Base:      {}", reinterpret_cast<void *>(mem_base));
        std::println("--- Kernel Footprint ---");
        std::println("Kernel Binary:  {} KB", kernel_binary_size / 1024);
        std::println("Reserved Space: {} KB (Includes Gap + Stack)", kernel_reserved_bytes /
    1024); std::println("--- Allocator Stats ---"); std::println("Heap Start:     {}",
    static_cast<void *>(&_heap_start)); std::println("Heap End:       {}", static_cast<void
    *>(&_heap_end)); std::println("Usable Heap:    {} MB", managed_heap_bytes / (1024 * 1024));
        std::println("-----------------------");
    };*/
};

extern "C" std::size_t total_heap_size() {
    return heap_end - &_heap_start;
};

extern "C" std::size_t used_heap() {
    return heap_ptr - &_heap_start;
};

extern "C" std::size_t free_heap() {
    return heap_end - heap_ptr;
};

// Helper to get the block from the user pointer
static FreeBlock* get_block(void* ptr) {
    return static_cast<FreeBlock*>(ptr) - 1;
};

extern "C" void* malloc(std::size_t size) {
    if (size == 0)
        size = 1;

    // Calculate total required bytes (Header + Payload)
    std::size_t total_size = sizeof(FreeBlock) + size;
    if (total_size < size) // Check for overflow before alignment
        return nullptr;

    total_size = align_up(total_size, ALLOC_ALIGN);

    // Find a free block
    FreeBlock** prev = &free_list;
    for (FreeBlock* block = free_list; block; block = block->next) {
        if (block->size >= total_size) {
            // split if large enough to avoid big internal waste
            const std::size_t remain = block->size - total_size;

            if (remain >= sizeof(FreeBlock) + ALLOC_ALIGN) {
                // split tail
                auto* tail =
                    reinterpret_cast<FreeBlock*>(reinterpret_cast<char*>(block) + total_size);
                tail->size = remain;
                tail->next = block->next;

                // Link previous to tail, giving us 'block'
                *prev       = tail;
                block->size = total_size;
            }
            else {
                // Use entire block (remove from list)
                *prev = block->next;
            };

            // return pointer just after header
            return block + 1;
        };

        prev = &block->next;
    };

    // Bump Allocation
    const auto current_addr = reinterpret_cast<std::size_t>(heap_ptr);
    const std::size_t aligned_addr = align_up(current_addr, ALLOC_ALIGN);

    const auto p = reinterpret_cast<char*>(aligned_addr);
    if (p + total_size > heap_end) {
        panic("Out of memory! System halted.");
        // panic(std::format("Out of memory! Tried to allocate {} bytes ({} requested, {}
        // overhead)",
        //     total_size, size, sizeof(FreeBlock)).c_str());
    };

    auto* block = reinterpret_cast<FreeBlock*>(p);
    block->size = total_size;

    // Update heap pointer
    heap_ptr = p + total_size;

    return block + 1;
};

extern "C" void free(void* ptr) {
    if (ptr == nullptr)
        return;

    FreeBlock* block = static_cast<FreeBlock*>(ptr) - 1;

    // Simple LIFO insertion (No coalescing)
    block->next = free_list;
    free_list   = block;
};

extern "C" void* realloc(void* ptr, const std::size_t size) {
    if (ptr == nullptr)
        return malloc(size);

    if (size == 0) {
        free(ptr);
        return nullptr;
    };

    const FreeBlock* block = get_block(ptr);

    // Calculate actual payload capacity
    const std::size_t old_total_size   = block->size;
    const std::size_t old_payload_size = old_total_size - sizeof(FreeBlock);

    // If it fits, return existing pointer
    if (size <= old_payload_size)
        return ptr; // fits in place

    // Need new block
    void* new_ptr = malloc(size);
    if (!new_ptr)
        return nullptr; // out of memory

    // copy old contents
    memcpy(new_ptr, ptr, old_payload_size);

    free(ptr);
    return new_ptr;
};