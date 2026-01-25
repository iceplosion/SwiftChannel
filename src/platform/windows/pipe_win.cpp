#include "platform_win.hpp"

#ifdef _WIN32

// Windows named pipe implementation (for future use)
// Currently, we're using shared memory for the main IPC
// Named pipes can be used for control/handshake channels

namespace swiftchannel::platform {

// TODO: Implement named pipe helpers if needed for control channel
// For now, shared memory is sufficient

} // namespace swiftchannel::platform

#endif // _WIN32
