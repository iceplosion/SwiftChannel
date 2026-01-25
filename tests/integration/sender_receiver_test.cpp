#include <swiftchannel/swiftchannel.hpp>
#include <swiftchannel/receiver/receiver.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <cassert>

using namespace swiftchannel;

struct TestData {
    int sequence;
    double timestamp;
    char payload[32];
};

int main() {
    std::cout << "Running sender/receiver integration test...\n";

    const std::string channel_name = "test_channel_integration";
    ChannelConfig config;
    config.ring_buffer_size = 1024 * 64;  // 64KB
    config.max_message_size = 1024;

    std::atomic<int> messages_received{0};
    std::atomic<bool> receiver_ready{false};

    // Start receiver in a separate thread
    std::thread receiver_thread([&]() {
        Receiver receiver(channel_name, config);
        receiver_ready.store(true);

        auto handler = [&](const void* data, size_t size) {
            const TestData* msg = static_cast<const TestData*>(data);
            std::cout << "  Received message #" << msg->sequence
                     << " with payload: " << msg->payload << "\n";
            messages_received.fetch_add(1);
        };

        // Run for a short time
        receiver.start_async(handler);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        receiver.stop();
    });

    // Wait for receiver to be ready
    while (!receiver_ready.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Send messages
    {
        Sender sender(channel_name, config);

        if (!sender.is_ready()) {
            std::cerr << "Sender not ready!\n";
            receiver_thread.join();
            return 1;
        }

        const int num_messages = 10;
        for (int i = 0; i < num_messages; ++i) {
            TestData msg;
            msg.sequence = i;
            msg.timestamp = static_cast<double>(i) * 0.1;
            snprintf(msg.payload, sizeof(msg.payload), "Message_%d", i);

            auto result = sender.send(msg);
            if (result.is_ok()) {
                std::cout << "  Sent message #" << i << "\n";
            } else {
                std::cerr << "  Failed to send message #" << i
                         << " error: " << static_cast<int>(result.error()) << "\n";
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    // Wait for receiver to finish
    receiver_thread.join();

    std::cout << "\nTest summary:\n";
    std::cout << "  Messages received: " << messages_received.load() << "\n";

    if (messages_received.load() > 0) {
        std::cout << "Integration test PASSED!\n";
        return 0;
    } else {
        std::cerr << "Integration test FAILED - no messages received\n";
        return 1;
    }
}
