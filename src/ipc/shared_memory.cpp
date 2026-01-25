#include "shared_memory.hpp"

namespace swiftchannel {

// SharedMemory implementation (delegated to platform-specific code)

SharedMemory::SharedMemory(std::string name, void* data, size_t size, void* handle)
    : name_(std::move(name))
    , data_(data)
    , size_(size)
    , platform_handle_(handle)
{}

SharedMemory::~SharedMemory() {
    close();
}

SharedMemory::SharedMemory(SharedMemory&& other) noexcept
    : name_(std::move(other.name_))
    , data_(other.data_)
    , size_(other.size_)
    , platform_handle_(other.platform_handle_)
{
    other.data_ = nullptr;
    other.size_ = 0;
    other.platform_handle_ = nullptr;
}

SharedMemory& SharedMemory::operator=(SharedMemory&& other) noexcept {
    if (this != &other) {
        close();

        name_ = std::move(other.name_);
        data_ = other.data_;
        size_ = other.size_;
        platform_handle_ = other.platform_handle_;

        other.data_ = nullptr;
        other.size_ = 0;
        other.platform_handle_ = nullptr;
    }
    return *this;
}

// Platform-specific implementations in platform/windows/shm_win.cpp and platform/posix/shm_posix.cpp

} // namespace swiftchannel
