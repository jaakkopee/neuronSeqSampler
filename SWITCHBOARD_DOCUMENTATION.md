# Switchboard Module System

## Overview

The Switchboard system provides a flexible, extensible architecture for managing communication between modules in the neuronSeqSampler application. It acts as a central controller that handles module registration, connection management, and data routing.

## Architecture

### Core Components

#### 1. Switchboard Class (`src/Switchboard.h`, `src/Switchboard.cpp`)

The central controller that manages module communication.

**Key Methods:**
- `void registerModule(moduleName, inputs, outputs, modulePtr)` - Register a module with its interface
- `void connect(outputKey, inputKey)` - Connect an output to an input
- `void routeData(outputKey, data)` - Route data from an output to all connected inputs
- `const ModuleInfo* getModuleInfo(moduleName)` - Get information about a registered module

**Example:**
```cpp
Switchboard switchboard;
ExampleModule module("MyModule");
switchboard.registerModule("MyModule", {"input1"}, {"output1"}, &module);
```

#### 2. BaseModule Abstract Class (`src/BaseModule.h`)

Defines the standard interface that all modules must implement.

**Required Methods:**
- `void receiveInput(inputName, data)` - Receive data on a named input
- `void processData(Switchboard& switchboard)` - Process data and send outputs

**Example:**
```cpp
class CustomModule : public BaseModule {
public:
    void receiveInput(const std::string& inputName, const std::any& data) override {
        // Store or process incoming data
    }
    
    void processData(Switchboard& switchboard) override {
        // Process and route output data
        switchboard.routeData("CustomModule.output", result);
    }
};
```

#### 3. ExampleModule Class (`src/ExampleModule.h`, `src/ExampleModule.cpp`)

A concrete implementation demonstrating module creation.

**Functionality:**
- Receives numeric input on "value" input port
- Doubles the value
- Outputs result on "result" output port

## Usage

### 1. Create Modules

```cpp
ExampleModule moduleA("ModuleA");
ExampleModule moduleB("ModuleB");
```

### 2. Create Switchboard and Register Modules

```cpp
Switchboard switchboard;
switchboard.registerModule("ModuleA", {"value"}, {"result"}, &moduleA);
switchboard.registerModule("ModuleB", {"value"}, {"result"}, &moduleB);
```

### 3. Connect Modules

```cpp
// Connect ModuleA's result output to ModuleB's value input
switchboard.connect("ModuleA.result", "ModuleB.value");
```

### 4. Send Data and Process

```cpp
// Send initial data
moduleA.receiveInput("value", 5.0);

// Process through the pipeline
moduleA.processData(switchboard);  // 5.0 * 2 = 10.0 -> ModuleB
moduleB.processData(switchboard);  // 10.0 * 2 = 20.0
```

## Connection Format

Connections use the format: `"ModuleName.PortName"`

- **Output Key**: `"ModuleA.result"` - ModuleA's "result" output
- **Input Key**: `"ModuleB.value"` - ModuleB's "value" input

## Data Types

The system uses `std::any` for flexible data passing. Modules must handle type casting:

```cpp
void receiveInput(const std::string& inputName, const std::any& data) override {
    try {
        double value = std::any_cast<double>(data);
        // Process value
    } catch (const std::bad_any_cast& e) {
        // Handle type mismatch
    }
}
```

## Building and Testing

### Build the Demo

```bash
g++ -std=c++17 -I. switchboard_demo.cpp src/Switchboard.cpp src/ExampleModule.cpp -o switchboard_demo
./switchboard_demo
```

Or using CMake:

```bash
cmake -B build_sb -DCMAKE_BUILD_TYPE=Release -S . -C CMakeLists_switchboard.txt
cmake --build build_sb
./build_sb/switchboard_demo
```

### Run Tests

```bash
g++ -std=c++17 -I. test_switchboard.cpp src/Switchboard.cpp src/ExampleModule.cpp -o test_switchboard
./test_switchboard
```

## Design Benefits

1. **Decoupling**: Modules don't need to know about each other directly
2. **Flexibility**: Connections can be reconfigured without changing module code
3. **Extensibility**: New modules can be added without modifying existing ones
4. **Testability**: Individual modules can be tested in isolation
5. **Scalability**: Supports complex data flow graphs with multiple connections

## Advanced Features

### Multiple Connections

One output can connect to multiple inputs:

```cpp
switchboard.connect("ModuleA.result", "ModuleB.value");
switchboard.connect("ModuleA.result", "ModuleC.value");
```

### Pipeline Processing

Create processing chains:

```cpp
switchboard.connect("ModuleA.result", "ModuleB.value");
switchboard.connect("ModuleB.result", "ModuleC.value");
// Data flows: A -> B -> C
```

### Broadcast Networks

Create complex topologies:

```cpp
// Fan-out: A broadcasts to B and C
switchboard.connect("ModuleA.result", "ModuleB.value");
switchboard.connect("ModuleA.result", "ModuleC.value");

// Fan-in: Both B and C feed into D
switchboard.connect("ModuleB.result", "ModuleD.value");
switchboard.connect("ModuleC.result", "ModuleD.value");
```

## Creating Custom Modules

To create a new module:

1. Inherit from `BaseModule`
2. Implement `receiveInput()` to handle incoming data
3. Implement `processData()` to process and route outputs
4. Register with the switchboard
5. Connect to other modules

Example:

```cpp
class AverageModule : public BaseModule {
private:
    std::vector<double> values_;
    std::string name_;
    
public:
    AverageModule(const std::string& name) : name_(name) {}
    
    void receiveInput(const std::string& inputName, const std::any& data) override {
        if (inputName == "value") {
            values_.push_back(std::any_cast<double>(data));
        }
    }
    
    void processData(Switchboard& switchboard) override {
        if (!values_.empty()) {
            double sum = 0.0;
            for (double v : values_) sum += v;
            double average = sum / values_.size();
            
            switchboard.routeData(name_ + ".average", average);
            values_.clear();
        }
    }
};
```

## Integration with Existing Code

The switchboard system is designed to integrate seamlessly with existing neuronSeqSampler components. Future modules could include:

- Audio processing modules
- Neuron network modules
- Visualization modules
- MIDI/OSC communication modules
- Recording and playback modules

## Files

- `src/Switchboard.h` - Switchboard class declaration
- `src/Switchboard.cpp` - Switchboard class implementation
- `src/BaseModule.h` - Abstract base class for modules
- `src/ExampleModule.h` - Example module declaration
- `src/ExampleModule.cpp` - Example module implementation
- `switchboard_demo.cpp` - Demonstration program
- `test_switchboard.cpp` - Comprehensive test suite
- `CMakeLists_switchboard.txt` - CMake configuration for switchboard demo
