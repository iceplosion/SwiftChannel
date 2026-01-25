#pragma once

#include <cstddef>
#include <cstdint>

namespace swiftchannel {

// Default configuration values
struct ChannelConfig {
    // Ring buffer size (must be power of 2)
    size_t ring_buffer_size = 1024 * 1024;  // 1MB default

    // Maximum message size
    size_t max_message_size = 64 * 1024;    // 64KB default

    // Flags
    uint64_t flags = 0;

    // Timeout for operations (microseconds, 0 = no timeout)
    uint64_t timeout_us = 0;

    // Enable checksum validation
    bool enable_checksum = false;

    // Overwrite behavior when buffer is full
    bool overwrite_on_full = false;

    // Validate configuration
    constexpr bool is_valid() const noexcept {
        // Ring buffer size must be power of 2
        if (!is_power_of_two(ring_buffer_size)) {
            return false;
        }

        // Max message size must fit in ring buffer
        if (max_message_size >= ring_buffer_size / 2) {
            return false;
        }

        // Minimum sizes
        if (ring_buffer_size < 4096 || max_message_size < 64) {
            return false;
        }

        return true;
    }

private:
    static constexpr bool is_power_of_two(size_t value) noexcept {
        return value != 0 && (value & (value - 1)) == 0;
    }
};

// Validate at compile time
static_assert(ChannelConfig{}.is_valid(), "Default config must be valid");

} // namespace swiftchannel
