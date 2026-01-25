@echo off
REM SwiftChannel - Build Verification Script (Windows)
REM This script builds the project and runs basic tests to verify everything works

echo ==================================
echo SwiftChannel Build Verification
echo ==================================
echo.

REM Check if CMake is available
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found. Please install CMake 3.20 or later.
    exit /b 1
)

echo CMake found
echo.

REM Create build directory
set BUILD_DIR=build_verify
if exist "%BUILD_DIR%" (
    echo Build directory exists. Cleaning...
    rmdir /s /q "%BUILD_DIR%"
)

mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

echo.
echo Step 1: Configuring...
echo =====================
cmake .. -DCMAKE_BUILD_TYPE=Release ^
         -DSWIFTCHANNEL_BUILD_TESTS=ON ^
         -DSWIFTCHANNEL_BUILD_EXAMPLES=ON

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed
    exit /b 1
)

echo.
echo Step 2: Building...
echo ===================
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    exit /b 1
)

echo.
echo Step 3: Running Unit Tests...
echo ==============================
if exist "tests\Release\ring_buffer_test.exe" (
    tests\Release\ring_buffer_test.exe
    tests\Release\message_test.exe
) else (
    echo Unit test executables not found
)

echo.
echo Step 4: Build Summary
echo =====================
echo.

REM Check if library was built
if exist "Release\swiftchannel.lib" (
    echo [OK] Library built successfully
) else (
    echo [WARN] Library file not found
)

REM Check if examples were built
if exist "examples\Release\simple_sender.exe" (
    echo [OK] Examples built successfully
) else (
    echo [WARN] Example executables not found
)

echo.
echo ==================================
echo Build verification complete!
echo ==================================
echo.
echo Next steps:
echo   1. Run the examples:
echo      Terminal 1: examples\Release\simple_receiver.exe
echo      Terminal 2: examples\Release\simple_sender.exe
echo.
echo   2. Run integration test:
echo      tests\Release\sender_receiver_test.exe
echo.
echo   3. Start developing:
echo      - Sender-only: Just include headers
echo      - Receiver: Link against swiftchannel.lib
echo.
echo See QUICK_REFERENCE.md for usage examples.
echo.

cd ..
