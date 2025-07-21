#pragma once
#include <vector>
#include <memory>
#include "Neuron.h"
#include "Connection.h"

// Forward declaration
class AudioManager;

class NeuronNetwork {
private:
    std::vector<std::unique_ptr<Neuron>> neurons;
    std::vector<std::unique_ptr<Connection>> connections;
    AudioManager* audioManager;

public:
    NeuronNetwork();
    ~NeuronNetwork() = default;
    
    void setAudioManager(AudioManager* manager);
    
    Neuron* addNeuron(int sampleIndex, float initialActivation = 0.0f, 
                      float threshold = 1.0f, float decayRate = 1.0f, 
                      ActivationFunction func = ActivationFunction::Linear);
    
    Connection* connect(Neuron* source, Neuron* target, float weight = 1.0f);
    
    void activate();
    void resetFiredFlags();
    
    // Getters
    const std::vector<std::unique_ptr<Neuron>>& getNeurons() const { return neurons; }
    const std::vector<std::unique_ptr<Connection>>& getConnections() const { return connections; }
    size_t getNeuronCount() const { return neurons.size(); }
    size_t getConnectionCount() const { return connections.size(); }
    
    // Get neuron by index (for GUI access)
    Neuron* getNeuron(size_t index) { 
        return (index < neurons.size()) ? neurons[index].get() : nullptr; 
    }
    
    // Get connection by index (for GUI access)
    Connection* getConnection(size_t index) { 
        return (index < connections.size()) ? connections[index].get() : nullptr; 
    }
};