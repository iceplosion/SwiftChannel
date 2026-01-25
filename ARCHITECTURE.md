# SwiftChannel Architecture

## Table of Contents
1. [Design Overview](#design-overview)
2. [Three-Layer Architecture](#three-layer-architecture)
3. [Memory Layout](#memory-layout)
4. [Lock-Free Ring Buffer](#lock-free-ring-buffer)
5. [Platform Abstraction](#platform-abstraction)
6. [Message Flow](#message-flow)
7. [Error Handling Strategy](#error-handling-strategy)
8. [Performance Characteristics](#performance-characteristics)

---

## Design Overview

SwiftChannel implements a **zero-copy, lock-free IPC** mechanism using shared memory and atomic operations. The design prioritizes:

- **Low latency**: Sub-microsecond message passing
- **High throughput**: Millions of messages per second
- **Zero-copy**: Direct memory access, no serialization
- **Easy integration**: Header-only sender API

### Core Principles

1. **Sender Optimization**: The hot path (sending) is header-only and inlined
2. **Lock-Free**: SPSC ring buffer with atomic indices
3. **Cache-Friendly**: Aligned to cache lines, minimal false sharing
4. **Platform Neutral**: Abstraction over Windows/POSIX primitives

---

## Three-Layer Architecture

```
┌─────────────────────────────────────────┐
│        Application Layer                │
│  (User Code - Sender/Receiver)          │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│     Header-Only Sender API              │  ← Zero friction
│  • Inline send()                        │
│  • Ring buffer writes                   │
│  • No allocations, no syscalls          │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│     Compiled Core Runtime               │  ← Stable ABI
│  • Receiver implementation              │
│  • Channel lifecycle                    │
│  • Handshake & versioning               │
│  • Platform abstraction                 │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│     OS Primitives                       │
│  Windows: File Mapping                  │
│  POSIX: shm_open + mmap                 │
└─────────────────────────────────────────┘
```

### Layer 1: Header-Only Sender API

**Files**: `include/swiftchannel/sender/*.hpp`

This layer provides the fast-path sending logic:
- `sender.hpp`: Main Sender class
- `ring_buffer.hpp`: Lock-free SPSC ring buffer
- `message.hpp`: Type-safe message wrappers
- `config.hpp`: Configuration structures

**Key characteristics**:
- ✅ No linking required
- ✅ Fully inlined
- ✅ Zero syscalls in fast path
- ✅ Compile-time optimizations

### Layer 2: Compiled Runtime

**Files**: `src/receiver/`, `src/ipc/`

This layer handles:
- Shared memory creation/opening
- Channel lifecycle management
- Receiver polling/dispatch
- Handshake and version negotiation
- Statistics and diagnostics

**Key characteristics**:
- ✅ Stable ABI (can be precompiled)
- ✅ Platform abstraction
- ✅ Resource management
- ✅ Error handling

### Layer 3: Platform Primitives

**Files**: `src/platform/windows/`, `src/platform/posix/`

Platform-specific implementations:
- Shared memory allocation
- Memory mapping
- Process synchronization (if needed)

---

## Memory Layout

### Shared Memory Structure

```
┌──────────────────────────────────────────────┐  ← Start of shared memory
│  SharedMemoryHeader (128 bytes, aligned)     │
│  ┌────────────────────────────────────────┐  │
│  │ magic: 0x53574946 ("SWIF")             │  │
│  │ version: uint32                        │  │
│  │ ring_buffer_size: uint64               │  │
│  │ write_index: atomic<uint64>            │  │  ← Producer writes here
│  │ read_index: atomic<uint64>             │  │  ← Consumer reads here
│  │ sender_pid: uint32                     │  │
│  │ receiver_pid: uint32                   │  │
│  │ flags: uint64                          │  │
│  │ reserved[8]: uint64                    │  │
│  └────────────────────────────────────────┘  │
├──────────────────────────────────────────────┤  ← Cache-line aligned
│                                              │
│           Ring Buffer Data                   │
│         (power-of-2 size)                    │
│                                              │
│  Message 1:                                  │
│  ┌────────────────────────────────────────┐  │
│  │ MessageHeader (32 bytes)               │  │
│  │ ┌──────────────────────────────────┐   │  │
│  │ │ magic: 0x53574946                │   │  │
│  │ │ size: uint32                     │   │  │
│  │ │ sequence: uint64                 │   │  │
│  │ │ timestamp: uint64                │   │  │
│  │ │ checksum: uint32                 │   │  │
│  │ └──────────────────────────────────┘   │  │
│  │ Payload (variable size, 8-byte aligned) │  │
│  └────────────────────────────────────────┘  │
│                                              │
│  Message 2: ...                              │
│                                              │
│  (wraps around at ring_buffer_size)          │
│                                              │
└──────────────────────────────────────────────┘
```

### Key Points

1. **Header Alignment**: SharedMemoryHeader is cache-line aligned to prevent false sharing
2. **Ring Buffer**: Power-of-2 size enables fast modulo using bitmask
3. **Message Alignment**: Each message payload is 8-byte aligned
4. **Wrap-Around**: Ring buffer wraps at boundaries, handled transparently

---

## Lock-Free Ring Buffer

### SPSC (Single Producer, Single Consumer) Design

The ring buffer uses **two atomic indices**:
- `write_index`: Updated by sender (producer)
- `read_index`: Updated by receiver (consumer)

### Write Algorithm

```cpp
// Pseudo-code for sender
uint64_t current_write = write_index.load(relaxed);
uint64_t current_read = read_index.load(acquire);  // ← Memory barrier

uint64_t available = ring_size - (current_write - current_read);

if (available >= message_size) {
    // Write message to ring[current_write % ring_size]
    write_index.store(current_write + message_size, release);  // ← Memory barrier
    return SUCCESS;
}
return BUFFER_FULL;
```

### Read Algorithm

```cpp
// Pseudo-code for receiver
uint64_t current_read = read_index.load(relaxed);
uint64_t current_write = write_index.load(acquire);  // ← Memory barrier

if (current_read < current_write) {
    // Read message from ring[current_read % ring_size]
    read_index.store(current_read + message_size, release);  // ← Memory barrier
    return SUCCESS;
}
return BUFFER_EMPTY;
```

### Memory Ordering

- **acquire**: Ensures all previous writes by the other thread are visible
- **release**: Ensures all writes before this point are visible to other threads
- **relaxed**: No synchronization, used for local reads

This is the **minimum necessary synchronization** for correctness.

### Cache-Line Optimization

```cpp
// write_index and read_index are in separate cache lines
struct alignas(64) CacheAligned<atomic<uint64_t>>;
```

This prevents **false sharing**: updates to one index don't invalidate the other's cache line.

---

## Platform Abstraction

### Windows Implementation

**Shared Memory**: Named File Mapping
```cpp
HANDLE CreateFileMappingW(
    INVALID_HANDLE_VALUE,      // Use page file
    NULL,                       // Security
    PAGE_READWRITE,            // Access
    size_high, size_low,       // Size
    L"Local\\SwiftChannel_name" // Name
);

void* MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, size);
```

### POSIX Implementation

**Shared Memory**: POSIX shm
```cpp
int fd = shm_open(
    "/swiftchannel_name",     // Name (must start with /)
    O_CREAT | O_RDWR,          // Flags
    0666                       // Permissions
);

ftruncate(fd, size);

void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
```

### Abstraction Layer

Both implementations are hidden behind:
```cpp
class SharedMemory {
    static Result<SharedMemory> create_or_open(name, size, create);
    void* data();
    void close();
};
```

---

## Message Flow

### Sending a Message

```
Application
    ↓
sender.send(msg)
    ↓
[Header-only inline code]
    ↓
Check buffer space
    ↓
Write MessageHeader
    ↓
Write payload
    ↓
Atomic update write_index (release)
    ↓
Return to application
```

**Latency**: ~50-200 nanoseconds (no syscalls!)

### Receiving a Message

```
Application
    ↓
receiver.start(handler)
    ↓
[Compiled runtime]
    ↓
Poll loop:
  ↓
  Atomic load write_index (acquire)
  ↓
  Check if data available
  ↓
  Read MessageHeader
  ↓
  Validate magic/checksum
  ↓
  Read payload
  ↓
  Atomic update read_index (release)
  ↓
  Call handler(data, size)
  ↓
  Repeat
```

---

## Error Handling Strategy

### Result<T> Type

```cpp
template<typename T>
class Result {
    ErrorCode error_;
    T value_;
public:
    bool is_ok() const;
    bool is_error() const;
    ErrorCode error() const;
    T& value();
};
```

### Error Categories

1. **Channel Errors**: Not found, already exists, full, closed
2. **Message Errors**: Too large, invalid, corrupted
3. **Memory Errors**: Out of memory, mapping failed
4. **System Errors**: Permission denied, resource busy
5. **Version Errors**: Protocol mismatch

### Error Propagation

```cpp
auto result = sender.send(msg);
if (result.is_error()) {
    switch (result.error()) {
        case ErrorCode::ChannelFull:
            // Handle backpressure
            break;
        case ErrorCode::MessageTooLarge:
            // Split message or increase max size
            break;
        default:
            // Fatal error
            break;
    }
}
```

---

## Performance Characteristics

### Latency

| Operation | Typical Latency | Notes |
|-----------|----------------|-------|
| Send (fast path) | 50-200 ns | Header-only, no syscalls |
| Send (buffer full) | 50-200 ns | Just returns error |
| Receive (data available) | 100-300 ns | Includes memcpy |
| Receive (no data) | 50-100 ns | Just atomic load |

### Throughput

| Scenario | Throughput | Notes |
|----------|------------|-------|
| Small messages (64B) | 10-20M msg/sec | Limited by CPU |
| Medium messages (1KB) | 2-5M msg/sec | Memory bandwidth |
| Large messages (64KB) | 100-500K msg/sec | Memory bandwidth limited |

### Memory Overhead

- **Fixed**: 128 bytes (SharedMemoryHeader)
- **Per message**: 32 bytes (MessageHeader)
- **Ring buffer**: User-configurable (typically 64KB - 16MB)

### CPU Usage

- **Sender**: Near-zero (just memory writes)
- **Receiver**: Depends on polling strategy
  - Spin-loop: 100% of one core
  - Yield: Minimal when idle
  - Sleep: Trades latency for CPU

---

## Future Enhancements

### Planned Features

1. **Multi-producer support**: MPSC ring buffer variant
2. **Zero-copy large messages**: Separate buffer pool
3. **Backpressure API**: Blocking send with timeout
4. **RDMA backend**: For RDMA-capable networks
5. **Monitoring**: Built-in metrics and tracing

### Extension Points

- **Custom allocators**: For ring buffer memory
- **Message filters**: Pre-processing on receive
- **Compression**: Transparent payload compression
- **Encryption**: Optional payload encryption

---

## Comparison to Alternatives

| Feature | SwiftChannel | Unix Sockets | Pipes | Message Queues |
|---------|--------------|--------------|-------|----------------|
| Latency | Very Low | Medium | Medium | High |
| Throughput | Very High | Medium | Medium | Low |
| Zero-copy | ✅ | ❌ | ❌ | ❌ |
| Easy integration | ✅ | ❌ | ❌ | ❌ |
| Cross-platform | ✅ | Partial | ✅ | Partial |
| Type-safety | ✅ | ❌ | ❌ | ❌ |

---

## References

- **Lock-Free Programming**: [Preshing on Programming](https://preshing.com/20120612/an-introduction-to-lock-free-programming/)
- **Memory Ordering**: [C++ Memory Model](https://en.cppreference.com/w/cpp/atomic/memory_order)
- **SPSC Queues**: [Dmitry Vyukov's MPMC Queue](https://www.1024cores.net/home/lock-free-algorithms/queues)
- **Shared Memory**: [POSIX Programmer's Manual](https://man7.org/linux/man-pages/man7/shm_overview.7.html)
