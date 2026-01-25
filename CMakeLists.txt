cmake_minimum_required(VERSION 3.20)
project(SwiftChannel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler options
include(cmake/compiler_options.cmake)

# Define the compiled library (receiver + infrastructure)
add_library(swiftchannel STATIC
    src/receiver/receiver.cpp
    src/receiver/dispatch.cpp
    src/sender/channel_impl.cpp
    src/ipc/shared_memory.cpp
    src/ipc/handshake.cpp
    src/diagnostics/stats.cpp
)

target_include_directories(swiftchannel
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Platform-specific sources
if (WIN32)
    target_sources(swiftchannel PRIVATE
        src/platform/windows/shm_win.cpp
        src/platform/windows/pipe_win.cpp
    )
else()
    target_sources(swiftchannel PRIVATE
        src/platform/posix/shm_posix.cpp
        src/platform/posix/socket_posix.cpp
    )
endif()

# Compiler definitions
target_compile_definitions(swiftchannel PUBLIC
    SWIFTCHANNEL_VERSION_MAJOR=1
    SWIFTCHANNEL_VERSION_MINOR=0
    SWIFTCHANNEL_VERSION_PATCH=0
)

# Enable testing
option(SWIFTCHANNEL_BUILD_TESTS "Build tests" ON)
option(SWIFTCHANNEL_BUILD_EXAMPLES "Build examples" ON)

if(SWIFTCHANNEL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(SWIFTCHANNEL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation
install(TARGETS swiftchannel
    EXPORT SwiftChannelTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/swiftchannel
    DESTINATION include
)

install(EXPORT SwiftChannelTargets
    FILE SwiftChannelTargets.cmake
    NAMESPACE SwiftChannel::
    DESTINATION lib/cmake/SwiftChannel
)

# Generate config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/SwiftChannelConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/SwiftChannelConfig.cmake
    INSTALL_DESTINATION lib/cmake/SwiftChannel
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/SwiftChannelConfig.cmake
    DESTINATION lib/cmake/SwiftChannel
)
