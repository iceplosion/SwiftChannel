# SwiftChannel

A high-performance, low-latency inter-process communication (IPC) library for modern C++.

## Features

- **Header-only sender API** - Zero friction integration for sender applications
- **Low latency** - Lock-free ring buffer design with minimal syscalls
- **Cross-platform** - Works on Windows (named pipes/shared memory) and POSIX systems (sockets/shm)
- **Type-safe** - Modern C++20 design
- **Easy to integrate** - CMake support with prebuilt binaries

## Quick Start

### Sender (Header-only)

```cpp
#include <swiftchannel/swiftchannel.hpp>

int main() {
    swiftchannel::Sender sender("my_channel");
    
    // Send a message
    MyMessage msg;
    sender.send(msg);
    
    return 0;
}
```

**CMake integration (sender-only):**
```cmake
add_executable(my_sender main.cpp)
target_include_directories(my_sender PRIVATE path/to/swiftchannel/include)
```

### Receiver (Requires linking)

```cpp
#include <swiftchannel/swiftchannel.hpp>

int main() {
    swiftchannel::Receiver receiver("my_channel");
    
    receiver.start([](const void* data, size_t size) {
        // Process message
    });
    
    return 0;
}
```

**CMake integration (receiver):**
```cmake
add_executable(my_receiver main.cpp)
target_link_libraries(my_receiver PRIVATE swiftchannel)
```

## Architecture

```
Application Layer
       ↓
Header-only Sender API (zero-copy, inline ring buffer)
       ↓
Compiled Core Runtime (receiver, lifecycle management)
       ↓
OS Primitives (shared memory, sockets/pipes)
```

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Options

- `SWIFTCHANNEL_BUILD_TESTS=ON/OFF` - Build unit tests (default: ON)
- `SWIFTCHANNEL_BUILD_EXAMPLES=ON/OFF` - Build examples (default: ON)

## 📚 Documentation

- **[INDEX.md](INDEX.md)** - Complete project navigation guide
- **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** - API cheat sheet & common patterns
- **[GETTING_STARTED.md](GETTING_STARTED.md)** - Comprehensive user guide
- **[ARCHITECTURE.md](ARCHITECTURE.md)** - Technical design deep dive
- **[PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)** - Framework overview

## 🚀 Quick Start

### Option 1: Use verification script
```bash
# Linux/Mac
./verify_build.sh

# Windows
verify_build.bat
```

### Option 2: Manual build
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
ctest -C Release
```

## License

MIT License - See LICENSE file for details.

## Design Philosophy

SwiftChannel is designed with the following principles:

1. **Sender-first optimization** - Most applications are senders; make their life easy
2. **Zero-copy where possible** - Direct memory access through shared memory
3. **Fail-fast semantics** - Clear error reporting and recovery
4. **ABI stability** - Compiled core with stable interface
