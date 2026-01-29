#include <swiftchannel/sender/message.hpp>
#include <iostream>
#include <cassert>

using namespace swiftchannel;

struct TestMessage {
    int id;
    double value;
    char tag[16];
};

int main() {
    std::cout << "Running message tests...\n";

    // Test 1: Typed message
    {
        TestMessage msg{42, 3.14, "test"};
        Message<TestMessage> typed_msg(msg);

        assert(typed_msg.size() == sizeof(TestMessage));
        assert(typed_msg.data().id == 42);
        assert(typed_msg.data().value == 3.14);

        std::cout << "  [PASS] Typed message test passed\n";
    }

    // Test 2: Dynamic message
    {
        DynamicMessage dyn_msg(128);
        assert(dyn_msg.size() == 128);

        // Write some data
        int* data = static_cast<int*>(dyn_msg.data());
        data[0] = 100;
        data[1] = 200;

        assert(data[0] == 100);
        assert(data[1] == 200);

        std::cout << "  [PASS] Dynamic message test passed\n";
    }

    // Test 3: Dynamic message from value
    {
        TestMessage msg{99, 2.71, "dynamic"};
        DynamicMessage dyn_msg(msg);

        assert(dyn_msg.size() == sizeof(TestMessage));

        const TestMessage* retrieved = static_cast<const TestMessage*>(dyn_msg.data());
        assert(retrieved->id == 99);
        assert(retrieved->value == 2.71);
        (void)retrieved; // Mark as used

        std::cout << "  [PASS] Dynamic message from value test passed\n";
    }

    std::cout << "All message tests passed!\n";
    return 0;
}
