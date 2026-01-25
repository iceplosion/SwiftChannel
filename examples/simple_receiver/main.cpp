#include <swiftchannel/swiftchannel.hpp>
#include <swiftchannel/receiver/receiver.hpp>
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>

struct PriceUpdate {
    int instrument_id;
    double bid;
    double ask;
    int64_t timestamp;
};

std::atomic<bool> running{true};

void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\n\nReceived interrupt signal, stopping...\n";
        running.store(false);
    }
}

int main() {
    std::cout << "SwiftChannel Simple Receiver Example\n";
    std::cout << "=====================================\n\n";

    // Set up signal handler
    std::signal(SIGINT, signal_handler);

    // Create a receiver for the "price_feed" channel
    swiftchannel::ChannelConfig config;
    config.ring_buffer_size = 1024 * 1024;  // 1MB
    config.max_message_size = 4096;

    swiftchannel::Receiver receiver("price_feed", config);

    std::cout << "Receiver ready. Waiting for price updates...\n";
    std::cout << "(Press Ctrl+C to stop)\n\n";

    // Define message handler
    auto handler = [](const void* data, size_t size) {
        if (size == sizeof(PriceUpdate)) {
            const PriceUpdate* update = static_cast<const PriceUpdate*>(data);
            std::cout << "Received update: "
                     << "Instrument=" << update->instrument_id
                     << " Bid=" << update->bid
                     << " Ask=" << update->ask
                     << " Timestamp=" << update->timestamp << "\n";
        } else {
            std::cout << "Received message of unexpected size: " << size << " bytes\n";
        }
    };

    // Start receiving in async mode
    auto result = receiver.start_async(handler);

    if (result.is_error()) {
        std::cerr << "Failed to start receiver!\n";
        return 1;
    }

    // Keep running until interrupted
    while (running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    receiver.stop();

    // Print statistics
    auto stats = receiver.get_stats();
    std::cout << "\nStatistics:\n";
    std::cout << "  Messages received: " << stats.messages_received << "\n";
    std::cout << "  Bytes received: " << stats.bytes_received << "\n";
    std::cout << "  Errors: " << stats.errors << "\n";

    std::cout << "\nReceiver finished.\n";
    return 0;
}
