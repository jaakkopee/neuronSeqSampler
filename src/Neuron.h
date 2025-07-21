#pragma once
#include <functional>
#include <vector>

// Forward declaration
class AudioManager;

enum class ActivationFunction {
    Linear,
    Sigmoid,
    ReLU,
    Tanh
};

class Neuron {
private:
    float activation;
    float threshold;
    float decayRate;
    int sampleIndex;
    std::vector<float> activationHistory;
    static const size_t maxHistoryLength = 100;
    ActivationFunction activationFunc;
    bool hasFired;
    AudioManager* audioManager;

public:
    Neuron(int sampleIndex, float initialActivation = 0.0f, 
           float threshold = 1.0f, float decayRate = 1.0f, 
           ActivationFunction func = ActivationFunction::Linear);
    
    void setAudioManager(AudioManager* manager);
    float activate(float inputValue);
    void playSample();
    
    // Getters
    float getActivation() const { return activation; }
    float getThreshold() const { return threshold; }
    bool getHasFired() const { return hasFired; }
    int getSampleIndex() const { return sampleIndex; }
    const std::vector<float>& getActivationHistory() const { return activationHistory; }
    
    // Setters
    void setActivation(float value) { activation = value; }
    void setThreshold(float value) { threshold = value; }
    void setDecayRate(float value) { decayRate = value; }
    void resetFiredFlag() { hasFired = false; }

private:
    float applyActivationFunction(float value);
};