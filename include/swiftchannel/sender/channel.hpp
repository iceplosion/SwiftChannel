#pragma once

#include "../common/types.hpp"
#include "../common/error.hpp"
#include "config.hpp"
#include "ring_buffer.hpp"

#include <string>
#include <memory>

namespace swiftchannel {

// Channel represents the shared memory region and ring buffer
// This is used internally by Sender
class Channel {
public:
    Channel() = default;
    ~Channel() { close(); }

    // Non-copyable, movable
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;
    Channel(Channel&&) noexcept = default;
    Channel& operator=(Channel&&) noexcept = default;

    // Open or create a channel
    [[nodiscard]] static Result<Channel> open(const std::string& name,
                                             const ChannelConfig& config) noexcept;

    // Check if channel is open
    [[nodiscard]] bool is_open() const noexcept {
        return shared_memory_ != nullptr;
    }

    // Get the ring buffer
    [[nodiscard]] RingBuffer* ring_buffer() noexcept {
        return ring_buffer_.get();
    }

    // Get the shared memory header
    [[nodiscard]] SharedMemoryHeader* header() noexcept {
        return header_;
    }

    // Close the channel
    void close() noexcept;

    // Get channel name
    [[nodiscard]] const std::string& name() const noexcept {
        return name_;
    }

    // Get config
    [[nodiscard]] const ChannelConfig& config() const noexcept {
        return config_;
    }

private:
    explicit Channel(std::string name, ChannelConfig config,
                    void* shared_memory, size_t total_size,
                    void* platform_handle) noexcept;

    std::string name_;
    ChannelConfig config_;
    void* shared_memory_ = nullptr;
    size_t total_size_ = 0;
    SharedMemoryHeader* header_ = nullptr;
    std::unique_ptr<RingBuffer> ring_buffer_;
    void* platform_handle_ = nullptr;  // Platform-specific handle
};

} // namespace swiftchannel
