#include "Switchboard.h"
#include "BaseModule.h"
#include <iostream>
#include <stdexcept>

void Switchboard::registerModule(const std::string& moduleName,
                                 const std::vector<std::string>& inputs,
                                 const std::vector<std::string>& outputs,
                                 BaseModule* modulePtr) {
    if (modules_.find(moduleName) != modules_.end()) {
        throw std::runtime_error("Module '" + moduleName + "' already registered");
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
