#include "stats.hpp"

namespace swiftchannel {

GlobalStats& GlobalStats::instance() {
    static GlobalStats instance;
    return instance;
}

void GlobalStats::record_send(size_t bytes) {
    stats_.messages_sent.fetch_add(1, std::memory_order_relaxed);
    stats_.bytes_sent.fetch_add(bytes, std::memory_order_relaxed);
}

void GlobalStats::record_receive(size_t bytes) {
    stats_.messages_received.fetch_add(1, std::memory_order_relaxed);
    stats_.bytes_received.fetch_add(bytes, std::memory_order_relaxed);
}

void GlobalStats::record_error(bool is_send) {
    if (is_send) {
        stats_.send_errors.fetch_add(1, std::memory_order_relaxed);
    } else {
        stats_.receive_errors.fetch_add(1, std::memory_order_relaxed);
    }
}

uint64_t GlobalStats::total_messages_sent() const {
    return stats_.messages_sent.load(std::memory_order_relaxed);
}

uint64_t GlobalStats::total_messages_received() const {
    return stats_.messages_received.load(std::memory_order_relaxed);
}

uint64_t GlobalStats::total_bytes_sent() const {
    return stats_.bytes_sent.load(std::memory_order_relaxed);
}

uint64_t GlobalStats::total_bytes_received() const {
    return stats_.bytes_received.load(std::memory_order_relaxed);
}

} // namespace swiftchannel
