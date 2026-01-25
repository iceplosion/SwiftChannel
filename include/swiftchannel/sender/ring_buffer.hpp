#pragma once

#include "../common/types.hpp"
#include "../common/alignment.hpp"
#include <atomic>
#include <cstring>
#include <bit>
#include <chrono>
#include <cassert>

namespace swiftchannel {

// Lock-free SPSC (Single Producer Single Consumer) ring buffer
// Optimized for cache-line alignment and false sharing prevention
class RingBuffer {
public:
    RingBuffer() = delete;
    RingBuffer(void* memory, size_t size) noexcept
        : buffer_(static_cast<uint8_t*>(memory))
        , size_(size)
        , mask_(size - 1)
    {
        // Size must be power of 2
        assert(is_power_of_two(size));
        assert(is_aligned(reinterpret_cast<uintptr_t>(memory), CACHE_LINE_SIZE));
    }

    // Try to write data to the ring buffer (non-blocking, header-only)
    [[nodiscard]] inline bool try_write(const void* data, size_t data_size,
                                        SharedMemoryHeader* header) noexcept {
        const size_t total_size = sizeof(MessageHeader) + align_up(data_size, 8);

        // Check if we have enough space
        const uint64_t current_write = header->write_index.load(std::memory_order_relaxed);
        const uint64_t current_read = header->read_index.load(std::memory_order_acquire);

        const uint64_t available = size_ - (current_write - current_read);
        if (available < total_size) {
            return false;  // Buffer full
        }

        // Write message header
        MessageHeader msg_header{};
        msg_header.magic = MessageHeader::MAGIC;
        msg_header.size = static_cast<uint32_t>(data_size);
        msg_header.sequence = current_write;
        msg_header.timestamp = get_timestamp_ns();
        msg_header.checksum = 0;  // TODO: compute if enabled
        msg_header.reserved = 0;

        // Write header to ring buffer
        write_bytes(&msg_header, sizeof(msg_header), current_write);

        // Write payload
        write_bytes(data, data_size, current_write + sizeof(msg_header));

        // Update write index (release semantics for visibility)
        header->write_index.store(current_write + total_size, std::memory_order_release);

        return true;
    }

    // Read data from ring buffer (used by receiver)
    [[nodiscard]] inline bool try_read(void* data, size_t& data_size,
                                       SharedMemoryHeader* header) noexcept {
        const uint64_t current_read = header->read_index.load(std::memory_order_relaxed);
        const uint64_t current_write = header->write_index.load(std::memory_order_acquire);

        // Check if data is available
        if (current_read >= current_write) {
            return false;  // Buffer empty
        }

        // Read message header
        MessageHeader msg_header{};
        read_bytes(&msg_header, sizeof(msg_header), current_read);

        // Validate header
        if (msg_header.magic != MessageHeader::MAGIC) {
            return false;  // Corrupted
        }

        // Check if caller's buffer is large enough
        if (msg_header.size > data_size) {
            data_size = msg_header.size;  // Return required size
            return false;
        }

        // Read payload
        read_bytes(data, msg_header.size, current_read + sizeof(msg_header));

        // Update read index
        const size_t total_size = sizeof(MessageHeader) + align_up(msg_header.size, 8);
        header->read_index.store(current_read + total_size, std::memory_order_release);

        data_size = msg_header.size;
        return true;
    }

    // Get available space for writing
    [[nodiscard]] inline size_t available_write_space(const SharedMemoryHeader* header) const noexcept {
        const uint64_t current_write = header->write_index.load(std::memory_order_relaxed);
        const uint64_t current_read = header->read_index.load(std::memory_order_acquire);
        return size_ - static_cast<size_t>(current_write - current_read);
    }

    // Get available data for reading
    [[nodiscard]] inline size_t available_read_data(const SharedMemoryHeader* header) const noexcept {
        const uint64_t current_read = header->read_index.load(std::memory_order_relaxed);
        const uint64_t current_write = header->write_index.load(std::memory_order_acquire);
        return static_cast<size_t>(current_write - current_read);
    }

private:
    // Write bytes to ring buffer (handles wrap-around)
    inline void write_bytes(const void* src, size_t size, uint64_t offset) noexcept {
        const size_t pos = static_cast<size_t>(offset & mask_);
        const size_t end_pos = pos + size;

        if (end_pos <= size_) {
            // No wrap-around
            std::memcpy(buffer_ + pos, src, size);
        } else {
            // Wrap-around
            const size_t first_part = size_ - pos;
            std::memcpy(buffer_ + pos, src, first_part);
            std::memcpy(buffer_, static_cast<const uint8_t*>(src) + first_part,
                       size - first_part);
        }
    }

    // Read bytes from ring buffer (handles wrap-around)
    inline void read_bytes(void* dst, size_t size, uint64_t offset) const noexcept {
        const size_t pos = static_cast<size_t>(offset & mask_);
        const size_t end_pos = pos + size;

        if (end_pos <= size_) {
            // No wrap-around
            std::memcpy(dst, buffer_ + pos, size);
        } else {
            // Wrap-around
            const size_t first_part = size_ - pos;
            std::memcpy(dst, buffer_ + pos, first_part);
            std::memcpy(static_cast<uint8_t*>(dst) + first_part, buffer_,
                       size - first_part);
        }
    }

    // Get current timestamp in nanoseconds
    static inline uint64_t get_timestamp_ns() noexcept {
        return std::chrono::steady_clock::now().time_since_epoch().count();
    }

    uint8_t* buffer_;
    size_t size_;
    size_t mask_;
};

} // namespace swiftchannel
