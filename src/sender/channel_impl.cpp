#include "swiftchannel/sender/channel.hpp"
#include "../ipc/shared_memory.hpp"
#include "../ipc/handshake.hpp"
#include "swiftchannel/common/alignment.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace swiftchannel {

Channel::Channel(std::string name, ChannelConfig config,
                void* shared_memory, size_t total_size,
                void* platform_handle) noexcept
    : name_(std::move(name))
    , config_(config)
    , shared_memory_(shared_memory)
    , total_size_(total_size)
    , platform_handle_(platform_handle)
{
    // Header is at the beginning
    header_ = static_cast<SharedMemoryHeader*>(shared_memory_);

    // Ring buffer starts after the header (aligned)
    uint8_t* ring_buffer_start = static_cast<uint8_t*>(shared_memory_) +
                                  align_up(sizeof(SharedMemoryHeader), CACHE_LINE_SIZE);

    ring_buffer_ = std::make_unique<RingBuffer>(ring_buffer_start, config_.ring_buffer_size);
}

Result<Channel> Channel::open(const std::string& name, const ChannelConfig& config) noexcept {
    if (!config.is_valid()) {
        return Result<Channel>(ErrorCode::InvalidOperation);
    }

    // Calculate total size: header + ring buffer (with alignment)
    size_t header_size = align_up(sizeof(SharedMemoryHeader), CACHE_LINE_SIZE);
    size_t total_size = header_size + config.ring_buffer_size;

    // Try to create or open shared memory
    auto shm_result = SharedMemory::create_or_open(name, total_size, true);

    if (shm_result.is_error()) {
        return Result<Channel>(shm_result.error());
    }

    auto shm = std::move(shm_result.value());
    void* memory = shm.data();
    void* platform_handle = nullptr;

    // Extract platform handle before moving shm
#ifdef _WIN32
    // On Windows, we need to duplicate the handle or extract it
    // For simplicity, we'll manage it through a helper
    platform_handle = shm.data();  // Placeholder - will be handled in platform code
#else
    // On POSIX, extract the fd
    platform_handle = reinterpret_cast<void*>(static_cast<intptr_t>(-1));
#endif

    // Get header pointer
    auto* header = static_cast<SharedMemoryHeader*>(memory);

    // Check if we need to initialize
    bool needs_init = (header->magic != SharedMemoryHeader::MAGIC);

    if (needs_init) {
        // Initialize header
        Handshake::initialize_header(header, config.ring_buffer_size, config.flags);
    } else {
        // Validate existing header
        auto validate_result = Handshake::validate_header(header);
        if (validate_result.is_error()) {
            return Result<Channel>(validate_result.error());
        }
    }

    // Perform sender handshake
    auto handshake_result = Handshake::sender_handshake(header);
    if (handshake_result.is_error()) {
        return Result<Channel>(handshake_result.error());
    }

    // Create channel
    return Result<Channel>(Channel(name, config, memory, total_size, platform_handle));
}

void Channel::close() noexcept {
#ifdef _WIN32
    if (shared_memory_) {
        ::UnmapViewOfFile(shared_memory_);
        shared_memory_ = nullptr;
    }
    if (platform_handle_) {
        ::CloseHandle(static_cast<HANDLE>(platform_handle_));
        platform_handle_ = nullptr;
    }
#else
    if (shared_memory_) {
        ::munmap(shared_memory_, total_size_);
        shared_memory_ = nullptr;
    }
    if (platform_handle_) {
        int fd = static_cast<int>(reinterpret_cast<intptr_t>(platform_handle_));
        if (fd >= 0) {
            ::close(fd);
        }
        platform_handle_ = nullptr;
    }
#endif

    header_ = nullptr;
    ring_buffer_.reset();
}

} // namespace swiftchannel
