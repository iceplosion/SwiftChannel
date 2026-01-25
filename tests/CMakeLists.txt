cmake_minimum_required(VERSION 3.20)

# Find or fetch a test framework (using a simple approach here)
# In production, you'd use FetchContent or find_package for GTest/Catch2

add_executable(ring_buffer_test
    unit/ring_buffer_test.cpp
)

target_link_libraries(ring_buffer_test PRIVATE swiftchannel)
target_include_directories(ring_buffer_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(NAME ring_buffer_test COMMAND ring_buffer_test)

add_executable(message_test
    unit/message_test.cpp
)

target_link_libraries(message_test PRIVATE swiftchannel)
target_include_directories(message_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(NAME message_test COMMAND message_test)

add_executable(sender_receiver_test
    integration/sender_receiver_test.cpp
)

target_link_libraries(sender_receiver_test PRIVATE swiftchannel)
target_include_directories(sender_receiver_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(NAME sender_receiver_test COMMAND sender_receiver_test)
