# SwiftChannel - Project Index

Welcome to **SwiftChannel**, a high-performance inter-process communication library!

This index helps you navigate the project structure and find what you need quickly.

---

## 📖 Documentation (Start Here!)

| Document | Purpose | Target Audience |
|----------|---------|----------------|
| [README.md](README.md) | Project overview & quick start | Everyone |
| [QUICK_REFERENCE.md](QUICK_REFERENCE.md) | API cheat sheet & patterns | Developers |
| [GETTING_STARTED.md](GETTING_STARTED.md) | Comprehensive user guide | New users |
| [ARCHITECTURE.md](ARCHITECTURE.md) | Technical deep dive | Advanced users |
| [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md) | Complete framework overview | Contributors |

---

## 🚀 Quick Actions

### I want to...

**...get started immediately**
→ Read [QUICK_REFERENCE.md](QUICK_REFERENCE.md) (5 minutes)

**...understand the architecture**
→ Read [ARCHITECTURE.md](ARCHITECTURE.md) (20 minutes)

**...build the project**
→ Run `verify_build.sh` (Linux/Mac) or `verify_build.bat` (Windows)

**...see examples**
→ Check [examples/](examples/) directory

**...run tests**
→ Build and run `ctest -C Release`

**...integrate into my project**
→ See "Integration" section in [GETTING_STARTED.md](GETTING_STARTED.md)

**...contribute**
→ Read [ARCHITECTURE.md](ARCHITECTURE.md) and [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)

---

## 📁 Directory Guide

### `/` (Root)
- **CMakeLists.txt** - Main build configuration
- **verify_build.sh** / **verify_build.bat** - Build verification scripts
- **LICENSE** - MIT License
- **.gitignore** - Git ignore rules

### `/cmake/`
Build system configuration
- **compiler_options.cmake** - Compiler flags & optimizations
- **SwiftChannelConfig.cmake.in** - Package config template

### `/include/swiftchannel/`
**Public API** (these are what you include in your code)

#### `/include/swiftchannel/common/`
Core types and utilities
- **types.hpp** - `MessageHeader`, `SharedMemoryHeader`, type aliases
- **error.hpp** - `ErrorCode`, `Result<T>`, error handling
- **version.hpp** - Version management
- **alignment.hpp** - Cache-line alignment utilities

#### `/include/swiftchannel/sender/` ⚡ HEADER-ONLY
Fast-path sending (no linking required!)
- **sender.hpp** - Main `Sender` class with inline `send()`
- **channel.hpp** - `Channel` abstraction
- **message.hpp** - `Message<T>`, `DynamicMessage` wrappers
- **ring_buffer.hpp** - Lock-free SPSC ring buffer
- **config.hpp** - `ChannelConfig` structure

#### `/include/swiftchannel/receiver/`
Receiver API (requires linking)
- **receiver.hpp** - `Receiver` class declaration

### `/src/`
**Compiled implementation** (builds into `libswiftchannel`)

#### `/src/receiver/`
Receiver implementation
- **receiver.cpp** - Main receiver logic
- **receiver_impl.hpp** - Private implementation
- **dispatch.cpp** - Message dispatch

#### `/src/sender/`
Compiled sender parts
- **channel_impl.cpp** - Channel lifecycle management

#### `/src/ipc/`
IPC infrastructure
- **shared_memory.{hpp,cpp}** - Shared memory abstraction
- **handshake.{hpp,cpp}** - Protocol handshake & versioning

#### `/src/platform/`
Platform-specific implementations

**`/src/platform/windows/`** - Windows support
- **platform_win.hpp** - Windows utilities
- **shm_win.cpp** - Named File Mapping implementation
- **pipe_win.cpp** - Named pipes (future)

**`/src/platform/posix/`** - POSIX support (Linux/macOS)
- **platform_posix.hpp** - POSIX utilities
- **shm_posix.cpp** - POSIX shm_open implementation
- **socket_posix.cpp** - Unix sockets (future)

#### `/src/diagnostics/`
Statistics & monitoring
- **stats.{hpp,cpp}** - Statistics tracking

### `/tests/`
Test suite
- **CMakeLists.txt** - Test build configuration
- **unit/ring_buffer_test.cpp** - Ring buffer tests
- **unit/message_test.cpp** - Message type tests
- **integration/sender_receiver_test.cpp** - End-to-end test

### `/examples/`
Example applications
- **CMakeLists.txt** - Examples build configuration
- **simple_sender/main.cpp** - Basic sender example
- **simple_receiver/main.cpp** - Basic receiver example

### `/tools/`
Diagnostic tools
- **ipc_inspector/main.cpp** - Channel inspector

---

## 🎯 Common Use Cases

### Use Case 1: I want to send messages (sender-only app)

**What to include:**
```cpp
#include <swiftchannel/swiftchannel.hpp>
```

**What to link:**
- Nothing! It's header-only for senders.

**Files to read:**
- [QUICK_REFERENCE.md](QUICK_REFERENCE.md) - API reference
- [include/swiftchannel/sender/sender.hpp](include/swiftchannel/sender/sender.hpp) - API details

**Example:**
- [examples/simple_sender/main.cpp](examples/simple_sender/main.cpp)

---

### Use Case 2: I want to receive messages

**What to include:**
```cpp
#include <swiftchannel/swiftchannel.hpp>
#include <swiftchannel/receiver/receiver.hpp>
```

**What to link:**
- `libswiftchannel.a` (or `swiftchannel.lib` on Windows)

**Files to read:**
- [QUICK_REFERENCE.md](QUICK_REFERENCE.md) - API reference
- [include/swiftchannel/receiver/receiver.hpp](include/swiftchannel/receiver/receiver.hpp) - API details

**Example:**
- [examples/simple_receiver/main.cpp](examples/simple_receiver/main.cpp)

---

### Use Case 3: I want to understand the design

**Files to read (in order):**
1. [README.md](README.md) - Overview
2. [ARCHITECTURE.md](ARCHITECTURE.md) - Detailed design
3. [include/swiftchannel/sender/ring_buffer.hpp](include/swiftchannel/sender/ring_buffer.hpp) - Lock-free implementation
4. [src/platform/windows/shm_win.cpp](src/platform/windows/shm_win.cpp) or [src/platform/posix/shm_posix.cpp](src/platform/posix/shm_posix.cpp) - Platform specifics

---

### Use Case 4: I want to extend the library

**Key extension points:**
- **Platform support**: Add new directory in `src/platform/`
- **Message dispatch**: Extend `src/receiver/dispatch.cpp`
- **Statistics**: Extend `src/diagnostics/stats.{hpp,cpp}`
- **Configuration**: Add fields to `ChannelConfig` in `include/swiftchannel/sender/config.hpp`

**Files to read:**
- [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md) - Complete overview
- [ARCHITECTURE.md](ARCHITECTURE.md) - Design rationale

---

## 🔍 Finding Specific Features

### Where is the lock-free ring buffer implementation?
→ [include/swiftchannel/sender/ring_buffer.hpp](include/swiftchannel/sender/ring_buffer.hpp)

### Where is the shared memory abstraction?
→ [src/ipc/shared_memory.hpp](src/ipc/shared_memory.hpp) (interface)
→ [src/platform/windows/shm_win.cpp](src/platform/windows/shm_win.cpp) (Windows impl)
→ [src/platform/posix/shm_posix.cpp](src/platform/posix/shm_posix.cpp) (POSIX impl)

### Where is error handling defined?
→ [include/swiftchannel/common/error.hpp](include/swiftchannel/common/error.hpp)

### Where are the message types defined?
→ [include/swiftchannel/common/types.hpp](include/swiftchannel/common/types.hpp) (headers)
→ [include/swiftchannel/sender/message.hpp](include/swiftchannel/sender/message.hpp) (wrappers)

### Where is the handshake protocol?
→ [src/ipc/handshake.{hpp,cpp}](src/ipc/handshake.hpp)

### Where are compile-time options?
→ [cmake/compiler_options.cmake](cmake/compiler_options.cmake)

---

## 📊 Performance Features

### Zero-copy messaging
→ Implemented in [ring_buffer.hpp](include/swiftchannel/sender/ring_buffer.hpp)

### Lock-free synchronization
→ Atomic operations in [ring_buffer.hpp](include/swiftchannel/sender/ring_buffer.hpp)

### Cache-line alignment
→ Utilities in [alignment.hpp](include/swiftchannel/common/alignment.hpp)

### Header-only sender
→ All sender code in [include/swiftchannel/sender/](include/swiftchannel/sender/)

---

## 🧪 Testing

### Unit tests
- **Ring buffer**: [tests/unit/ring_buffer_test.cpp](tests/unit/ring_buffer_test.cpp)
- **Messages**: [tests/unit/message_test.cpp](tests/unit/message_test.cpp)

### Integration tests
- **End-to-end**: [tests/integration/sender_receiver_test.cpp](tests/integration/sender_receiver_test.cpp)

### Run tests
```bash
cd build
ctest -C Release --verbose
```

---

## 🛠️ Build System

### Main configuration
→ [CMakeLists.txt](CMakeLists.txt)

### Compiler options
→ [cmake/compiler_options.cmake](cmake/compiler_options.cmake)

### Package config
→ [cmake/SwiftChannelConfig.cmake.in](cmake/SwiftChannelConfig.cmake.in)

### Verification scripts
- Linux/Mac: [verify_build.sh](verify_build.sh)
- Windows: [verify_build.bat](verify_build.bat)

---

## 💡 Tips & Tricks

### Fastest way to get started
1. Run `verify_build.sh` or `verify_build.bat`
2. Read [QUICK_REFERENCE.md](QUICK_REFERENCE.md)
3. Copy [examples/simple_sender/main.cpp](examples/simple_sender/main.cpp)
4. Start coding!

### Best practices
→ See "Best Practices" section in [GETTING_STARTED.md](GETTING_STARTED.md)

### Performance tuning
→ See "Performance Tuning" section in [GETTING_STARTED.md](GETTING_STARTED.md)

### Troubleshooting
→ See "Troubleshooting" section in [QUICK_REFERENCE.md](QUICK_REFERENCE.md)

---

## 📚 Learning Path

### Beginner
1. ✅ Read [README.md](README.md)
2. ✅ Run [verify_build.sh](verify_build.sh)
3. ✅ Study [examples/simple_sender/main.cpp](examples/simple_sender/main.cpp)
4. ✅ Read [QUICK_REFERENCE.md](QUICK_REFERENCE.md)
5. ✅ Write your first sender app

### Intermediate
1. ✅ Read [GETTING_STARTED.md](GETTING_STARTED.md)
2. ✅ Study [examples/simple_receiver/main.cpp](examples/simple_receiver/main.cpp)
3. ✅ Write a receiver app
4. ✅ Experiment with configuration options
5. ✅ Run benchmarks

### Advanced
1. ✅ Read [ARCHITECTURE.md](ARCHITECTURE.md)
2. ✅ Study [ring_buffer.hpp](include/swiftchannel/sender/ring_buffer.hpp)
3. ✅ Read platform-specific code
4. ✅ Profile your application
5. ✅ Consider contributing

---

## 🤝 Contributing

### Before contributing
1. Read [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)
2. Read [ARCHITECTURE.md](ARCHITECTURE.md)
3. Study the existing code
4. Run all tests

### Areas for contribution
- Additional platform support
- Performance optimizations
- More examples
- Documentation improvements
- Bug fixes

---

## 📞 Support

### Documentation
- Start with [README.md](README.md)
- Check [QUICK_REFERENCE.md](QUICK_REFERENCE.md) for API
- Read [GETTING_STARTED.md](GETTING_STARTED.md) for details

### Examples
- Simple use cases: [examples/](examples/)
- Test code: [tests/](tests/)

### Issues
- Check troubleshooting in [QUICK_REFERENCE.md](QUICK_REFERENCE.md)
- Review [GETTING_STARTED.md](GETTING_STARTED.md) troubleshooting section

---

## 📈 Project Statistics

- **Total Files**: 47+
- **Lines of Code**: ~5,000+ (headers + implementation)
- **Documentation**: ~3,000+ lines
- **Languages**: C++20
- **Platforms**: Windows, Linux, macOS
- **License**: MIT

---

## 🎉 Next Steps

1. **Build the project**: Run `verify_build.sh` or `verify_build.bat`
2. **Learn the API**: Read [QUICK_REFERENCE.md](QUICK_REFERENCE.md)
3. **Run examples**: Try `examples/simple_sender` and `examples/simple_receiver`
4. **Start coding**: Copy examples and modify for your use case
5. **Optimize**: Read performance tips in [GETTING_STARTED.md](GETTING_STARTED.md)

---

**Happy coding with SwiftChannel!** 🚀

*For questions or issues, refer to the documentation files listed above.*
