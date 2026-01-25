#include "platform_win.hpp"

#ifdef _WIN32

#include "../../ipc/shared_memory.hpp"
#include "../../ipc/handshake.hpp"
#include "swiftchannel/sender/channel.hpp"
#include "swiftchannel/common/alignment.hpp"

#include <codecvt>
#include <locale>

namespace swiftchannel::platform {

std::wstring PlatformWin::to_shared_memory_name(const std::string& channel_name) {
    // Convert UTF-8 to UTF-16 and add prefix
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wide_name = L"Local\\SwiftChannel_" + converter.from_bytes(channel_name);
    return wide_name;
}

ErrorCode PlatformWin::get_last_error() {
    DWORD error = ::GetLastError();

    switch (error) {
        case ERROR_SUCCESS:
            return ErrorCode::Success;
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return ErrorCode::ChannelNotFound;
        case ERROR_ALREADY_EXISTS:
            return ErrorCode::ChannelAlreadyExists;
        case ERROR_ACCESS_DENIED:
            return ErrorCode::PermissionDenied;
        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_OUTOFMEMORY:
            return ErrorCode::OutOfMemory;
        default:
            return ErrorCode::SystemError;
    }
}

uint32_t PlatformWin::get_process_id() {
    return ::GetCurrentProcessId();
}

} // namespace swiftchannel::platform

namespace swiftchannel {

// Windows implementation of SharedMemory::create_or_open
Result<SharedMemory> SharedMemory::create_or_open(
    const std::string& name,
    size_t size,
    bool create)
{
    std::wstring wide_name = platform::PlatformWin::to_shared_memory_name(name);

    HANDLE file_mapping = nullptr;

    if (create) {
        // Create new shared memory
        file_mapping = ::CreateFileMappingW(
            INVALID_HANDLE_VALUE,
            nullptr,
            PAGE_READWRITE,
            static_cast<DWORD>(size >> 32),
            static_cast<DWORD>(size & 0xFFFFFFFF),
            wide_name.c_str()
        );

        if (!file_mapping) {
            return Result<SharedMemory>(platform::PlatformWin::get_last_error());
        }

        // Check if it already existed
        if (::GetLastError() == ERROR_ALREADY_EXISTS) {
            // That's okay, we'll use the existing one
        }
    } else {
        // Open existing shared memory
        file_mapping = ::OpenFileMappingW(
            FILE_MAP_ALL_ACCESS,
            FALSE,
            wide_name.c_str()
        );

        if (!file_mapping) {
            return Result<SharedMemory>(platform::PlatformWin::get_last_error());
        }
    }

    // Map the memory
    void* data = ::MapViewOfFile(
        file_mapping,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        size
    );

    if (!data) {
        ErrorCode error = platform::PlatformWin::get_last_error();
        ::CloseHandle(file_mapping);
        return Result<SharedMemory>(error);
    }

    return Result<SharedMemory>(
        SharedMemory(name, data, size, file_mapping)
    );
}

void SharedMemory::close() {
    if (data_) {
        ::UnmapViewOfFile(data_);
        data_ = nullptr;
    }

    if (platform_handle_) {
        ::CloseHandle(static_cast<HANDLE>(platform_handle_));
        platform_handle_ = nullptr;
    }

    size_ = 0;
}

} // namespace swiftchannel

#endif // _WIN32
