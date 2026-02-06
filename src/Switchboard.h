#ifndef SWITCHBOARD_H
#define SWITCHBOARD_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <functional>
#include <any>

class BaseModule;

class Switchboard {
public:
    struct ModuleInfo {
        std::vector<std::string> inputs;
        std::vector<std::string> outputs;
        BaseModule* modulePtr;
    };

    // Register a module with its inputs and outputs
    void registerModule(const std::string& moduleName, 
                       const std::vector<std::string>& inputs,
                       const std::vector<std::string>& outputs,
                       BaseModule* modulePtr);

    // Connect an output from one module to an input of another
    void connect(const std::string& outputKey, const std::string& inputKey);

    // Route data from a module output to connected inputs
    void routeData(const std::string& outputKey, const std::any& data);

    // Get module information
    const ModuleInfo* getModuleInfo(const std::string& moduleName) const;

private:
    // Map of module names to their info
    std::map<std::string, ModuleInfo> modules_;
    
    // Map of output keys to input keys (connection routing)
    std::map<std::string, std::vector<std::string>> connections_;
};

#endif // SWITCHBOARD_H
