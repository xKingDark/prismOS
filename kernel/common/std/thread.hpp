#ifndef STD_THREAD_HPP
#define STD_THREAD_HPP
#include "common/cppruntime_support.hpp" // For panic()
#include "common/lib/thread.hpp"
#include "common/std/utility.hpp"

namespace std {
    template <class T>
    struct remove_reference {
        typedef T type;
    };

    template <class T>
    struct remove_reference<T&> {
        typedef T type;
    };

    template <class T>
    struct remove_reference<T&&> {
        typedef T type;
    };

    class thread {
    public:
        using native_handle_type = Thread*;

        thread() noexcept = default;

        // The Simplified Constructor (Takes a Lambda/Callable)
        // Usage: std::thread t([=] { my_func(10); });
        template <typename Callable>
        explicit thread(Callable&& f) {
            start_thread(std::forward<Callable>(f));
        };

        // Move Constructor
        thread(thread&& other) noexcept : m_handle(other.m_handle) {
            other.m_handle = nullptr;
        };

        // Move Assignment
        thread& operator=(thread&& other) noexcept {
            if (joinable())
                panic("std::thread: overwritten while joinable!");

            m_handle       = other.m_handle;
            other.m_handle = nullptr;
            return *this;
        };

        // Disable copying
        thread(const thread&) = delete;

        thread& operator=(const thread&) = delete;

        // Destructor
        ~thread() {
            if (joinable()) {
                // In a real OS, this terminates the program.
                // In a kernel, this is a bug.
                panic("std::thread: destroyed while joinable!");
            };
        };

        void join() {
            if (!joinable())
                return;

            // Wait for DEAD state
            while (m_handle->state != ThreadState::DEAD) {
                yield();
            };

            // Clean up kernel resources
            delete[] m_handle->stack;
            delete m_handle;
            m_handle = nullptr;
        };

        void detach() {
            if (!joinable())
                return;

            m_handle = nullptr;
        };

        [[nodiscard]] bool joinable() const noexcept {
            return m_handle != nullptr;
        };
        [[nodiscard]] native_handle_type native_handle() const {
            return m_handle;
        };

    private:
        native_handle_type m_handle{nullptr};

        // Helper to move the lambda to the heap and cast to void*
        template <typename Callable>
        void start_thread(Callable&& f) {
            // Allocate Kernel Thread
            m_handle             = new Thread();
            m_handle->stack_size = 64 * 1024; // 64 KB
            m_handle->stack      = new uint8_t[m_handle->stack_size];

            // Move the lambda to the heap so it survives this scope
            // 'Decay' ensures we store the object, not a reference
            using DecayedCallable = typename remove_reference<Callable>::type;
            auto* heap_callable   = new DecayedCallable(std::forward<Callable>(f));

            spawn_thread(m_handle, &thread_entry_point<DecayedCallable>, heap_callable);
        };

        // Static Trampoline to cast void* back to Lambda*
        template <typename T>
        static void thread_entry_point(void* arg) {
            auto* callable = static_cast<T*>(arg);
            (*callable)();

            delete callable;
            // Return to kernel trampoline (which sets thread to DEAD)
        };
    };
} // namespace std

#endif // STD_THREAD_HPP
