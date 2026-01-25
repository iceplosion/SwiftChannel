#include "platform_posix.hpp"

#ifndef _WIN32

#include "../../ipc/shared_memory.hpp"
#include "../../ipc/handshake.hpp"
#include "swiftchannel/sender/channel.hpp"
#include "swiftchannel/common/alignment.hpp"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

namespace swiftchannel::platform {

std::string PlatformPosix::to_shared_memory_name(const std::string& channel_name) {
    // POSIX shared memory names must start with /
    return "/swiftchannel_" + channel_name;
}

ErrorCode PlatformPosix::get_last_error() {
    switch (errno) {
        case 0:
            return ErrorCode::Success;
        case ENOENT:
            return ErrorCode::ChannelNotFound;
        case EEXIST:
            return ErrorCode::ChannelAlreadyExists;
        case EACCES:
        case EPERM:
            return ErrorCode::PermissionDenied;
        case ENOMEM:
            return ErrorCode::OutOfMemory;
        case EBUSY:
            return ErrorCode::ResourceBusy;
        default:
            return ErrorCode::SystemError;
    }
}

uint32_t PlatformPosix::get_process_id() {
    return static_cast<uint32_t>(getpid());
}

} // namespace swiftchannel::platform

namespace swiftchannel {

// POSIX implementation of SharedMemory::create_or_open
Result<SharedMemory> SharedMemory::create_or_open(
    const std::string& name,
    size_t size,
    bool create)
{
    std::string shm_name = platform::PlatformPosix::to_shared_memory_name(name);

    int shm_fd = -1;

    if (create) {
        // Create new shared memory
        shm_fd = ::shm_open(shm_name.c_str(), O_CREAT | O_RDWR, 0666);

        if (shm_fd == -1) {
            return Result<SharedMemory>(platform::PlatformPosix::get_last_error());
        }

        // Set size
        if (::ftruncate(shm_fd, static_cast<off_t>(size)) == -1) {
            ErrorCode error = platform::PlatformPosix::get_last_error();
            ::close(shm_fd);
            return Result<SharedMemory>(error);
        }
    } else {
        // Open existing shared memory
        shm_fd = ::shm_open(shm_name.c_str(), O_RDWR, 0666);

        if (shm_fd == -1) {
            return Result<SharedMemory>(platform::PlatformPosix::get_last_error());
        }
    }

    // Map the memory
    void* data = ::mmap(
        nullptr,
        size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        shm_fd,
        0
    );

    if (data == MAP_FAILED) {
        ErrorCode error = platform::PlatformPosix::get_last_error();
        ::close(shm_fd);
        return Result<SharedMemory>(error);
    }

    // Store fd as void* (cast to intptr_t first to avoid warnings)
    void* handle = reinterpret_cast<void*>(static_cast<intptr_t>(shm_fd));

    return Result<SharedMemory>(
        SharedMemory(name, data, size, handle)
    );
}

void SharedMemory::close() {
    if (data_) {
        ::munmap(data_, size_);
        data_ = nullptr;
    }

    if (platform_handle_) {
        int fd = static_cast<int>(reinterpret_cast<intptr_t>(platform_handle_));
        ::close(fd);
        platform_handle_ = nullptr;
    }

    size_ = 0;
}

} // namespace swiftchannel

#endif // !_WIN32
