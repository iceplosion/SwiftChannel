#pragma once

#include "swiftchannel/common/types.hpp"
#include "swiftchannel/common/error.hpp"
#include "swiftchannel/sender/config.hpp"

#include <string>
#include <memory>

namespace swiftchannel {

// Shared memory abstraction
class SharedMemory {
public:
    SharedMemory() = default;
    ~SharedMemory();

    // Non-copyable, movable
    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;
    SharedMemory(SharedMemory&&) noexcept;
    SharedMemory& operator=(SharedMemory&&) noexcept;

    // Create or open shared memory
    [[nodiscard]] static Result<SharedMemory> create_or_open(
        const std::string& name,
        size_t size,
        bool create);

    // Get pointer to mapped memory
    [[nodiscard]] void* data() noexcept { return data_; }
    [[nodiscard]] const void* data() const noexcept { return data_; }

    // Get size
    [[nodiscard]] size_t size() const noexcept { return size_; }

    // Get name
    [[nodiscard]] const std::string& name() const noexcept { return name_; }

    // Check if valid
    [[nodiscard]] bool is_valid() const noexcept { return data_ != nullptr; }

    // Close/unmap
    void close();

private:
    SharedMemory(std::string name, void* data, size_t size, void* handle);

    std::string name_;
    void* data_ = nullptr;
    size_t size_ = 0;
    void* platform_handle_ = nullptr;  // HANDLE on Windows, int on POSIX
};

} // namespace swiftchannel
