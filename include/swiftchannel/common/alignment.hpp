#pragma once

#include <cstddef>
#include <cstdint>
#include <new>

namespace swiftchannel {

// Cache line size (can be overridden at compile time)
#ifndef SWIFTCHANNEL_CACHE_LINE_SIZE
#define SWIFTCHANNEL_CACHE_LINE_SIZE 64
#endif

constexpr size_t CACHE_LINE_SIZE = SWIFTCHANNEL_CACHE_LINE_SIZE;

// Alignment helpers
constexpr size_t align_up(size_t value, size_t alignment) noexcept {
    return (value + alignment - 1) & ~(alignment - 1);
}

constexpr size_t align_down(size_t value, size_t alignment) noexcept {
    return value & ~(alignment - 1);
}

constexpr bool is_aligned(size_t value, size_t alignment) noexcept {
    return (value & (alignment - 1)) == 0;
}

constexpr bool is_power_of_two(size_t value) noexcept {
    return value != 0 && (value & (value - 1)) == 0;
}

// Cache-line aligned allocator helper
template<typename T>
struct alignas(CACHE_LINE_SIZE) CacheAligned {
    T value;

    CacheAligned() = default;
    CacheAligned(const T& v) : value(v) {}
    CacheAligned(T&& v) : value(std::move(v)) {}

    operator T&() noexcept { return value; }
    operator const T&() const noexcept { return value; }

    T& operator*() noexcept { return value; }
    const T& operator*() const noexcept { return value; }
};

// Ensure no false sharing for atomic variables
template<typename T>
struct alignas(CACHE_LINE_SIZE) NoPadding {
    T value;
    char padding[CACHE_LINE_SIZE - sizeof(T) % CACHE_LINE_SIZE];

    NoPadding() = default;
    NoPadding(const T& v) : value(v) {}
    NoPadding(T&& v) : value(std::move(v)) {}

    operator T&() noexcept { return value; }
    operator const T&() const noexcept { return value; }
};

} // namespace swiftchannel
