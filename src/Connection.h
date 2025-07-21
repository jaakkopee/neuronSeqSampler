#pragma once

// Forward declarations
class Neuron;

class Connection {
private:
    Neuron* source;
    Neuron* target;
    float weight;

public:
    Connection(Neuron* sourceNeuron, Neuron* targetNeuron, float weight = 1.0f);
    
    void activate();
    
    // Getters
    Neuron* getSource() const { return source; }
    Neuron* getTarget() const { return target; }
    float getWeight() const { return weight; }
    
    // Setters
    void setWeight(float newWeight) { weight = newWeight; }
};