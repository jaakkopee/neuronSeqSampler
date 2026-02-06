#ifndef EXAMPLEMODULE_H
#define EXAMPLEMODULE_H

#include "BaseModule.h"
#include <map>
#include <iostream>

class ExampleModule : public BaseModule {
public:
    ExampleModule(const std::string& name) : name_(name) {}
    
    void receiveInput(const std::string& inputName, const std::any& data) override;
    void processData(Switchboard& switchboard) override;
    
    const std::string& getName() const { return name_; }

private:
    std::string name_;
    std::map<std::string, std::any> inputData_;
};

#endif // EXAMPLEMODULE_H
