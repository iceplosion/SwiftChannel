# Compiler options for SwiftChannel

if(MSVC)
    # MSVC-specific options
    add_compile_options(
        /W4                 # Warning level 4
        /WX                 # Treat warnings as errors (optional, comment out if too strict)
        /permissive-        # Standards conformance
        /Zc:__cplusplus     # Enable correct __cplusplus macro
        /MP                 # Multi-processor compilation
    )

    # Optimization flags for Release
    add_compile_options(
        $<$<CONFIG:Release>:/O2>        # Maximum optimization
        $<$<CONFIG:Release>:/Ob2>       # Inline expansion
        $<$<CONFIG:Release>:/Oi>        # Enable intrinsics
        $<$<CONFIG:Release>:/Ot>        # Favor fast code
        $<$<CONFIG:Release>:/GL>        # Whole program optimization
    )

    # Link-time optimization for Release
    add_link_options(
        $<$<CONFIG:Release>:/LTCG>      # Link-time code generation
    )

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang options
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
    )

    # Optimization flags for Release
    add_compile_options(
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-march=native>
        $<$<CONFIG:Release>:-flto>      # Link-time optimization
    )

    add_link_options(
        $<$<CONFIG:Release>:-flto>
    )
endif()

# Cache line size hint (can be overridden)
if(NOT DEFINED SWIFTCHANNEL_CACHE_LINE_SIZE)
    set(SWIFTCHANNEL_CACHE_LINE_SIZE 64 CACHE STRING "CPU cache line size")
endif()

add_compile_definitions(SWIFTCHANNEL_CACHE_LINE_SIZE=${SWIFTCHANNEL_CACHE_LINE_SIZE})
