#ifndef ATOMIC_HPP
#define ATOMIC_HPP

namespace std {
    // Standard memory order models
    enum class memory_order {
        memory_order_relaxed = __ATOMIC_RELAXED,
        memory_order_consume = __ATOMIC_CONSUME,
        memory_order_acquire = __ATOMIC_ACQUIRE,
        memory_order_release = __ATOMIC_RELEASE,
        memory_order_acq_rel = __ATOMIC_ACQ_REL,
        memory_order_seq_cst = __ATOMIC_SEQ_CST
    };

    inline constexpr auto memory_order_relaxed = memory_order::memory_order_relaxed;
    inline constexpr auto memory_order_consume = memory_order::memory_order_consume;
    inline constexpr auto memory_order_acquire = memory_order::memory_order_acquire;
    inline constexpr auto memory_order_release = memory_order::memory_order_release;
    inline constexpr auto memory_order_acq_rel = memory_order::memory_order_acq_rel;
    inline constexpr auto memory_order_seq_cst = memory_order::memory_order_seq_cst;

    template <typename T>
    class atomic {
    private:
        T _val;

    public:
        atomic(T initial_val = 0) : _val(initial_val) {};

        // Disable copy/move
        atomic(const atomic&) = delete;

        atomic& operator=(const atomic&) = delete;

        T load(memory_order order = memory_order_seq_cst) const volatile {
            return __atomic_load_n(&_val, static_cast<int>(order));
        };

        void store(T desired, memory_order order = memory_order_seq_cst) volatile {
            __atomic_store_n(&_val, desired, static_cast<int>(order));
        };

        T fetch_add(T arg, memory_order order = memory_order_seq_cst) volatile {
            return __atomic_fetch_add(&_val, arg, static_cast<int>(order));
        };

        T fetch_sub(T arg, memory_order order = memory_order_seq_cst) volatile {
            return __atomic_fetch_sub(&_val, arg, static_cast<int>(order));
        };

        bool compare_exchange_weak(
            T& expected, T desired, memory_order success = memory_order_seq_cst,
            memory_order failure = memory_order_seq_cst
        ) volatile {
            return __atomic_compare_exchange_n(
                &_val, &expected, desired, true, static_cast<int>(success),
                static_cast<int>(failure)
            );
        };

        // Operators
        T operator++(int) volatile {
            return fetch_add(1);
        };
        T operator--(int) volatile {
            return fetch_sub(1);
        };
        operator T() const volatile {
            return load();
        };

        T operator=(T desired) volatile {
            store(desired);
            return desired;
        };
    };
}; // namespace std

#endif // ATOMIC_HPP
