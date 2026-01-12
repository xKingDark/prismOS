#include "thread.hpp"
#include "common/std/print.hpp"

// Configuration
constexpr int MAX_THREADS = 16;

// A proper Ring Buffer
static Thread* thread_queue[MAX_THREADS];
static int     queue_head   = 0; // Read from here
static int     queue_tail   = 0; // Write to here
static int     active_count = 0; // Number of runnable threads in queue

// Pointer to the currently executing thread
Thread* current_thread = nullptr;

// Helper: Ring Buffer Enqueue
static void enqueue(Thread* t) {
    if (active_count >= MAX_THREADS) {
        std::println("Scheduler queue full! Dropping thread.");
        return;
    };

    thread_queue[queue_tail] = t;
    queue_tail               = (queue_tail + 1) % MAX_THREADS;
    active_count++;
};

// Helper: Ring Buffer Dequeue
static Thread* dequeue() {
    if (active_count == 0)
        return nullptr;

    Thread* t  = thread_queue[queue_head];
    queue_head = (queue_head + 1) % MAX_THREADS;
    active_count--;
    return t;
};

extern "C" [[noreturn]] void exit_thread() {
    // Mark as dead so join() knows we are done
    if (current_thread)
        current_thread->state = ThreadState::DEAD;

    // Switch straight to the next thread WITHOUT enqueueing ourselves
    schedule();

    // We should never reach here (stack is gone/we are stopped)
    while (true) {
        asm volatile("wfe");
    };
};

extern "C" [[noreturn]] void thread_trampoline(void (*func)(void*), void* arg) {
    func(arg);

    exit_thread();
};

extern "C" void spawn_thread(Thread* t, void (*func)(void*), void* arg) {
    // Calculate Stack Pointer (Top of stack, growing down)
    auto* sp_raw = t->stack + t->stack_size;

    // Align to 16 bytes (AArch64 Requirement)
    auto* sp_aligned =
        reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(sp_raw) & ~0xFULL);

    // Setup Context
    t->ctx.sp         = reinterpret_cast<std::uint64_t>(sp_aligned);
    t->ctx.pc         = reinterpret_cast<std::uint64_t>(thread_trampoline);
    t->ctx.initial_x0 = reinterpret_cast<std::uint64_t>(func);
    t->ctx.initial_x1 = reinterpret_cast<std::uint64_t>(arg);

    // Initialize callee-saved regs to 0 (good for debugging)
    t->ctx.x19 = 0;
    t->ctx.x20 = 0;
    t->ctx.x21 = 0;
    t->ctx.x22 = 0;
    t->ctx.x23 = 0;
    t->ctx.x24 = 0;
    t->ctx.x25 = 0;
    t->ctx.x26 = 0;
    t->ctx.x27 = 0;
    t->ctx.x28 = 0;
    t->ctx.x29 = 0;
    t->ctx.x30 = 0;

    // Add to run queue
    enqueue(t);
};

extern "C" void schedule() {
    // Get next thread
    Thread* next_thread = dequeue();

    // Skip any DEAD threads that might have snuck into the queue
    while (next_thread && next_thread->state == ThreadState::DEAD) {
        next_thread = dequeue();
    };

    if (!next_thread) {
        // We are yielding, but no one else is ready.
        // If current thread is valid and runnable, just keep running it.
        if (current_thread && current_thread->state != ThreadState::DEAD)
            return;

        // Current thread is DEAD (exiting) and queue is empty.
        // The system is now idle. In a real OS, we sleep or wait for interrupts.
        std::println("System Idle: No runnable threads.");

        // Unset current thread so we don't try to save context for a dead thread later
        current_thread = nullptr;

        while (true) {
            asm volatile("wfe"); // Wait For Event (save power)
        };
    };

    Thread* old_thread = current_thread;
    current_thread     = next_thread;

    // Context Switch:
    // If old_thread is null (boot or idle wake-up) or DEAD,
    // we must ensure we don't rely on it saving state meaningfully.
    ThreadContext* old_ctx_ptr = nullptr;
    if (old_thread)
        old_ctx_ptr = &old_thread->ctx;

    context_switch(old_ctx_ptr, &next_thread->ctx);
};

extern "C" void yield() {
    // Put current thread back in queue (Round Robin)
    if (current_thread) {
        enqueue(current_thread);
    };

    // Switch to next
    schedule();
};