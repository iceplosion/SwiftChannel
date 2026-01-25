#include "platform_posix.hpp"

#ifndef _WIN32

// POSIX socket implementation (for future use)
// Currently, we're using shared memory for the main IPC
// Unix domain sockets can be used for control/handshake channels

namespace swiftchannel::platform {

// TODO: Implement Unix domain socket helpers if needed for control channel
// For now, shared memory is sufficient

} // namespace swiftchannel::platform

#endif // !_WIN32
