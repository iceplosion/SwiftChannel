cmake_minimum_required(VERSION 3.20)

# Simple sender example
add_executable(simple_sender
    simple_sender/main.cpp
)

target_link_libraries(simple_sender PRIVATE swiftchannel)
target_include_directories(simple_sender PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Simple receiver example
add_executable(simple_receiver
    simple_receiver/main.cpp
)

target_link_libraries(simple_receiver PRIVATE swiftchannel)
target_include_directories(simple_receiver PRIVATE ${CMAKE_SOURCE_DIR}/include)
