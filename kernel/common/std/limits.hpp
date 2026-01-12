#pragma once
#include "stdint.hpp"

namespace std {
    inline constexpr int8_t  INT8_MIN  = -128;
    inline constexpr int8_t  INT8_MAX  = 127;
    inline constexpr int16_t INT16_MIN = -32768;
    inline constexpr int16_t INT16_MAX = 32767;
    inline constexpr int32_t INT32_MIN = -2147483648;
    inline constexpr int32_t INT32_MAX = 2147483647;
    inline constexpr int64_t INT64_MIN = -9223372036854775807LL - 1;
    inline constexpr int64_t INT64_MAX = 9223372036854775807LL;

    inline constexpr uint8_t  UINT8_MAX  = 0xFF;
    inline constexpr uint16_t UINT16_MAX = 0xFFFF;
    inline constexpr uint32_t UINT32_MAX = 0xFFFFFFFFu;
    inline constexpr uint64_t UINT64_MAX = 0xFFFFFFFFFFFFFFFFull;

    template <typename T>
    struct numeric_limits;

    template <>
    struct numeric_limits<int8_t> {
        static constexpr int8_t min() noexcept {
            return INT8_MIN;
        };
        static constexpr int8_t max() noexcept {
            return INT8_MAX;
        };
    };

    template <>
    struct numeric_limits<uint8_t> {
        static constexpr uint8_t min() noexcept {
            return 0;
        };
        static constexpr uint8_t max() noexcept {
            return UINT8_MAX;
        };
    };

    template <>
    struct numeric_limits<int16_t> {
        static constexpr int16_t min() noexcept {
            return INT16_MIN;
        };
        static constexpr int16_t max() noexcept {
            return INT16_MAX;
        };
    };

    template <>
    struct numeric_limits<uint16_t> {
        static constexpr uint16_t min() noexcept {
            return 0;
        };
        static constexpr uint16_t max() noexcept {
            return UINT16_MAX;
        };
    };

    template <>
    struct numeric_limits<int32_t> {
        static constexpr int32_t min() noexcept {
            return INT32_MIN;
        };
        static constexpr int32_t max() noexcept {
            return INT32_MAX;
        };
    };

    template <>
    struct numeric_limits<uint32_t> {
        static constexpr uint32_t min() noexcept {
            return 0;
        };
        static constexpr uint32_t max() noexcept {
            return UINT32_MAX;
        };
    };

    template <>
    struct numeric_limits<int64_t> {
        static constexpr int64_t min() noexcept {
            return INT64_MIN;
        };
        static constexpr int64_t max() noexcept {
            return INT64_MAX;
        };
    };

    template <>
    struct numeric_limits<uint64_t> {
        static constexpr uint64_t min() noexcept {
            return 0;
        };
        static constexpr uint64_t max() noexcept {
            return UINT64_MAX;
        };
    };

    template <>
    struct numeric_limits<float> {
        static constexpr float min() noexcept {
            return 1.17549435e-38F;
        };
        static constexpr float max() noexcept {
            return 3.40282347e+38F;
        };
    };

    template <>
    struct numeric_limits<double> {
        static constexpr double min() noexcept {
            return 2.2250738585072014e-308;
        };
        static constexpr double max() noexcept {
            return 1.7976931348623157e+308;
        };
    };

    template <>
    struct numeric_limits<long double> {
        static constexpr long double min() noexcept {
            return 2.2250738585072014e-308L;
        };
        static constexpr long double max() noexcept {
            return 1.7976931348623157e+308L;
        };
    };
}; // namespace std