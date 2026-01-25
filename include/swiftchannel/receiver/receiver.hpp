#pragma once

#include "swiftchannel/common/types.hpp"
#include "swiftchannel/common/error.hpp"
#include "swiftchannel/sender/config.hpp"

#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>

namespace swiftchannel {

// Receiver implementation (compiled, not header-only)
// Handles the lifecycle, polling, and message dispatch
class Receiver {
public:
    using MessageHandler = std::function<void(const void* data, size_t size)>;

    // Create a receiver for a named channel
    explicit Receiver(const std::string& channel_name,
                     const ChannelConfig& config = {});

    ~Receiver();

    // Non-copyable, non-movable (due to thread management)
    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;
    Receiver(Receiver&&) = delete;
    Receiver& operator=(Receiver&&) = delete;

    // Start receiving messages (blocking, runs in current thread)
    Result<void> start(MessageHandler handler);

    // Start receiving messages in a background thread
    Result<void> start_async(MessageHandler handler);

    // Stop receiving
    void stop();

    // Check if receiver is running
    [[nodiscard]] bool is_running() const noexcept;

    // Poll for one message (non-blocking)
    Result<bool> poll_one(MessageHandler handler);

    // Get channel name
    [[nodiscard]] const std::string& channel_name() const noexcept;

    // Get statistics
    struct Stats {
        uint64_t messages_received;
        uint64_t bytes_received;
        uint64_t errors;
        uint64_t buffer_full_count;
    };

    [[nodiscard]] Stats get_stats() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace swiftchannel
