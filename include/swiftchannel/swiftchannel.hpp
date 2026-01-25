#pragma once

#include "common/types.hpp"
#include "common/error.hpp"
#include "common/version.hpp"
#include "common/alignment.hpp"

#include "sender/sender.hpp"
#include "sender/channel.hpp"
#include "sender/message.hpp"
#include "sender/ring_buffer.hpp"
#include "sender/config.hpp"

// Main umbrella header for SwiftChannel
// For sender-only applications, just include this header - no linking required!
// For receiver applications, you'll need to link against the swiftchannel library.

namespace swiftchannel {

// Version information
constexpr auto get_version() noexcept -> Version {
    return Version{SWIFTCHANNEL_VERSION_MAJOR,
                   SWIFTCHANNEL_VERSION_MINOR,
                   SWIFTCHANNEL_VERSION_PATCH};
}

} // namespace swiftchannel
