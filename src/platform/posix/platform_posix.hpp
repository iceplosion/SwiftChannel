#pragma once

#include "swiftchannel/common/types.hpp"

#ifndef _WIN32

#include <string>
#include <cstdint>

namespace swiftchannel::platform {

// POSIX-specific platform utilities
class PlatformPosix {
public:
    // Convert channel name to POSIX shared memory name
    static std::string to_shared_memory_name(const std::string& channel_name);

    // Get error code from errno
    static ErrorCode get_last_error();

    // Get current process ID
    static uint32_t get_process_id();
};

} // namespace swiftchannel::platform

#endif // !_WIN32
