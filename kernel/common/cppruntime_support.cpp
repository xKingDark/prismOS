#include "cppruntime_support.hpp"
#include "lib/memory.hpp"
#include "std/print.hpp"

[[noreturn]] void panic(const char* msg) {
    std::println("KERNEL PANIC: {}", msg);
    while (true) {}; // halt
};

extern "C" void* __dso_handle = nullptr;
extern "C" void  _main() {
    for (const func_ptr* ctor = __init_array_start; ctor != __init_array_end; ++ctor) {
        (*ctor)();
    };
};

// Small static fallback buffer
static atexit_func_entry_t _small_buf[32];
static std::size_t         _small_cap = (sizeof(_small_buf) / sizeof(_small_buf[0]));

// Dynamic container pointer & state
static atexit_func_entry_t* exit_funcs    = _small_buf;
static std::size_t          exit_count    = 0;
static std::size_t          exit_capacity = _small_cap;

// Use a simple atomic flag
static volatile int atexit_lock = 0;

static void lock_acq() {
    while (__atomic_exchange_n(&atexit_lock, 1, __ATOMIC_ACQUIRE) == 1) {
        // Spin hint
#if defined(__aarch64__)
        asm volatile("yield" ::: "memory");
#endif
    };
};

static void lock_rel() {
    __atomic_store_n(&atexit_lock, 0, __ATOMIC_RELEASE);
};

/* Grow helper: returns 0 on success, -1 on failure */
static int ensure_capacity(const std::size_t need) {
    if (need <= exit_capacity)
        return 0;

    std::size_t new_cap = exit_capacity ? exit_capacity * 2 : 8;
    while (new_cap < need) new_cap *= 2;

    if (exit_funcs == _small_buf) {
        void* mem = malloc(new_cap * sizeof(atexit_func_entry_t));
        if (!mem)
            return -1;

        // Simple copy
        auto* new_buf = static_cast<atexit_func_entry_t*>(mem);
        for (std::size_t i = 0; i < exit_count; ++i) new_buf[i] = _small_buf[i];

        exit_funcs    = new_buf;
        exit_capacity = new_cap;
        return 0;
    };

    // Realloc handles the copy automatically
    void* mem = realloc(exit_funcs, new_cap * sizeof(atexit_func_entry_t));
    if (!mem)
        return -1;

    exit_funcs    = static_cast<atexit_func_entry_t*>(mem);
    exit_capacity = new_cap;
    return 0;
};

// atexit support stub
extern "C" int __cxa_atexit(void (*f)(void*), void* p, void* d) {
    if (!f)
        return -1;

    lock_acq();
    if (ensure_capacity(exit_count + 1) != 0) {
        lock_rel();
        return -1;
    };

    exit_funcs[exit_count].destructor_func = f;
    exit_funcs[exit_count].obj_ptr         = p;
    exit_funcs[exit_count].dso_handle      = d;
    exit_count++;
    lock_rel();
    return 0;
};

extern "C" void __cxa_finalize(const void* dso) {
    lock_acq();
    if (dso == nullptr) {
        for (long i = static_cast<long>(exit_count) - 1; i >= 0; --i) {
            if (exit_funcs[i].destructor_func)
                exit_funcs[i].destructor_func(exit_funcs[i].obj_ptr);
        };

        exit_count = 0;
    }
    else {
        for (long i = static_cast<long>(exit_count) - 1; i >= 0; --i) {
            if (exit_funcs[i].dso_handle == dso && exit_funcs[i].destructor_func) {
                exit_funcs[i].destructor_func(exit_funcs[i].obj_ptr);

                // Shift remaining entries down
                for (std::size_t j = i; j + 1 < exit_count; ++j)
                    exit_funcs[j] = exit_funcs[j + 1];

                --exit_count;
            };
        };
    };

    lock_rel();
};

extern "C" void _atexit() {
    __cxa_finalize(nullptr);
    if (exit_funcs != _small_buf) {
        free(exit_funcs);
        exit_funcs    = _small_buf;
        exit_capacity = _small_cap;
    };

    for (const func_ptr* dtor = __fini_array_start; dtor != __fini_array_end; ++dtor) {
        (*dtor)();
    };
};

extern "C" int atexit(void (*f)()) {
    return __cxa_atexit(reinterpret_cast<void (*)(void*)>(f), nullptr, nullptr);
};

// Static local guard support (for thread-safe static vars)
// The compiler builtins handle all the complex Exclusive Access logic for AArch64 automatically
extern "C" int __cxa_guard_acquire(std::uint64_t* guard) {
    // The ABI says the first byte is the initialized flag
    auto* g = reinterpret_cast<unsigned char*>(guard);

    // Check (Relaxed read is sufficient for the first check)
    if (__atomic_load_n(g, __ATOMIC_ACQUIRE) == 0) {
        // Try to set lock bit (using 1 as "pending/locking")
        // Note: Itanium ABI is complex, but for bare metal, a simple atomic swap 0->1 works for the lock.
        // However, a standard compliant implementation usually uses the second byte for locks.
        // 0=uninit, 2=init, 1=pending

        unsigned char expected = 0;
        if (__atomic_compare_exchange_n(
                g, &expected, 1, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED
            )) {
            return 1; // We acquired the lock, run the constructor
        };

        // If we failed, someone else is initializing, or it finished.
        // Wait for it to become 2.
        while (__atomic_load_n(g, __ATOMIC_ACQUIRE) != 2) {
#if defined(__aarch64__)
            asm volatile("yield");
#endif
        };
    };

    return 0; // Already initialized
};

extern "C" void __cxa_guard_release(std::uint64_t* guard) {
    auto* g = reinterpret_cast<unsigned char*>(guard);
    __atomic_store_n(g, 2, __ATOMIC_RELEASE);
};

extern "C" void __cxa_guard_abort(std::uint64_t* guard) {
    auto* g = reinterpret_cast<unsigned char*>(guard);
    __atomic_store_n(g, 0, __ATOMIC_RELEASE);
};

extern "C" void __cxa_pure_virtual() {
    panic("Pure virtual function call!");
};

void* operator new(const std::size_t size) {
    void* ptr = malloc(size);
    if (!ptr)
        panic("Out of memory!");

    return ptr;
};

void* operator new[](const std::size_t size) {
    return operator new(size);
};

void operator delete(void* ptr) noexcept {
    free(ptr);
};

// Sized deallocation (C++14+)
void operator delete(void* ptr, const std::size_t size) noexcept {
    (void)size; // We don't use size in our simple free()
    free(ptr);
};

void operator delete[](void* ptr) noexcept {
    operator delete(ptr);
};

// Sized array deallocation (C++14+)
void operator delete[](void* ptr, const std::size_t size) noexcept {
    (void)size;
    operator delete(ptr);
};