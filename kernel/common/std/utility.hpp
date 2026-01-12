#pragma once
namespace std {
    // Move: cast to rvalue reference
    template <typename T>
    constexpr T&& move(T& t) noexcept {
        return static_cast<T&&>(t);
    };

    // Forward: preserve value category
    template <typename T>
    constexpr T&& forward(T& t) noexcept {
        return static_cast<T&&>(t);
    };

    template <typename T>
    constexpr T&& forward(T&& t) noexcept {
        return static_cast<T&&>(t);
    };

    template <typename T>
    constexpr const T& min(const T& a, const T& b) {
        return (b < a) ? b : a;
    };

    template <typename T>
    constexpr const T& max(const T& a, const T& b) {
        return (a < b) ? b : a;
    };

    // Helper: Integral Constant
    template <typename T, T v>
    struct integral_constant {
        static constexpr T value = v;
        using value_type         = T;
        using type               = integral_constant;
        constexpr operator value_type() const noexcept {
            return value;
        }
        constexpr value_type operator()() const noexcept {
            return value;
        }
    };

    using true_type  = integral_constant<bool, true>;
    using false_type = integral_constant<bool, false>;

    // enable_if
    template <bool B, typename T = void>
    struct enable_if {};

    template <typename T>
    struct enable_if<true, T> {
        using type = T;
    };

    template <bool B, typename T = void>
    using enable_if_t = typename enable_if<B, T>::type;

    // remove_cv (Const-Volatile)
    template <typename T>
    struct remove_cv {
        using type = T;
    };
    template <typename T>
    struct remove_cv<const T> {
        using type = T;
    };
    template <typename T>
    struct remove_cv<volatile T> {
        using type = T;
    };
    template <typename T>
    struct remove_cv<const volatile T> {
        using type = T;
    };

    template <typename T>
    using remove_cv_t = typename remove_cv<T>::type;

    // is_integral
    // Helper to identify integral types
    template <typename T>
    struct is_integral_base : false_type {};

    template <>
    struct is_integral_base<bool> : true_type {};
    template <>
    struct is_integral_base<char> : true_type {};
    template <>
    struct is_integral_base<signed char> : true_type {};
    template <>
    struct is_integral_base<unsigned char> : true_type {};
    template <>
    struct is_integral_base<short> : true_type {};
    template <>
    struct is_integral_base<unsigned short> : true_type {};
    template <>
    struct is_integral_base<int> : true_type {};
    template <>
    struct is_integral_base<unsigned int> : true_type {};
    template <>
    struct is_integral_base<long> : true_type {};
    template <>
    struct is_integral_base<unsigned long> : true_type {};
    template <>
    struct is_integral_base<long long> : true_type {};
    template <>
    struct is_integral_base<unsigned long long> : true_type {};

    // Main trait (ignores const/volatile)
    template <typename T>
    struct is_integral : is_integral_base<remove_cv_t<T>> {};

    template <typename T>
    inline constexpr bool is_integral_v = is_integral<T>::value;

    // is_signed
    namespace detail {
        template <typename T, bool = is_integral<T>::value>
        struct is_signed_impl : integral_constant<bool, T(-1) < T(0)> {};

        template <typename T> // Floating point, etc. not supported here, just fail gracefully
                              // or false
                              struct is_signed_impl<T, false> : true_type {
        }; // Default to true if not integral? Usually false for non-arithmetic.
        // Note: In standard lib, is_signed works for float too.
        // For OS kernel integers, the T(-1) < T(0) check is sufficient for integral types.
    } // namespace detail

    template <typename T>
    struct is_signed : detail::is_signed_impl<T>::type {};

    // make_unsigned
    // Helper to map signed types to unsigned types
    template <typename T>
    struct make_unsigned_base {};

    template <>
    struct make_unsigned_base<char> {
        using type = unsigned char;
    };
    template <>
    struct make_unsigned_base<signed char> {
        using type = unsigned char;
    };
    template <>
    struct make_unsigned_base<short> {
        using type = unsigned short;
    };
    template <>
    struct make_unsigned_base<int> {
        using type = unsigned int;
    };
    template <>
    struct make_unsigned_base<long> {
        using type = unsigned long;
    };
    template <>
    struct make_unsigned_base<long long> {
        using type = unsigned long long;
    };
    template <>
    struct make_unsigned_base<unsigned char> {
        using type = unsigned char;
    };
    template <>
    struct make_unsigned_base<unsigned short> {
        using type = unsigned short;
    };
    template <>
    struct make_unsigned_base<unsigned int> {
        using type = unsigned int;
    };
    template <>
    struct make_unsigned_base<unsigned long> {
        using type = unsigned long;
    };
    template <>
    struct make_unsigned_base<unsigned long long> {
        using type = unsigned long long;
    };

    template <typename T>
    struct make_unsigned {
        using type = typename make_unsigned_base<remove_cv_t<T>>::type;
    };

    template <typename T>
    using make_unsigned_t = typename make_unsigned<T>::type;

    // Helper Type Traits
    template <typename T>
    struct add_rvalue_reference {
        using type = T&&;
    };

    template <typename T>
    struct add_rvalue_reference<T&> {
        using type = T&;
    };

    template <>
    struct add_rvalue_reference<void> {
        using type = void;
    };

    template <>
    struct add_rvalue_reference<const void> {
        using type = const void;
    };

    template <typename T>
    using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

    // It has no body because it is only used at compile-time for type deduction.
    template <typename T>
    add_rvalue_reference_t<T> declval() noexcept;
}; // namespace std