#pragma once

#include "../common/types.hpp"
#include "../common/error.hpp"
#include "config.hpp"
#include "channel.hpp"
#include "message.hpp"
#include "ring_buffer.hpp"

#include <string>
#include <memory>
#include <chrono>

namespace swiftchannel {

// Header-only Sender API
// Zero-allocation, inline ring buffer writes
// No syscalls in the fast path
class Sender {
public:
    // Create a sender for a named channel
    explicit Sender(const std::string& channel_name,
                   const ChannelConfig& config = {})
        : channel_name_(channel_name)
        , config_(config)
    {
        // Open or create the channel (this may allocate/map memory)
        auto result = Channel::open(channel_name, config);
        if (result.is_ok()) {
            channel_ = std::make_unique<Channel>(std::move(result.value()));
        }
        // If failed, channel_ remains nullptr and sends will fail
    }

    ~Sender() = default;

    // Non-copyable, movable
    Sender(const Sender&) = delete;
    Sender& operator=(const Sender&) = delete;
    Sender(Sender&&) noexcept = default;
    Sender& operator=(Sender&&) noexcept = default;

    // Check if sender is ready
    [[nodiscard]] bool is_ready() const noexcept {
        return channel_ && channel_->is_open();
    }

    // Send a typed message (header-only fast path)
    template<Sendable T>
    [[nodiscard]] inline Result<void> send(const T& message) noexcept {
        return send_bytes(&message, sizeof(T));
    }

    // Send a Message<T>
    template<Sendable T>
    [[nodiscard]] inline Result<void> send(const Message<T>& message) noexcept {
        return send_bytes(message.raw_data(), message.size());
    }

    // Send a DynamicMessage
    [[nodiscard]] inline Result<void> send(const DynamicMessage& message) noexcept {
        return send_bytes(message.data(), message.size());
    }

    // Send raw bytes (the core implementation)
    [[nodiscard]] inline Result<void> send_bytes(const void* data, size_t size) noexcept {
        if (!is_ready()) {
            return Result<void>(ErrorCode::ChannelClosed);
        }

        if (size > config_.max_message_size) {
            return Result<void>(ErrorCode::MessageTooLarge);
        }

        // Fast path: try to write directly to ring buffer
        auto* rb = channel_->ring_buffer();
        auto* header = channel_->header();

        if (rb->try_write(data, size, header)) {
            return Result<void>();  // Success
        }

        // Buffer full
        if (config_.overwrite_on_full) {
            // TODO: Implement overwrite logic
            return Result<void>(ErrorCode::ChannelFull);
        }

        return Result<void>(ErrorCode::ChannelFull);
    }

    // Try to send without blocking (returns false if would block)
    template<Sendable T>
    [[nodiscard]] inline bool try_send(const T& message) noexcept {
        return send(message).is_ok();
    }

    // Get available space in the channel
    [[nodiscard]] size_t available_space() const noexcept {
        if (!is_ready()) {
            return 0;
        }
        return channel_->ring_buffer()->available_write_space(channel_->header());
    }

    // Get channel name
    [[nodiscard]] const std::string& channel_name() const noexcept {
        return channel_name_;
    }

    // Get configuration
    [[nodiscard]] const ChannelConfig& config() const noexcept {
        return config_;
    }

private:
    std::string channel_name_;
    ChannelConfig config_;
    std::unique_ptr<Channel> channel_;
};

} // namespace swiftchannel
