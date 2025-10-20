#pragma once
#include <vector>
#include <memory>
#include "Neuron.h"
#include "Connection.h"

// Forward declarations
class AudioManager;
class RhythmInterpreter;

class NeuronNetwork {
private:
    std::vector<std::unique_ptr<Neuron>> neurons;
    std::vector<std::unique_ptr<Connection>> connections;
    AudioManager* audioManager;
    RhythmInterpreter* rhythmInterpreter; // Use raw pointer to avoid incomplete type issues
    
    // Rhythm-to-neuron connection matrix (filterIndex -> neuronIndex -> weight)
    std::vector<std::vector<float>> rhythmConnectionMatrix;

public:
    NeuronNetwork();
    ~NeuronNetwork();
    
    void setAudioManager(AudioManager* manager);
    AudioManager* getAudioManager() const { return audioManager; }
    
    Neuron* addNeuron(int sampleIndex, float initialActivation = 0.0f, 
                      float threshold = 1.0f, float decayRate = 1.0f, 
                      float activationIncreasePerIteration = 0.0f,
                      ActivationFunction func = ActivationFunction::Linear);
    
    Connection* connect(Neuron* source, Neuron* target, float weight = 1.0f);
    
    // Removal methods
    bool removeNeuron(size_t index);
    bool removeConnection(size_t index);
    void clearNetwork();
    
    void activate();
    void resetFiredFlags();
    
    // Rhythm interpreter methods
    void initializeRhythmInterpreter();
    void processAudioForRhythm(const std::vector<float>& audioData);
    std::vector<float> getProcessedAudioOutput() const; // Get filtered audio output
    RhythmInterpreter* getRhythmInterpreter() const { return rhythmInterpreter; }
    
    // Rhythm-to-neuron connection matrix methods
    void setRhythmConnection(size_t filterIndex, size_t neuronIndex, float weight);
    float getRhythmConnection(size_t filterIndex, size_t neuronIndex) const;
    void clearRhythmConnection(size_t filterIndex, size_t neuronIndex);
    void applyRhythmConnections(); // Apply rhythm filter outputs to connected neurons
    
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