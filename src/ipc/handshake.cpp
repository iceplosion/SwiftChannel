#include "handshake.hpp"
#include "swiftchannel/common/version.hpp"

#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#endif

namespace swiftchannel {

void Handshake::initialize_header(SharedMemoryHeader* header,
                                  size_t ring_buffer_size,
                                  uint64_t flags) {
    std::memset(header, 0, sizeof(SharedMemoryHeader));

    header->magic = SharedMemoryHeader::MAGIC;
    header->version = PROTOCOL_VERSION.as_uint32();
    header->ring_buffer_size = ring_buffer_size;
    header->write_index.store(0, std::memory_order_release);
    header->read_index.store(0, std::memory_order_release);
    header->flags = flags;

#ifdef _WIN32
    header->sender_pid = GetCurrentProcessId();
#else
    header->sender_pid = getpid();
#endif
}

Result<void> Handshake::sender_handshake(SharedMemoryHeader* header) {
    if (!header) {
        return Result<void>(ErrorCode::InvalidOperation);
    }

    // Check if already initialized
    if (header->magic == SharedMemoryHeader::MAGIC) {
        // Validate version
        return validate_header(header);
    }

    // Not initialized - sender will initialize
    return Result<void>();
}

Result<void> Handshake::receiver_handshake(SharedMemoryHeader* header) {
    if (!header) {
        return Result<void>(ErrorCode::InvalidOperation);
    }

    // Receiver expects sender to have initialized
    if (header->magic != SharedMemoryHeader::MAGIC) {
        return Result<void>(ErrorCode::ChannelNotFound);
    }

    // Validate header
    auto result = validate_header(header);
    if (result.is_error()) {
        return result;
    }

    // Set receiver PID
#ifdef _WIN32
    header->receiver_pid = GetCurrentProcessId();
#else
    header->receiver_pid = getpid();
#endif

    return Result<void>();
}

Result<void> Handshake::validate_header(const SharedMemoryHeader* header) {
    if (!header) {
        return Result<void>(ErrorCode::InvalidOperation);
    }

    if (header->magic != SharedMemoryHeader::MAGIC) {
        return Result<void>(ErrorCode::InvalidMemoryLayout);
    }

    // Check protocol version compatibility
    Version header_version{
        static_cast<uint16_t>((header->version >> 16) & 0xFFFF),
        static_cast<uint16_t>((header->version >> 8) & 0xFF),
        static_cast<uint16_t>(header->version & 0xFF)
    };

    if (!PROTOCOL_VERSION.is_compatible_with(header_version)) {
        return Result<void>(ErrorCode::VersionMismatch);
    }

    // Validate ring buffer size (must be power of 2)
    if (header->ring_buffer_size == 0 ||
        (header->ring_buffer_size & (header->ring_buffer_size - 1)) != 0) {
        return Result<void>(ErrorCode::InvalidMemoryLayout);
    }

    return Result<void>();
}

} // namespace swiftchannel
