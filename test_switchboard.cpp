#include <iostream>
#include <cassert>
#include <cmath>
#include "src/Switchboard.h"
#include "src/BaseModule.h"
#include "src/ExampleModule.h"

// Test helper macros
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running test: " << #name << "..."; \
    test_##name(); \
    std::cout << " PASSED" << std::endl; \
} while(0)

// Custom module for testing
class CounterModule : public BaseModule {
public:
    CounterModule(const std::string& name) : name_(name), count_(0) {}
    
    void receiveInput(const std::string& inputName, const std::any& data) override {
        count_++;
        lastInput_ = inputName;
        lastData_ = data;
    }
    
    void processData(Switchboard& switchboard) override {
        // Just forwards the data with a count
        if (count_ > 0) {
            std::string outputKey = name_ + ".output";
            switchboard.routeData(outputKey, count_);
        }
    }
    
    int getCount() const { return count_; }
    std::string getLastInput() const { return lastInput_; }
    
private:
    std::string name_;
    int count_;
    std::string lastInput_;
    std::any lastData_;
};

// Test 1: Module Registration
TEST(module_registration) {
    Switchboard switchboard;
    ExampleModule module("TestModule");
    
    switchboard.registerModule("TestModule", {"input1", "input2"}, {"output1"}, &module);
    
    const auto* info = switchboard.getModuleInfo("TestModule");
    assert(info != nullptr);
    assert(info->inputs.size() == 2);
    assert(info->outputs.size() == 1);
    assert(info->inputs[0] == "input1");
    assert(info->inputs[1] == "input2");
    assert(info->outputs[0] == "output1");
}

// Test 2: Connection Setup
TEST(connection_setup) {
    Switchboard switchboard;
    ExampleModule moduleA("ModuleA");
    ExampleModule moduleB("ModuleB");
    
    switchboard.registerModule("ModuleA", {"input"}, {"output"}, &moduleA);
    switchboard.registerModule("ModuleB", {"input"}, {"output"}, &moduleB);
    
    // This should not throw
    switchboard.connect("ModuleA.output", "ModuleB.input");
}

// Test 3: Data Routing
TEST(data_routing) {
    Switchboard switchboard;
    CounterModule moduleA("ModuleA");
    CounterModule moduleB("ModuleB");
    
    switchboard.registerModule("ModuleA", {"input"}, {"output"}, &moduleA);
    switchboard.registerModule("ModuleB", {"input"}, {"output"}, &moduleB);
    
    switchboard.connect("ModuleA.output", "ModuleB.input");
    
    // Send data from ModuleA
    moduleA.receiveInput("input", 42);
    moduleA.processData(switchboard);
    
    // Check that ModuleB received data
    assert(moduleB.getCount() == 1);
    assert(moduleB.getLastInput() == "input");
}

// Test 4: Multiple Connections
TEST(multiple_connections) {
    Switchboard switchboard;
    CounterModule moduleA("ModuleA");
    CounterModule moduleB("ModuleB");
    CounterModule moduleC("ModuleC");
    
    switchboard.registerModule("ModuleA", {"input"}, {"output"}, &moduleA);
    switchboard.registerModule("ModuleB", {"input"}, {"output"}, &moduleB);
    switchboard.registerModule("ModuleC", {"input"}, {"output"}, &moduleC);
    
    // Connect A to both B and C
    switchboard.connect("ModuleA.output", "ModuleB.input");
    switchboard.connect("ModuleA.output", "ModuleC.input");
    
    moduleA.receiveInput("input", 100);
    moduleA.processData(switchboard);
    
    // Both B and C should have received data
    assert(moduleB.getCount() == 1);
    assert(moduleC.getCount() == 1);
}

// Test 5: Pipeline Processing
TEST(pipeline_processing) {
    Switchboard switchboard;
    ExampleModule moduleA("ModuleA");
    ExampleModule moduleB("ModuleB");
    ExampleModule moduleC("ModuleC");
    
    switchboard.registerModule("ModuleA", {"value"}, {"result"}, &moduleA);
    switchboard.registerModule("ModuleB", {"value"}, {"result"}, &moduleB);
    switchboard.registerModule("ModuleC", {"value"}, {"result"}, &moduleC);
    
    switchboard.connect("ModuleA.result", "ModuleB.value");
    switchboard.connect("ModuleB.result", "ModuleC.value");
    
    // Process through pipeline: 2.0 -> 4.0 -> 8.0 -> 16.0
    moduleA.receiveInput("value", 2.0);
    moduleA.processData(switchboard);
    moduleB.processData(switchboard);
    moduleC.processData(switchboard);
    
    // Verify the pipeline worked (no exceptions)
}

// Test 6: No Connection Handling
TEST(no_connection_handling) {
    Switchboard switchboard;
    CounterModule module("Isolated");
    
    switchboard.registerModule("Isolated", {"input"}, {"output"}, &module);
    
    // This should not crash even with no connections
    module.receiveInput("input", 42);
    module.processData(switchboard);
}

// Test 7: Nonexistent Module
TEST(nonexistent_module) {
    Switchboard switchboard;
    
    const auto* info = switchboard.getModuleInfo("NonexistentModule");
    assert(info == nullptr);
}

// Test 8: Duplicate Registration
TEST(duplicate_registration) {
    Switchboard switchboard;
    ExampleModule module1("TestModule");
    ExampleModule module2("TestModule");
    
    switchboard.registerModule("TestModule", {"input"}, {"output"}, &module1);
    
    // Attempting to register again should throw
    bool exceptionThrown = false;
    try {
        switchboard.registerModule("TestModule", {"input"}, {"output"}, &module2);
    } catch (const std::runtime_error& e) {
        exceptionThrown = true;
    }
    assert(exceptionThrown);
}

int main() {
    std::cout << "=== Switchboard Test Suite ===" << std::endl;
    std::cout << std::endl;
    
    RUN_TEST(module_registration);
    RUN_TEST(connection_setup);
    RUN_TEST(data_routing);
    RUN_TEST(multiple_connections);
    RUN_TEST(pipeline_processing);
    RUN_TEST(no_connection_handling);
    RUN_TEST(nonexistent_module);
    RUN_TEST(duplicate_registration);
    
    std::cout << std::endl;
    std::cout << "=== All Tests Passed ===" << std::endl;
    
    return 0;
}
