#include <common/lib/memory.hpp>
#include <common/std/atomic.hpp>
#include <common/std/limits.hpp>
#include <common/std/print.hpp>
#include <common/std/thread.hpp>

#include <common/drivers/fdt.hpp>
#include <common/drivers/virtio.hpp>

std::atomic<int> l{0};

static Thread main_thread_obj;

extern "C" void kernel_main() {
    std::println("Main thread starting...");
    std::println("Max memory: {} MB", total_heap_size() / (1024 * 1024));

    // We don't need to allocate a stack because we are ALREADY running on the boot stack.
    // The scheduler just needs a place to save registers (ctx).
    main_thread_obj.state = ThreadState::RUNNING;
    main_thread_obj.stack = nullptr;          // Indicates we shouldn't delete this stack
    current_thread        = &main_thread_obj; // Tell the scheduler "I am the current thread"

    std::thread threads[4];

    for (int i = 0; i < 4; ++i) {
        threads[i] = std::thread([=] {
            for (int k = 0; k < 5; k++) {
                l.fetch_add(1, std::memory_order_relaxed);
                std::println("Thread {}: Count {}", i, l.load(std::memory_order_relaxed));

                yield();
            };
        });
    };

    std::println("All threads spawned. Waiting for completion...");

    for (auto& thread : threads) {
        thread.join();
    };

    std::println("All threads finished. Safe to shutdown.");
};

/*extern "C" void kernel_main() {
    // We don't need to allocate a stack because we are ALREADY running on the boot stack.
    // The scheduler just needs a place to save registers (ctx).
    main_thread_obj.state = ThreadState::RUNNING;
    main_thread_obj.stack = nullptr; // Indicates we shouldn't delete this stack
    current_thread = &main_thread_obj; // Tell the scheduler "I am the current thread"

    // Find VirtIO Network Device (ID = 1)
    if (const std::uint64_t net_base = fdt::find_virtio_device(1); net_base != 0) {
        std::println("Found Network Card at {}", reinterpret_cast<void *>(net_base));
        virtio_net_init(net_base);
        virtio_net_init_rx();
    }
    else {
        std::println("No Network Card found!");
    };

    while (true) {
        virtio_net_poll();
        yield();
    };
};*/
