#pragma once

#include <cstdint>
#include <atomic>

namespace swiftchannel {

// Statistics tracking for diagnostics
struct ChannelStats {
    std::atomic<uint64_t> messages_sent{0};
    std::atomic<uint64_t> messages_received{0};
    std::atomic<uint64_t> bytes_sent{0};
    std::atomic<uint64_t> bytes_received{0};
    std::atomic<uint64_t> send_errors{0};
    std::atomic<uint64_t> receive_errors{0};
    std::atomic<uint64_t> buffer_full_events{0};
    std::atomic<uint64_t> checksum_errors{0};
};

// Global statistics (optional, for debugging)
class GlobalStats {
public:
    static GlobalStats& instance();

    void record_send(size_t bytes);
    void record_receive(size_t bytes);
    void record_error(bool is_send);

    uint64_t total_messages_sent() const;
    uint64_t total_messages_received() const;
    uint64_t total_bytes_sent() const;
    uint64_t total_bytes_received() const;

private:
    GlobalStats() = default;
    ChannelStats stats_;
};

} // namespace swiftchannel
