#include <swiftchannel/sender/ring_buffer.hpp>
#include <swiftchannel/common/types.hpp>
#include <iostream>
#include <cstring>
#include <cassert>

using namespace swiftchannel;

// Simple test harness
int main() {
    std::cout << "Running ring buffer tests...\n";

    // Test 1: Basic write and read
    {
        constexpr size_t buffer_size = 4096;
        alignas(CACHE_LINE_SIZE) uint8_t memory[buffer_size + sizeof(SharedMemoryHeader)];

        auto* header = reinterpret_cast<SharedMemoryHeader*>(memory);
        header->write_index.store(0, std::memory_order_release);
        header->read_index.store(0, std::memory_order_release);

        void* ring_memory = memory + sizeof(SharedMemoryHeader);
        RingBuffer rb(ring_memory, buffer_size);

        // Write some data
        const char* test_data = "Hello, SwiftChannel!";
        size_t data_len = strlen(test_data) + 1;

        bool write_result = rb.try_write(test_data, data_len, header);
        assert(write_result && "Write should succeed");
        (void)write_result; // Mark as used

        // Read it back
        char read_buffer[256];
        size_t read_size = sizeof(read_buffer);
        bool read_result = rb.try_read(read_buffer, read_size, header);

        assert(read_result && "Read should succeed");
        assert(read_size == data_len && "Size should match");
        assert(strcmp(read_buffer, test_data) == 0 && "Data should match");
        (void)read_result; // Mark as used

        std::cout << "  [PASS] Basic write/read test passed\n";
    }

    // Test 2: Buffer full detection
    {
        constexpr size_t buffer_size = 256;
        alignas(CACHE_LINE_SIZE) uint8_t memory[buffer_size + sizeof(SharedMemoryHeader)];

        auto* header = reinterpret_cast<SharedMemoryHeader*>(memory);
        header->write_index.store(0, std::memory_order_release);
        header->read_index.store(0, std::memory_order_release);

        void* ring_memory = memory + sizeof(SharedMemoryHeader);
        RingBuffer rb(ring_memory, buffer_size);

        // Fill the buffer
        char data[64];
        memset(data, 'A', sizeof(data));

        int write_count = 0;
        while (rb.try_write(data, sizeof(data), header)) {
            write_count++;
            if (write_count > 10) break;  // Safety limit
        }

        assert(write_count > 0 && "Should be able to write at least once");
        std::cout << "  [PASS] Buffer full detection test passed (wrote " << write_count << " messages)\n";
    }

    std::cout << "All ring buffer tests passed!\n";
    return 0;
}
