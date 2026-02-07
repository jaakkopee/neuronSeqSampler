#ifndef BASEMODULE_H
#define BASEMODULE_H

#include <string>
#include <any>

class Switchboard;

class BaseModule {
public:
    virtual ~BaseModule() = default;
    
    // Receive input data
    virtual void receiveInput(const std::string& inputName, const std::any& data) = 0;
    
    // Process data and send outputs through the switchboard
    virtual void processData(Switchboard& switchboard) = 0;
};

#endif // BASEMODULE_H
