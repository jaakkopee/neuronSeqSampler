#include "ExampleModule.h"
#include "Switchboard.h"
#include <iostream>

void ExampleModule::receiveInput(const std::string& inputName, const std::any& data) {
    inputData_[inputName] = data;
    std::cout << "[" << name_ << "] Received input on '" << inputName << "'" << std::endl;
}

void ExampleModule::processData(Switchboard& switchboard) {
    // Example: Double the value from input and send to output
    if (inputData_.find("value") != inputData_.end()) {
        try {
            double inputValue = std::any_cast<double>(inputData_["value"]);
            double outputValue = inputValue * 2.0;
            
            std::cout << "[" << name_ << "] Processing: " << inputValue 
                     << " -> " << outputValue << std::endl;
            
            // Route the processed data through the switchboard
            std::string outputKey = name_ + ".result";
            switchboard.routeData(outputKey, outputValue);
            
            // Clear processed input
            inputData_.erase("value");
        } catch (const std::bad_any_cast& e) {
            std::cerr << "[" << name_ << "] Error: Invalid data type for 'value' input" << std::endl;
        }
    }
}
