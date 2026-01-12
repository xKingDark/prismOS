#pragma once
#include "stdint.hpp"

namespace std {
    class string {
    private:
        char*  data{};
        size_t length{0};
        size_t capacity{0};

        // Geometric growth to prevent heap fragmentation
        // Returns the next power of 2 or adequate size
        [[nodiscard]] size_t recommend_size(const size_t new_len) const {
            size_t new_cap = (capacity == 0) ? 16 : capacity;
            while (new_cap <= new_len) new_cap *= 2;

            return new_cap;
        };

    public:
        string() : capacity(16) {
            this->data    = new char[this->capacity];
            this->data[0] = '\0';
        };

        explicit string(const char* s) {
            if (!s) {
                capacity = 16;
                data     = new char[capacity];
                data[0]  = '\0';
            }
            else {
                append(s);
            };
        };

        // Copy Constructor
        string(const string& other) : length(other.length), capacity(other.capacity) {
            this->data = new char[capacity];
            for (size_t i = 0; i <= length; i++) this->data[i] = other.data[i];
        };

        // Move Constructor
        string(string&& other) noexcept
            : data(other.data), length(other.length), capacity(other.capacity) {
            other.data     = nullptr;
            other.length   = 0;
            other.capacity = 0;
        };

        ~string() {
            if (data != nullptr)
                delete[] data;
        };

        // Operators
        string& operator=(const string& other) {
            if (this != &other) {
                const auto new_data = new char[other.capacity];
                for (size_t i = 0; i <= other.length; i++) new_data[i] = other.data[i];

                delete[] data;
                data     = new_data;
                length   = other.length;
                capacity = other.capacity;
            };
            return *this;
        };

        string& operator=(string&& other) noexcept {
            if (this != &other) {
                delete[] data;
                data     = other.data;
                length   = other.length;
                capacity = other.capacity;

                other.data     = nullptr;
                other.length   = 0;
                other.capacity = 0;
            };
            return *this;
        };

        string& operator+=(const string& other) {
            append(other);
            return *this;
        };

        // Accessors
        [[nodiscard]] size_t size() const {
            return length;
        };
        [[nodiscard]] const char* c_str() const {
            return data ? data : "";
        };
        [[nodiscard]] bool empty() const {
            return length == 0;
        };

        // Core Logic
        void resize(const size_t new_cap) {
            if (new_cap <= capacity)
                return;

            const auto new_data = new char[new_cap];
            if (data) {
                for (size_t i = 0; i <= length; ++i) {
                    new_data[i] = data[i];
                };
                delete[] data;
            }
            else {
                new_data[0] = '\0';
            };

            data     = new_data;
            capacity = new_cap;
        };

        void append(const char c) {
            if (length + 1 >= capacity)
                resize(recommend_size(length + 1));

            data[length++] = c;
            data[length]   = '\0';
        };

        void append(const char* s) {
            if (!s)
                return;

            size_t s_len = 0;
            while (s[s_len]) s_len++; // Manual strlen

            if (length + s_len >= capacity)
                resize(recommend_size(length + s_len));

            for (size_t i = 0; i < s_len; ++i) {
                data[length + i] = s[i];
            };

            length       += s_len;
            data[length]  = '\0';
        };

        void append(const string& other) {
            if (other.length == 0)
                return;

            if (length + other.length >= capacity)
                resize(recommend_size(length + other.length));

            for (size_t i = 0; i < other.length; ++i) {
                data[length + i] = other.data[i];
            };

            length       += other.length;
            data[length]  = '\0';
        };

        // Numeric Conversions
        // Signed integers
        void append(const int64_t value) {
            if (value == 0) {
                append('0');
                return;
            };

            if (value < 0) {
                append('-');
                // Convert to unsigned safely to handle INT64_MIN
                append_unsigned(0 - static_cast<uint64_t>(value));
            }
            else {
                append_unsigned(static_cast<uint64_t>(value));
            };
        };

        // Unsigned integers (The core logic)
        void append_unsigned(uint64_t value) {
            if (value == 0) {
                append('0');
                return;
            };

            char buffer[24];
            int  i = 0;
            while (value > 0) {
                buffer[i++]  = '0' + (value % 10);
                value       /= 10;
            };

            if (length + i >= capacity)
                resize(recommend_size(length + i));

            while (i > 0) {
                data[length++] = buffer[--i];
            };

            data[length] = '\0';
        };

        // Overloads to route to correct handler
        void append(const uint64_t value) {
            append_unsigned(value);
        };
        void append(const int32_t value) {
            append(static_cast<int64_t>(value));
        };
        void append(const uint32_t value) {
            append_unsigned(value);
        };
        void append(const size_t value) {
            append_unsigned(value);
        };
        void append(const long value) {
            append(static_cast<int64_t>(value));
        };
    };

    // Concatenation operator
    inline string operator+(string lhs, const string& rhs) {
        lhs += rhs;
        return lhs;
    };
}; // namespace std