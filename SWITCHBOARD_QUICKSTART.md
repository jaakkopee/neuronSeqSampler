# Switchboard Quick Start Guide

## What is the Switchboard?

The Switchboard is a communication system that allows different modules to send and receive data without being directly connected to each other.

## Quick Example

```cpp
#include "src/Switchboard.h"
#include "src/ExampleModule.h"

int main() {
    // 1. Create switchboard and modules
    Switchboard switchboard;
    ExampleModule moduleA("ModuleA");
    ExampleModule moduleB("ModuleB");
    
    // 2. Register modules with their inputs and outputs
    switchboard.registerModule("ModuleA", {"value"}, {"result"}, &moduleA);
    switchboard.registerModule("ModuleB", {"value"}, {"result"}, &moduleB);
    
    // 3. Connect modules
    switchboard.connect("ModuleA.result", "ModuleB.value");
    
    // 4. Send data and process
    moduleA.receiveInput("value", 5.0);
    moduleA.processData(switchboard);  // 5.0 * 2 = 10.0 -> ModuleB
    moduleB.processData(switchboard);  // 10.0 * 2 = 20.0
    
    return 0;
}
```

## Building

```bash
g++ -std=c++17 -I. your_file.cpp src/Switchboard.cpp src/ExampleModule.cpp -o your_program
```

## Creating a Custom Module

```cpp
#include "src/BaseModule.h"

class MyModule : public BaseModule {
private:
    std::map<std::string, std::any> data_;
    std::string name_;
    
public:
    MyModule(const std::string& name) : name_(name) {}
    
    void receiveInput(const std::string& inputName, const std::any& data) override {
        data_[inputName] = data;
    }
    
    void processData(Switchboard& switchboard) override {
        // Your processing logic here
        // Example: forward data
        if (data_.count("input") > 0) {
            switchboard.routeData(name_ + ".output", data_["input"]);
        }
    }
};
```

## Common Patterns

### Pipeline
```cpp
A -> B -> C
switchboard.connect("A.output", "B.input");
switchboard.connect("B.output", "C.input");
```

### Broadcast
```cpp
    A
   / \
  B   C
switchboard.connect("A.output", "B.input");
switchboard.connect("A.output", "C.input");
```

### Merge
```cpp
  A   B
   \ /
    C
switchboard.connect("A.output", "C.input");
switchboard.connect("B.output", "C.input");
```

## Testing

```bash
# Run the demo
./switchboard_demo

# Run tests
./test_switchboard
```

## Documentation

For detailed documentation, see:
- **SWITCHBOARD_DOCUMENTATION.md** - Complete user guide
- **SWITCHBOARD_IMPLEMENTATION_SUMMARY.md** - Technical details
- **SWITCHBOARD_SECURITY_SUMMARY.md** - Security analysis

## Files to Include

When using the switchboard in your project, include:
- `src/Switchboard.h` and `src/Switchboard.cpp`
- `src/BaseModule.h`
- Your custom module files
