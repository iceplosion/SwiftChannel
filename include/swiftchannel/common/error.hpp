#pragma once

#include <cstdint>
#include <string_view>
#include <system_error>

namespace swiftchannel {

// Error codes
enum class ErrorCode : int32_t {
    Success = 0,

    // Channel errors
    ChannelNotFound = 1000,
    ChannelAlreadyExists,
    ChannelFull,
    ChannelClosed,
    InvalidChannelName,

    // Message errors
    MessageTooLarge = 2000,
    InvalidMessage,
    MessageCorrupted,
    ChecksumMismatch,

    // Memory errors
    OutOfMemory = 3000,
    SharedMemoryError,
    MappingFailed,
    InvalidMemoryLayout,

    // Synchronization errors
    LockTimeout = 4000,
    ConcurrencyViolation,

    // System errors
    SystemError = 5000,
    PermissionDenied,
    ResourceBusy,
    InvalidOperation,

    // Versioning
    VersionMismatch = 6000,
    IncompatibleProtocol,
};

// Result type for operations
template<typename T = void>
class [[nodiscard]] Result {
public:
    Result() : error_(ErrorCode::Success), value_{} {}
    explicit Result(ErrorCode error) : error_(error), value_{} {}
    explicit Result(T&& value) : error_(ErrorCode::Success), value_(std::move(value)) {}

    [[nodiscard]] bool is_ok() const noexcept { return error_ == ErrorCode::Success; }
    [[nodiscard]] bool is_error() const noexcept { return error_ != ErrorCode::Success; }
    [[nodiscard]] ErrorCode error() const noexcept { return error_; }

    [[nodiscard]] T& value() & { return value_; }
    [[nodiscard]] const T& value() const & { return value_; }
    [[nodiscard]] T&& value() && { return std::move(value_); }

    [[nodiscard]] T value_or(T&& default_value) const & {
        return is_ok() ? value_ : std::move(default_value);
    }

private:
    ErrorCode error_;
    T value_;
};

// Specialization for void
template<>
class [[nodiscard]] Result<void> {
public:
    Result() : error_(ErrorCode::Success) {}
    explicit Result(ErrorCode error) : error_(error) {}

    [[nodiscard]] bool is_ok() const noexcept { return error_ == ErrorCode::Success; }
    [[nodiscard]] bool is_error() const noexcept { return error_ != ErrorCode::Success; }
    [[nodiscard]] ErrorCode error() const noexcept { return error_; }

private:
    ErrorCode error_;
};

// Convert error code to string
constexpr std::string_view error_to_string(ErrorCode code) noexcept {
    switch (code) {
        case ErrorCode::Success: return "Success";
        case ErrorCode::ChannelNotFound: return "Channel not found";
        case ErrorCode::ChannelAlreadyExists: return "Channel already exists";
        case ErrorCode::ChannelFull: return "Channel buffer is full";
        case ErrorCode::ChannelClosed: return "Channel is closed";
        case ErrorCode::InvalidChannelName: return "Invalid channel name";
        case ErrorCode::MessageTooLarge: return "Message too large";
        case ErrorCode::InvalidMessage: return "Invalid message";
        case ErrorCode::MessageCorrupted: return "Message corrupted";
        case ErrorCode::ChecksumMismatch: return "Checksum mismatch";
        case ErrorCode::OutOfMemory: return "Out of memory";
        case ErrorCode::SharedMemoryError: return "Shared memory error";
        case ErrorCode::MappingFailed: return "Memory mapping failed";
        case ErrorCode::InvalidMemoryLayout: return "Invalid memory layout";
        case ErrorCode::LockTimeout: return "Lock timeout";
        case ErrorCode::ConcurrencyViolation: return "Concurrency violation";
        case ErrorCode::SystemError: return "System error";
        case ErrorCode::PermissionDenied: return "Permission denied";
        case ErrorCode::ResourceBusy: return "Resource busy";
        case ErrorCode::InvalidOperation: return "Invalid operation";
        case ErrorCode::VersionMismatch: return "Version mismatch";
        case ErrorCode::IncompatibleProtocol: return "Incompatible protocol";
        default: return "Unknown error";
    }
}

} // namespace swiftchannel
