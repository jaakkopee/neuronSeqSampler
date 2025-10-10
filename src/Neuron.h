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
    float activationIncreasePerIteration;
    int sampleIndex;
    std::vector<float> activationHistory;
    static const size_t maxHistoryLength = 100;
    ActivationFunction activationFunc;
    bool hasFired;
    AudioManager* audioManager;

public:
    Neuron(int sampleIndex, float initialActivation = 0.0f, 
           float threshold = 1.0f, float decayRate = 1.0f, 
           float activationIncreasePerIteration = 0.0f,
           ActivationFunction func = ActivationFunction::Linear);
    
    void setAudioManager(AudioManager* manager);
    float activate(float inputValue);
    void update(); // Update neuron state (apply activation_increase_per_iteration)
    void playSample();
    void playSample(float offsetSeconds); // Play sample starting from the beginning with optional time offset
    
    // Getters
    float getActivation() const { return activation; }
    float getThreshold() const { return threshold; }
    float getDecayRate() const { return decayRate; }
    float getActivationIncreasePerIteration() const { return activationIncreasePerIteration; }
    ActivationFunction getActivationFunction() const { return activationFunc; }
    bool getHasFired() const { return hasFired; }
    int getSampleIndex() const { return sampleIndex; }
    const std::vector<float>& getActivationHistory() const { return activationHistory; }
    
    // Setters
    void setActivation(float value) { activation = value; }
    void setThreshold(float value) { threshold = value; }
    void setDecayRate(float value) { decayRate = value; }
    void setActivationIncreasePerIteration(float value) { activationIncreasePerIteration = value; }
    void setActivationFunction(ActivationFunction func) { activationFunc = func; }
    void resetFiredFlag() { hasFired = false; }

private:
    float applyActivationFunction(float value);
};