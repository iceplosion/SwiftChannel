#include "receiver_impl.hpp"
#include "swiftchannel/sender/channel.hpp"
#include "swiftchannel/sender/ring_buffer.hpp"

#include <chrono>
#include <thread>

namespace swiftchannel {

// Receiver::Impl implementation
class Receiver::Impl {
public:
    explicit Impl(const std::string& channel_name, const ChannelConfig& config)
        : channel_name_(channel_name)
        , config_(config)
        , running_(false)
    {
        // Open the existing channel (created by sender)
        auto result = Channel::open(channel_name, config);
        if (result.is_ok()) {
            channel_ = std::make_unique<Channel>(std::move(result.value()));
        }

        stats_ = {};
    }

    ~Impl() {
        stop();
    }

    Result<void> start(MessageHandler handler) {
        if (!channel_ || !channel_->is_open()) {
            return Result<void>(ErrorCode::ChannelNotFound);
        }

        running_.store(true, std::memory_order_release);

        // Message buffer (reuse to avoid allocations)
        std::vector<uint8_t> buffer(config_.max_message_size);

        while (running_.load(std::memory_order_acquire)) {
            size_t size = buffer.size();

            if (channel_->ring_buffer()->try_read(buffer.data(), size, channel_->header())) {
                // Successfully read a message
                handler(buffer.data(), size);
                stats_.messages_received++;
                stats_.bytes_received += size;
            } else {
                // No messages available, yield CPU
                std::this_thread::yield();
                // Or use a short sleep: std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        }

        return Result<void>();
    }

    Result<void> start_async(MessageHandler handler) {
        if (worker_thread_.joinable()) {
            return Result<void>(ErrorCode::InvalidOperation);
        }

        worker_thread_ = std::thread([this, handler = std::move(handler)]() {
            this->start(handler);
        });

        return Result<void>();
    }

    void stop() {
        running_.store(false, std::memory_order_release);

        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }

    bool is_running() const noexcept {
        return running_.load(std::memory_order_acquire);
    }

    Result<bool> poll_one(MessageHandler handler) {
        if (!channel_ || !channel_->is_open()) {
            return Result<bool>(ErrorCode::ChannelNotFound);
        }

        std::vector<uint8_t> buffer(config_.max_message_size);
        size_t size = buffer.size();

        if (channel_->ring_buffer()->try_read(buffer.data(), size, channel_->header())) {
            handler(buffer.data(), size);
            stats_.messages_received++;
            stats_.bytes_received += size;
            return Result<bool>(true);
        }

        return Result<bool>(false);
    }

    const std::string& channel_name() const noexcept {
        return channel_name_;
    }

    Receiver::Stats get_stats() const noexcept {
        return stats_;
    }

private:
    std::string channel_name_;
    ChannelConfig config_;
    std::unique_ptr<Channel> channel_;
    std::atomic<bool> running_;
    std::thread worker_thread_;
    Receiver::Stats stats_;
};

// Receiver public API implementation
Receiver::Receiver(const std::string& channel_name, const ChannelConfig& config)
    : impl_(std::make_unique<Impl>(channel_name, config))
{}

Receiver::~Receiver() = default;

Result<void> Receiver::start(MessageHandler handler) {
    return impl_->start(std::move(handler));
}

Result<void> Receiver::start_async(MessageHandler handler) {
    return impl_->start_async(std::move(handler));
}

void Receiver::stop() {
    impl_->stop();
}

bool Receiver::is_running() const noexcept {
    return impl_->is_running();
}

Result<bool> Receiver::poll_one(MessageHandler handler) {
    return impl_->poll_one(std::move(handler));
}

const std::string& Receiver::channel_name() const noexcept {
    return impl_->channel_name();
}

Receiver::Stats Receiver::get_stats() const noexcept {
    return impl_->get_stats();
}

} // namespace swiftchannel
