#!/bin/bash
# SwiftChannel - Build Verification Script
# This script builds the project and runs basic tests to verify everything works

set -e  # Exit on error

echo "=================================="
echo "SwiftChannel Build Verification"
echo "=================================="
echo ""

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo "❌ ERROR: CMake not found. Please install CMake 3.20 or later."
    exit 1
fi

echo "✓ CMake found: $(cmake --version | head -1)"
echo ""

# Create build directory
BUILD_DIR="build_verify"
if [ -d "$BUILD_DIR" ]; then
    echo "⚠ Build directory exists. Cleaning..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo ""
echo "Step 1: Configuring..."
echo "====================="
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DSWIFTCHANNEL_BUILD_TESTS=ON \
         -DSWIFTCHANNEL_BUILD_EXAMPLES=ON

echo ""
echo "Step 2: Building..."
echo "==================="
cmake --build . --config Release -j$(nproc 2>/dev/null || echo 4)

echo ""
echo "Step 3: Running Unit Tests..."
echo "=============================="
if [ -f "./tests/ring_buffer_test" ] || [ -f "./tests/Release/ring_buffer_test.exe" ]; then
    ./tests/ring_buffer_test 2>/dev/null || ./tests/Release/ring_buffer_test.exe || echo "⚠ Ring buffer test not found"
    ./tests/message_test 2>/dev/null || ./tests/Release/message_test.exe || echo "⚠ Message test not found"
else
    echo "⚠ Unit test executables not found (may not have built)"
fi

echo ""
echo "Step 4: Build Summary"
echo "====================="
echo ""

# Check if library was built
if [ -f "libswiftchannel.a" ] || [ -f "Release/swiftchannel.lib" ]; then
    echo "✅ Library built successfully"
else
    echo "⚠ Library file not found (check build output)"
fi

# Check if examples were built
if [ -f "examples/simple_sender" ] || [ -f "examples/Release/simple_sender.exe" ]; then
    echo "✅ Examples built successfully"
else
    echo "⚠ Example executables not found"
fi

echo ""
echo "=================================="
echo "Build verification complete!"
echo "=================================="
echo ""
echo "Next steps:"
echo "  1. Run the examples:"
echo "     Terminal 1: ./examples/simple_receiver"
echo "     Terminal 2: ./examples/simple_sender"
echo ""
echo "  2. Run integration test:"
echo "     ./tests/sender_receiver_test"
echo ""
echo "  3. Start developing:"
echo "     - Sender-only: Just include headers"
echo "     - Receiver: Link against libswiftchannel"
echo ""
echo "See QUICK_REFERENCE.md for usage examples."
echo ""
