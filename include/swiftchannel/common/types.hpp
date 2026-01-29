#pragma once

#include <cstdint>
#include <cstddef>
#include <string_view>
#include <chrono>
#include <atomic>

namespace swiftchannel {

// Forward declarations
class Sender;
class Receiver;
class Channel;

// Type aliases for common types
using ChannelId = std::string_view;
using MessageId = uint64_t;
using TimePoint = std::chrono::steady_clock::time_point;
using Duration = std::chrono::nanoseconds;

// Message header structure (appears before every message in the ring buffer)
struct MessageHeader {
    uint32_t magic;         // Magic number for validation
    uint32_t size;          // Payload size in bytes
    uint64_t sequence;      // Sequence number (monotonic)
    uint64_t timestamp;     // Nanoseconds since epoch
    uint32_t checksum;      // Optional checksum (0 if disabled)
    uint32_t reserved;      // Reserved for future use

    static constexpr uint32_t MAGIC = 0x53574946;  // "SWIF"
};

static_assert(sizeof(MessageHeader) == 32, "MessageHeader must be 32 bytes");
static_assert(alignof(MessageHeader) <= 8, "MessageHeader alignment");

// Shared memory layout header
struct SharedMemoryHeader {
    uint32_t magic;                 // Magic number
    uint32_t version;               // Protocol version
    uint64_t ring_buffer_size;      // Size of ring buffer in bytes
    std::atomic<uint64_t> write_index;  // Write position (atomic)
    std::atomic<uint64_t> read_index;   // Read position (atomic)
    uint32_t sender_pid;            // Sender process ID
    uint32_t receiver_pid;          // Receiver process ID
    uint64_t flags;                 // Configuration flags
    uint64_t reserved[10];          // Reserved for future use (80 bytes)

    static constexpr uint32_t MAGIC = 0x53574946;  // "SWIF"
};

static_assert(sizeof(SharedMemoryHeader) == 128, "SharedMemoryHeader must be 128 bytes");

// Configuration flags
enum class ChannelFlags : uint64_t {
    None            = 0,
    NoChecksum      = 1 << 0,   // Disable checksum validation
    Overwrite       = 1 << 1,   // Overwrite old messages if buffer full
    SingleProducer  = 1 << 2,   // Only one sender (enables optimizations)
    SingleConsumer  = 1 << 3,   // Only one receiver (enables optimizations)
};

// Callback type for message processing
using MessageCallback = void(*)(const void* data, size_t size, void* user_data);

} // namespace swiftchannel
