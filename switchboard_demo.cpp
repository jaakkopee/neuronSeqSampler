#include <iostream>
#include "src/Switchboard.h"
#include "src/BaseModule.h"
#include "src/ExampleModule.h"

int main() {
    std::cout << "=== Switchboard Demo ===" << std::endl;
    std::cout << std::endl;
    
    // Create the switchboard
    Switchboard switchboard;
    
    // Create example modules
    ExampleModule moduleA("ModuleA");
    ExampleModule moduleB("ModuleB");
    ExampleModule moduleC("ModuleC");
    
    std::cout << "--- Registering Modules ---" << std::endl;
    // Register modules with their inputs and outputs
    switchboard.registerModule("ModuleA", {"value"}, {"result"}, &moduleA);
    switchboard.registerModule("ModuleB", {"value"}, {"result"}, &moduleB);
    switchboard.registerModule("ModuleC", {"value"}, {"result"}, &moduleC);
    
    std::cout << std::endl;
    std::cout << "--- Setting up Connections ---" << std::endl;
    // Connect modules: A -> B -> C
    switchboard.connect("ModuleA.result", "ModuleB.value");
    switchboard.connect("ModuleB.result", "ModuleC.value");
    
    std::cout << std::endl;
    std::cout << "--- Simulating Data Flow ---" << std::endl;
    
    // Send initial data to ModuleA
    std::cout << "Sending initial value 5.0 to ModuleA" << std::endl;
    moduleA.receiveInput("value", 5.0);
    
    std::cout << std::endl;
    std::cout << "Processing data through the pipeline:" << std::endl;
    
    // Process data through the pipeline
    moduleA.processData(switchboard);  // 5.0 * 2 = 10.0 -> ModuleB
    moduleB.processData(switchboard);  // 10.0 * 2 = 20.0 -> ModuleC
    moduleC.processData(switchboard);  // 20.0 * 2 = 40.0
    
    std::cout << std::endl;
    std::cout << "--- Testing Multiple Connections ---" << std::endl;
    
    // Add another connection from ModuleA to ModuleC (bypass ModuleB)
    switchboard.connect("ModuleA.result", "ModuleC.value");
    
    std::cout << "Sending another value 3.0 to ModuleA" << std::endl;
    moduleA.receiveInput("value", 3.0);
    
    std::cout << std::endl;
    moduleA.processData(switchboard);  // 3.0 * 2 = 6.0 -> ModuleB and ModuleC
    moduleB.processData(switchboard);  // 6.0 * 2 = 12.0 -> ModuleC
    moduleC.processData(switchboard);  // Process both 6.0 and 12.0
    
    std::cout << std::endl;
    std::cout << "=== Demo Complete ===" << std::endl;
    
    return 0;
}
