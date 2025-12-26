#pragma once
#include <vector>
#include <memory>
#include "Neuron.h"
#include "Connection.h"

// Forward declarations
class AudioManager;
class RhythmInterpreter;
class Quantizer;

class NeuronNetwork {
private:
    std::vector<std::unique_ptr<Neuron>> neurons;
    std::vector<std::unique_ptr<Connection>> connections;
    AudioManager* audioManager;
    RhythmInterpreter* rhythmInterpreter; // Use raw pointer to avoid incomplete type issues
    
    // Rhythm-to-neuron mapping matrix (filterIndex -> neuronIndex -> weight)
    std::vector<std::vector<float>> rhythmConnectionMatrix;
    
    Quantizer* quantizer; // Quantization system for musical timing

    // Learning state
    bool learningEnabled = false;
    float learningRate = 0.01f;
    float weightDecay = 0.0005f; // small decay to prevent runaway growth
    float maxWeight = 1.0f;      // clamp weights for stability
    float mappingGain = 0.2f;    // global scaling for rhythm mapping strength
    float onsetBias = 0.5f;      // 0.0 = filter output only, 1.0 = onset only, 0.5 = balanced
    
    void ensureMatrixSize(size_t bandCount, size_t neuronCount);
    // Per-neuron rhythmogram band mapping (user configurable)
    std::vector<size_t> neuronBandMap; // size = neurons, values in [0, bandCount)
    size_t assignedBandForNeuron(size_t neuronIndex) const;

public:
    NeuronNetwork();
    ~NeuronNetwork();
    
    void setAudioManager(AudioManager* manager);
    AudioManager* getAudioManager() const { return audioManager; }
    
    void setQuantizer(Quantizer* quantizer);
    Quantizer* getQuantizer() const { return quantizer; }
    
    Neuron* addNeuron(int sampleIndex, float initialActivation = 0.0f, 
                      float threshold = 1.0f, float decayRate = 1.0f, 
                      float activationIncreasePerIteration = 0.0f,
                      ActivationFunction func = ActivationFunction::Linear,
                      const std::string& sampleFilePath = "");
    
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

    // Learning controls
    void setLearningEnabled(bool enabled) { learningEnabled = enabled; }
    bool isLearningEnabled() const { return learningEnabled; }
    void setLearningRate(float rate) { learningRate = std::max(0.0f, rate); }
    float getLearningRate() const { return learningRate; }
    void setWeightDecay(float decay) { weightDecay = std::max(0.0f, decay); }
    float getWeightDecay() const { return weightDecay; }
    void resetRhythmWeights(float value = 0.0f);
    void learnFromRhythm();
    // Global mapping gain
    void setMappingGain(float gain) { mappingGain = std::max(0.0f, std::min(gain, 1.0f)); }
    float getMappingGain() const { return mappingGain; }
    // Onset bias (balance between onset and filter output learning)
    void setOnsetBias(float bias) { onsetBias = std::max(0.0f, std::min(bias, 1.0f)); }
    float getOnsetBias() const { return onsetBias; }

    // Mapping controls
    void setNeuronBandMapping(size_t neuronIndex, size_t bandIndex);
    size_t getNeuronBandMapping(size_t neuronIndex) const;
    
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