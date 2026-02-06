#include "Switchboard.h"
#include "BaseModule.h"
#include <iostream>
#include <stdexcept>

void Switchboard::registerModule(const std::string& moduleName,
                                 const std::vector<std::string>& inputs,
                                 const std::vector<std::string>& outputs,
                                 BaseModule* modulePtr) {
    if (modules_.find(moduleName) != modules_.end()) {
        throw std::runtime_error("Module '" + moduleName + "' is already registered. Use a different name or unregister the existing module first.");
    }
    
    ModuleInfo info;
    info.inputs = inputs;
    info.outputs = outputs;
    info.modulePtr = modulePtr;
    
    modules_[moduleName] = info;
    
    std::cout << "Registered module: " << moduleName 
              << " with " << inputs.size() << " inputs and " 
              << outputs.size() << " outputs" << std::endl;
}

void Switchboard::connect(const std::string& outputKey, const std::string& inputKey) {
    // Validate output key format and module existence
    size_t outDotPos = outputKey.find('.');
    if (outDotPos == std::string::npos) {
        throw std::runtime_error("Invalid output key format: '" + outputKey + "'. Expected format: 'ModuleName.outputName'");
    }
    std::string outModuleName = outputKey.substr(0, outDotPos);
    std::string outPortName = outputKey.substr(outDotPos + 1);
    
    auto outModuleIt = modules_.find(outModuleName);
    if (outModuleIt == modules_.end()) {
        throw std::runtime_error("Output module '" + outModuleName + "' not registered");
    }
    
    // Check if output port exists
    bool outputExists = false;
    for (const auto& output : outModuleIt->second.outputs) {
        if (output == outPortName) {
            outputExists = true;
            break;
        }
    }
    if (!outputExists) {
        throw std::runtime_error("Output port '" + outPortName + "' does not exist in module '" + outModuleName + "'");
    }
    
    // Validate input key format and module existence
    size_t inDotPos = inputKey.find('.');
    if (inDotPos == std::string::npos) {
        throw std::runtime_error("Invalid input key format: '" + inputKey + "'. Expected format: 'ModuleName.inputName'");
    }
    std::string inModuleName = inputKey.substr(0, inDotPos);
    std::string inPortName = inputKey.substr(inDotPos + 1);
    
    auto inModuleIt = modules_.find(inModuleName);
    if (inModuleIt == modules_.end()) {
        throw std::runtime_error("Input module '" + inModuleName + "' not registered");
    }
    
    // Check if input port exists
    bool inputExists = false;
    for (const auto& input : inModuleIt->second.inputs) {
        if (input == inPortName) {
            inputExists = true;
            break;
        }
    }
    if (!inputExists) {
        throw std::runtime_error("Input port '" + inPortName + "' does not exist in module '" + inModuleName + "'");
    }
    
    connections_[outputKey].push_back(inputKey);
    std::cout << "Connected: " << outputKey << " -> " << inputKey << std::endl;
}

void Switchboard::routeData(const std::string& outputKey, const std::any& data) {
    auto it = connections_.find(outputKey);
    if (it == connections_.end()) {
        std::cout << "No connections for output: " << outputKey << std::endl;
        return;
    }
    
    for (const auto& inputKey : it->second) {
        // Parse inputKey to find module and input name
        size_t dotPos = inputKey.find('.');
        if (dotPos == std::string::npos) {
            std::cerr << "Invalid input key format: " << inputKey << std::endl;
            continue;
        }
        
        std::string moduleName = inputKey.substr(0, dotPos);
        std::string inputName = inputKey.substr(dotPos + 1);
        
        auto moduleIt = modules_.find(moduleName);
        if (moduleIt == modules_.end()) {
            std::cerr << "Module not found: " << moduleName << std::endl;
            continue;
        }
        
        // Send data to the module's input
        moduleIt->second.modulePtr->receiveInput(inputName, data);
    }
}

const Switchboard::ModuleInfo* Switchboard::getModuleInfo(const std::string& moduleName) const {
    auto it = modules_.find(moduleName);
    if (it == modules_.end()) {
        return nullptr;
    }
    return &(it->second);
}
