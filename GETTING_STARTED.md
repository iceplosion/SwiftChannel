# SwiftChannel - Getting Started Guide

## Overview

SwiftChannel is a high-performance inter-process communication (IPC) library designed for low-latency, high-throughput message passing between processes.

## Key Design Principles

### 1. **Header-Only Sender API**
The sender side is completely header-only. This means:
- ✅ Zero friction integration - just include headers
- ✅ No linking required for sender-only apps
- ✅ Inline optimizations for maximum performance
- ✅ Direct ring buffer writes with no syscalls

### 2. **Compiled Receiver Core**
The receiver and infrastructure are compiled into a library:
- ✅ ABI stability
- ✅ Shared memory lifecycle management
- ✅ Platform abstraction (Windows/POSIX)
- ✅ Easy to ship prebuilt binaries

### 3. **Lock-Free Architecture**
- Single Producer Single Consumer (SPSC) ring buffer
- Atomic operations for synchronization
- Cache-line aligned data structures to prevent false sharing
- Zero-copy message passing through shared memory

## Building the Library

### Prerequisites
- CMake 3.20 or later
- C++20 compatible compiler (MSVC 2019+, GCC 10+, Clang 11+)
- Windows or POSIX-compatible OS (Linux, macOS, *BSD)

### Build Steps

```bash
# Clone or extract the repository
cd SwiftChannel

# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build . --config Release

# Optional: Run tests
ctest -C Release

# Optional: Install
cmake --install . --prefix /your/install/path
```

### Windows-Specific (MSVC)

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

## Usage Examples

### Sender-Only Application (Header-Only)

**main.cpp:**
```cpp
#include <swiftchannel/swiftchannel.hpp>

struct MyMessage {
    int id;
    double value;
};

int main() {
    // Create a sender
    swiftchannel::Sender sender("my_channel");
    
    // Send messages
    MyMessage msg{42, 3.14};
    auto result = sender.send(msg);
    
    if (result.is_ok()) {
        // Success!
    }
    
    return 0;
}
```

**CMakeLists.txt:**
```cmake
add_executable(my_sender main.cpp)
target_include_directories(my_sender PRIVATE 
    /path/to/swiftchannel/include)
```

**That's it!** No linking required for sender-only apps.

### Receiver Application

**main.cpp:**
```cpp
#include <swiftchannel/swiftchannel.hpp>
#include <swiftchannel/receiver/receiver.hpp>

struct MyMessage {
    int id;
    double value;
};

int main() {
    swiftchannel::Receiver receiver("my_channel");
    
    receiver.start([](const void* data, size_t size) {
        auto* msg = static_cast<const MyMessage*>(data);
        // Process message
    });
    
    return 0;
}
```

**CMakeLists.txt:**
```cmake
add_executable(my_receiver main.cpp)
target_link_libraries(my_receiver PRIVATE swiftchannel)
```

## Configuration Options

```cpp
swiftchannel::ChannelConfig config;
config.ring_buffer_size = 1024 * 1024;  // 1MB (must be power of 2)
config.max_message_size = 64 * 1024;     // 64KB
config.enable_checksum = false;          // Optional checksum validation
config.overwrite_on_full = false;        // Overwrite old messages when full

swiftchannel::Sender sender("channel", config);
```

## Performance Tuning

### 1. **Ring Buffer Size**
- Must be a power of 2
- Larger = fewer buffer-full events, but more memory
- Typical values: 64KB - 16MB

### 2. **Cache Line Alignment**
By default, SwiftChannel uses 64-byte cache lines. Override at compile time:
```cmake
add_compile_definitions(SWIFTCHANNEL_CACHE_LINE_SIZE=128)
```

### 3. **Compiler Optimizations**
For maximum performance:
- Use Release builds (`-O3` or `/O2`)
- Enable LTO (Link-Time Optimization)
- Consider `-march=native` for CPU-specific optimizations

## API Reference

### Sender API

```cpp
// Create sender
Sender(const std::string& channel_name, 
       const ChannelConfig& config = {});

// Send a message
template<Sendable T>
Result<void> send(const T& message);

// Check if ready
bool is_ready() const;

// Get available space
size_t available_space() const;
```

### Receiver API

```cpp
// Create receiver
Receiver(const std::string& channel_name,
         const ChannelConfig& config = {});

// Start receiving (blocking)
Result<void> start(MessageHandler handler);

// Start receiving (async)
Result<void> start_async(MessageHandler handler);

// Stop receiving
void stop();

// Poll for one message
Result<bool> poll_one(MessageHandler handler);

// Get statistics
Stats get_stats() const;
```

## Error Handling

SwiftChannel uses a `Result<T>` type for error handling:

```cpp
auto result = sender.send(msg);

if (result.is_ok()) {
    // Success
} else {
    ErrorCode error = result.error();
    std::string msg = error_to_string(error);
    // Handle error
}
```

Common error codes:
- `ErrorCode::ChannelFull` - Ring buffer is full
- `ErrorCode::MessageTooLarge` - Message exceeds max size
- `ErrorCode::ChannelNotFound` - Channel doesn't exist
- `ErrorCode::VersionMismatch` - Protocol version incompatible

## Platform Differences

### Windows
- Uses Named File Mapping for shared memory
- Shared memory name: `Local\SwiftChannel_<channel_name>`
- Requires no special privileges

### POSIX (Linux/macOS)
- Uses POSIX shared memory (`shm_open`)
- Shared memory name: `/swiftchannel_<channel_name>`
- May require `/dev/shm` mounted

## Best Practices

1. **Match configurations**: Sender and receiver should use the same `ChannelConfig`
2. **Handle buffer full**: Either increase buffer size or implement backpressure
3. **Message size**: Keep messages reasonably small (< 64KB recommended)
4. **Trivially copyable**: Only send POD types or trivially copyable structs
5. **Cleanup**: Channels are automatically cleaned up, but consider manual cleanup in long-running apps

## Troubleshooting

### "Channel not found" error
- Ensure sender creates the channel before receiver connects
- Check channel name matches exactly
- On POSIX, verify `/dev/shm` is writable

### Messages not received
- Verify both sides use the same `ChannelConfig`
- Check protocol version compatibility
- Ensure receiver is running before sender sends

### Performance issues
- Increase ring buffer size
- Disable checksum validation
- Use Release builds
- Check CPU affinity and thread pinning

## Next Steps

- See `examples/` directory for complete examples
- Read the API documentation in header files
- Check `tests/` for usage patterns
- Profile your application with the IPC inspector tool

## Support and Contributing

For issues, questions, or contributions:
- Check the existing examples and tests
- Review the header documentation
- Open an issue with detailed reproduction steps

## License

SwiftChannel is released under the MIT License. See LICENSE file for details.
