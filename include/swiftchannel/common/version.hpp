#pragma once

#include <cstdint>
#include <string>

namespace swiftchannel {

struct Version {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;

    constexpr bool operator==(const Version& other) const noexcept {
        return major == other.major && minor == other.minor && patch == other.patch;
    }

    constexpr bool operator!=(const Version& other) const noexcept {
        return !(*this == other);
    }

    constexpr bool is_compatible_with(const Version& other) const noexcept {
        // Major version must match, minor/patch can differ
        return major == other.major;
    }

    std::string to_string() const {
        return std::to_string(major) + "." +
               std::to_string(minor) + "." +
               std::to_string(patch);
    }

    constexpr uint32_t as_uint32() const noexcept {
        return (static_cast<uint32_t>(major) << 16) |
               (static_cast<uint32_t>(minor) << 8) |
               static_cast<uint32_t>(patch);
    }
};

// Library version
#ifndef SWIFTCHANNEL_VERSION_MAJOR
#define SWIFTCHANNEL_VERSION_MAJOR 1
#endif

#ifndef SWIFTCHANNEL_VERSION_MINOR
#define SWIFTCHANNEL_VERSION_MINOR 0
#endif

#ifndef SWIFTCHANNEL_VERSION_PATCH
#define SWIFTCHANNEL_VERSION_PATCH 0
#endif

constexpr Version LIBRARY_VERSION = {
    SWIFTCHANNEL_VERSION_MAJOR,
    SWIFTCHANNEL_VERSION_MINOR,
    SWIFTCHANNEL_VERSION_PATCH
};

// Protocol version (separate from library version)
constexpr Version PROTOCOL_VERSION = {1, 0, 0};

} // namespace swiftchannel
