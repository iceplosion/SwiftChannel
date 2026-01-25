#include <swiftchannel/swiftchannel.hpp>
#include <iostream>
#include <thread>
#include <chrono>

struct PriceUpdate {
    int instrument_id;
    double bid;
    double ask;
    int64_t timestamp;
};

int main() {
    std::cout << "SwiftChannel Simple Sender Example\n";
    std::cout << "===================================\n\n";

    // Create a sender for the "price_feed" channel
    swiftchannel::ChannelConfig config;
    config.ring_buffer_size = 1024 * 1024;  // 1MB
    config.max_message_size = 4096;

    swiftchannel::Sender sender("price_feed", config);

    if (!sender.is_ready()) {
        std::cerr << "Failed to create sender!\n";
        return 1;
    }

    std::cout << "Sender ready. Sending price updates...\n\n";

    // Send 20 price updates
    for (int i = 0; i < 20; ++i) {
        PriceUpdate update;
        update.instrument_id = 1000 + (i % 5);
        update.bid = 100.0 + (i * 0.5);
        update.ask = update.bid + 0.1;
        update.timestamp = std::chrono::system_clock::now().time_since_epoch().count();

        auto result = sender.send(update);

        if (result.is_ok()) {
            std::cout << "Sent update #" << i << ": "
                     << "Instrument=" << update.instrument_id
                     << " Bid=" << update.bid
                     << " Ask=" << update.ask << "\n";
        } else {
            std::cerr << "Failed to send update #" << i
                     << " (error code: " << static_cast<int>(result.error()) << ")\n";
        }

        // Send at 10Hz (100ms interval)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "\nSender finished.\n";
    return 0;
}
