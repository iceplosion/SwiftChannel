#pragma once

#include "swiftchannel/common/types.hpp"
#include "swiftchannel/common/error.hpp"

namespace swiftchannel {

// Handshake protocol for sender-receiver synchronization
class Handshake {
public:
    // Perform handshake (sender side)
    static Result<void> sender_handshake(SharedMemoryHeader* header);

    // Perform handshake (receiver side)
    static Result<void> receiver_handshake(SharedMemoryHeader* header);

    // Validate shared memory header
    static Result<void> validate_header(const SharedMemoryHeader* header);

    // Initialize header (first time)
    static void initialize_header(SharedMemoryHeader* header,
                                  size_t ring_buffer_size,
                                  uint64_t flags);
};

} // namespace swiftchannel
