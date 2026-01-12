#pragma once
#include "common/std/stdint.hpp"

struct ThreadContext {
    std::uint64_t sp;         // 0: Stack pointer
    std::uint64_t pc;         // 8: Program counter
    std::uint64_t x19;        // 16: Callee-saved registers
    std::uint64_t x20;        // 24
    std::uint64_t x21;        // 32
    std::uint64_t x22;        // 40
    std::uint64_t x23;        // 48
    std::uint64_t x24;        // 56
    std::uint64_t x25;        // 64
    std::uint64_t x26;        // 72
    std::uint64_t x27;        // 80
    std::uint64_t x28;        // 88
    std::uint64_t x29;        // 96: Frame pointer
    std::uint64_t x30;        // 104: Link register
    std::uint64_t initial_x0; // 112: First arg for new thread
    std::uint64_t initial_x1; // 120: Second arg for new thread
};

enum class ThreadState { UNUSED, RUNNABLE, RUNNING, DEAD };

struct Thread {
    ThreadContext ctx{};
    std::uint8_t* stack{};
    std::size_t   stack_size{};

    ThreadState state{ThreadState::UNUSED};

    ~Thread() {
        delete[] stack; // free stack memory
    };
};

// Forward declarations
extern Thread* current_thread;

extern "C" void              context_switch(ThreadContext* old_ctx, ThreadContext* new_ctx);
extern "C" [[noreturn]] void exit_thread();
extern "C" [[noreturn]] void thread_trampoline(void (*func)(void*), void* arg);
extern "C" void              spawn_thread(Thread* t, void (*func)(void*), void* arg);
extern "C" void              schedule();
extern "C" void              yield();