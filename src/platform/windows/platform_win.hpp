#pragma once

#include "swiftchannel/common/types.hpp"
#include "swiftchannel/common/error.hpp"

#ifdef _WIN32

#include <windows.h>
#include <string>

namespace swiftchannel::platform {

// Windows-specific platform utilities
class PlatformWin {
public:
    // Convert channel name to Windows shared memory name
    static std::wstring to_shared_memory_name(const std::string& channel_name);

    // Get last error as ErrorCode
    static ErrorCode get_last_error();

    // Get current process ID
    static uint32_t get_process_id();
};

} // namespace swiftchannel::platform

#endif // _WIN32
