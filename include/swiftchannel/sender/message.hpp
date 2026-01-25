#pragma once

#include "../common/types.hpp"
#include <concepts>
#include <cstring>
#include <span>
#include <vector>

namespace swiftchannel {

// Concept for trivially copyable types that can be sent
template<typename T>
concept Sendable = std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>;

// Message wrapper for type-safe messaging
template<Sendable T>
class Message {
public:
    Message() = default;
    explicit Message(const T& data) : data_(data) {}
    explicit Message(T&& data) : data_(std::move(data)) {}

    [[nodiscard]] const T& data() const noexcept { return data_; }
    [[nodiscard]] T& data() noexcept { return data_; }

    [[nodiscard]] const void* raw_data() const noexcept {
        return &data_;
    }

    [[nodiscard]] static constexpr size_t size() noexcept {
        return sizeof(T);
    }

private:
    T data_;
};

// Dynamic message for variable-length data
class DynamicMessage {
public:
    DynamicMessage() = default;

    explicit DynamicMessage(size_t size) : data_(size) {}

    DynamicMessage(const void* data, size_t size)
        : data_(static_cast<const uint8_t*>(data),
                static_cast<const uint8_t*>(data) + size) {}

    template<typename T>
    explicit DynamicMessage(const T& value)
        : data_(sizeof(T)) {
        static_assert(std::is_trivially_copyable_v<T>);
        std::memcpy(data_.data(), &value, sizeof(T));
    }

    [[nodiscard]] const void* data() const noexcept {
        return data_.data();
    }

    [[nodiscard]] void* data() noexcept {
        return data_.data();
    }

    [[nodiscard]] size_t size() const noexcept {
        return data_.size();
    }

    [[nodiscard]] std::span<const uint8_t> as_span() const noexcept {
        return data_;
    }

    void resize(size_t new_size) {
        data_.resize(new_size);
    }

    void reserve(size_t capacity) {
        data_.reserve(capacity);
    }

private:
    std::vector<uint8_t> data_;
};

} // namespace swiftchannel
