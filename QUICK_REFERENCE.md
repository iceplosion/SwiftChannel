# SwiftChannel - Quick Reference Card

## 🚀 Quick Start (5 Minutes)

### Step 1: Build
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Step 2: Write a Sender (Header-Only!)
```cpp
#include <swiftchannel/swiftchannel.hpp>

struct MyData {
    int id;
    double value;
};

int main() {
    swiftchannel::Sender sender("my_channel");
    
    MyData data{42, 3.14};
    auto result = sender.send(data);
    
    if (result.is_ok()) {
        // Success!
    }
    return 0;
}
```

**CMake (Sender-only):**
```cmake
add_executable(my_app main.cpp)
target_include_directories(my_app PRIVATE /path/to/swiftchannel/include)
# No linking needed!
```

### Step 3: Write a Receiver
```cpp
#include <swiftchannel/swiftchannel.hpp>
#include <swiftchannel/receiver/receiver.hpp>

int main() {
    swiftchannel::Receiver receiver("my_channel");
    
    receiver.start([](const void* data, size_t size) {
        auto* msg = static_cast<const MyData*>(data);
        // Process message
    });
    
    return 0;
}
```

**CMake (Receiver):**
```cmake
add_executable(my_receiver main.cpp)
target_link_libraries(my_receiver PRIVATE swiftchannel)
```

---

## 📋 API Cheat Sheet

### Sender API

```cpp
// Create
Sender(const std::string& channel_name, const ChannelConfig& config = {});

// Send (type-safe)
template<Sendable T>
Result<void> send(const T& message);

// Send (raw bytes)
Result<void> send_bytes(const void* data, size_t size);

// Check status
bool is_ready() const;
size_t available_space() const;
```

### Receiver API

```cpp
// Create
Receiver(const std::string& channel_name, const ChannelConfig& config = {});

// Receive (blocking)
Result<void> start(MessageHandler handler);

// Receive (async)
Result<void> start_async(MessageHandler handler);

// Poll once (non-blocking)
Result<bool> poll_one(MessageHandler handler);

// Control
void stop();
bool is_running() const;

// Stats
Stats get_stats() const;
```

### Configuration

```cpp
ChannelConfig config;
config.ring_buffer_size = 1024 * 1024;  // Must be power of 2
config.max_message_size = 64 * 1024;
config.enable_checksum = false;
config.overwrite_on_full = false;
```

---

## 🎯 Common Patterns

### Pattern: Fire-and-Forget
```cpp
Sender sender("fast_channel");
sender.send(data);  // Returns immediately
```

### Pattern: Continuous Streaming
```cpp
Sender sender("stream");
while (running) {
    auto result = sender.send(next_data());
    if (result.is_error() && result.error() == ErrorCode::ChannelFull) {
        // Handle backpressure
        std::this_thread::sleep_for(1ms);
    }
}
```

### Pattern: Type-Safe Messages
```cpp
struct Trade {
    uint64_t order_id;
    double price;
    int quantity;
};

Message<Trade> msg(trade);
sender.send(msg);
```

### Pattern: Dynamic Messages
```cpp
DynamicMessage msg(1024);
// Fill msg.data()...
sender.send(msg);
```

### Pattern: Async Receiver
```cpp
Receiver receiver("channel");
receiver.start_async([](const void* data, size_t size) {
    // Runs in background thread
});

// Do other work...

receiver.stop();  // Clean shutdown
```

### Pattern: Polling Receiver
```cpp
Receiver receiver("channel");
while (running) {
    auto result = receiver.poll_one([](const void* data, size_t size) {
        // Process one message
    });
    
    if (result.is_ok() && !result.value()) {
        // No messages available
        std::this_thread::yield();
    }
}
```

---

## ⚡ Performance Tips

### 1. Buffer Sizing
```cpp
// Small messages, high frequency
config.ring_buffer_size = 1024 * 1024;      // 1MB
config.max_message_size = 1024;             // 1KB

// Large messages, moderate frequency
config.ring_buffer_size = 16 * 1024 * 1024; // 16MB
config.max_message_size = 256 * 1024;       // 256KB
```

### 2. Disable Checksums for Max Speed
```cpp
config.enable_checksum = false;  // Saves ~20ns per message
```

### 3. Power-of-2 Sizes
```cpp
// ✅ Good: Uses fast bitmask modulo
config.ring_buffer_size = 1048576;  // 2^20

// ❌ Bad: Requires expensive modulo
config.ring_buffer_size = 1000000;
```

### 4. Compiler Optimizations
```cmake
# CMake
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)  # LTO

# Or manually
target_compile_options(my_app PRIVATE -O3 -march=native)
```

### 5. CPU Affinity (Linux)
```cpp
// Pin receiver to specific core
cpu_set_t cpuset;
CPU_ZERO(&cpuset);
CPU_SET(1, &cpuset);
pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
```

---

## 🔧 Error Handling

### Check Result
```cpp
auto result = sender.send(msg);
if (result.is_error()) {
    switch (result.error()) {
        case ErrorCode::ChannelFull:
            // Implement backpressure
            break;
        case ErrorCode::MessageTooLarge:
            // Message exceeds max_message_size
            break;
        case ErrorCode::ChannelClosed:
            // Channel no longer available
            break;
        default:
            // Fatal error
            break;
    }
}
```

### Error Messages
```cpp
std::string msg = error_to_string(result.error());
std::cerr << "Error: " << msg << "\n";
```

---

## 🐛 Troubleshooting

### Issue: "Channel not found"
**Solution:**
- Start sender first (creates channel)
- Check channel names match exactly
- On POSIX: verify `/dev/shm` is writable

### Issue: Messages not received
**Solution:**
- Verify both sides use same `ChannelConfig`
- Check receiver is running before sending
- Verify message size < `max_message_size`

### Issue: "Buffer full" errors
**Solution:**
- Increase `ring_buffer_size`
- Speed up receiver processing
- Enable `overwrite_on_full` (data loss!)

### Issue: Poor performance
**Solution:**
- Use Release build (`-O3`)
- Disable checksums
- Check CPU governor (performance mode)
- Profile with `perf` or VTune

### Issue: Windows access denied
**Solution:**
- Check UAC settings
- Verify process has rights to create file mappings
- Run as Administrator (test only)

### Issue: POSIX permission denied
**Solution:**
- Check `/dev/shm` permissions: `ls -la /dev/shm`
- Verify user has read/write access
- Try: `sudo chmod 777 /dev/shm` (test only)

---

## 📊 Benchmarking

### Simple Latency Test
```cpp
auto start = std::chrono::high_resolution_clock::now();
sender.send(msg);
auto end = std::chrono::high_resolution_clock::now();
auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
std::cout << "Latency: " << latency.count() << " ns\n";
```

### Throughput Test
```cpp
const int N = 1'000'000;
auto start = std::chrono::high_resolution_clock::now();

for (int i = 0; i < N; ++i) {
    sender.send(msg);
}

auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
double throughput = N / duration.count();
std::cout << "Throughput: " << throughput << " msg/s\n";
```

---

## 🔍 Debugging

### Enable Debug Build
```cmake
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

### Check Channel Status
```bash
# Windows
# Use Resource Monitor → Memory → Shared Memory

# Linux
ls -lh /dev/shm/swiftchannel_*
cat /proc/sys/kernel/shmmax  # Max shared memory

# macOS
ipcs -m  # List shared memory segments
```

### Inspect Channel
```bash
./tools/ipc_inspector my_channel
```

### Statistics
```cpp
auto stats = receiver.get_stats();
std::cout << "Messages received: " << stats.messages_received << "\n";
std::cout << "Bytes received: " << stats.bytes_received << "\n";
std::cout << "Errors: " << stats.errors << "\n";
```

---

## 📚 Examples Location

All examples in `examples/` directory:
- **simple_sender**: Basic message sending
- **simple_receiver**: Basic message receiving
- **ipc_inspector**: Channel diagnostics tool

Build examples:
```bash
cmake --build . --target simple_sender
cmake --build . --target simple_receiver
```

Run:
```bash
# Terminal 1
./examples/simple_receiver

# Terminal 2
./examples/simple_sender
```

---

## 🔗 Quick Links

- **Full Guide**: `GETTING_STARTED.md`
- **Architecture**: `ARCHITECTURE.md`
- **API Docs**: Check header files in `include/swiftchannel/`
- **Examples**: `examples/` directory
- **Tests**: `tests/` directory

---

## 💡 Pro Tips

1. **Sender is header-only** - No linking needed for send-only apps!
2. **Power of 2 sizes** - Always use 2^N for buffer sizes
3. **POD types only** - Only send trivially copyable structs
4. **Preallocate** - Create Sender/Receiver at startup, reuse
5. **Profile first** - Measure before optimizing
6. **Use Release builds** - Debug builds are 10-100x slower
7. **Pin threads** - Use CPU affinity for consistent latency
8. **Warm up** - Run a few messages before benchmarking

---

## 📖 Example: Complete Application

**sender.cpp:**
```cpp
#include <swiftchannel/swiftchannel.hpp>

struct Tick {
    uint64_t timestamp;
    double price;
};

int main() {
    swiftchannel::Sender sender("market_data");
    
    for (int i = 0; i < 100; ++i) {
        Tick tick{
            std::chrono::system_clock::now().time_since_epoch().count(),
            100.0 + i * 0.1
        };
        
        if (auto result = sender.send(tick); result.is_error()) {
            std::cerr << "Send failed\n";
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    return 0;
}
```

**receiver.cpp:**
```cpp
#include <swiftchannel/swiftchannel.hpp>
#include <swiftchannel/receiver/receiver.hpp>

struct Tick {
    uint64_t timestamp;
    double price;
};

int main() {
    swiftchannel::Receiver receiver("market_data");
    
    receiver.start([](const void* data, size_t size) {
        if (size == sizeof(Tick)) {
            auto* tick = static_cast<const Tick*>(data);
            std::cout << "Price: " << tick->price << "\n";
        }
    });
    
    return 0;
}
```

**CMakeLists.txt:**
```cmake
# Sender (header-only)
add_executable(sender sender.cpp)
target_include_directories(sender PRIVATE /path/to/swiftchannel/include)

# Receiver (links library)
add_executable(receiver receiver.cpp)
target_link_libraries(receiver PRIVATE swiftchannel)
```

---

**That's it! You're ready to build high-performance IPC applications with SwiftChannel.** 🚀
