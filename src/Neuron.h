#pragma once
#include <functional>
#include <string>
#include <vector>

// Forward declaration
class AudioManager;
class Quantizer;

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
    float externalInput; // Accumulates external inputs from rhythm interpreter
    int sampleIndex;
    std::string sampleFilePath; // Full pathname of the associated sample file
    std::vector<float> activationHistory;
    static const size_t maxHistoryLength = 100;
    ActivationFunction activationFunc;
    bool hasFired;
    AudioManager* audioManager;
    Quantizer* quantizer; // Musical timing quantization

public:
    Neuron(int sampleIndex, float initialActivation = 0.0f, 
           float threshold = 1.0f, float decayRate = 1.0f, 
           float activationIncreasePerIteration = 0.0f,
           ActivationFunction func = ActivationFunction::Linear,
           const std::string& sampleFilePath = "");
    
    void setAudioManager(AudioManager* manager);
    void setQuantizer(Quantizer* quantizer);
    float activate(float inputValue);
    void update(); // Update neuron state (apply activation_increase_per_iteration)
    void addExternalInput(float input); // Add external input (e.g., from rhythm interpreter)
    void playSample();
    void playSample(float offsetSeconds); // Play sample starting from the beginning with optional time offset
    void playSampleWithVolume(float volume); // Play sample with specific volume based on activation function
    
    // Getters
    float getActivation() const { return applyActivationFunction(activation); } // Processed output for neural network and visualization
    float getRawActivation() const { return activation; } // Raw activation level for debugging
    float getThreshold() const { return threshold; }
    float getDecayRate() const { return decayRate; }
    float getActivationIncreasePerIteration() const { return activationIncreasePerIteration; }
    float getExternalInput() const { return externalInput; }
    ActivationFunction getActivationFunction() const { return activationFunc; }
    bool getHasFired() const { return hasFired; }
    int getSampleIndex() const { return sampleIndex; }
    const std::string& getSampleFilePath() const { return sampleFilePath; }
    const std::vector<float>& getActivationHistory() const { return activationHistory; }
    
    // Setters
    void setActivation(float value) { activation = value; }
    void setThreshold(float value) { threshold = value; }
    void setDecayRate(float value) { decayRate = value; }
    void setActivationIncreasePerIteration(float value) { activationIncreasePerIteration = value; }
    void setActivationFunction(ActivationFunction func) { activationFunc = func; }
    void setSampleFilePath(const std::string& filePath) { sampleFilePath = filePath; }
    void setSampleIndex(int index) { sampleIndex = index; }
    void resetFiredFlag() { hasFired = false; }

private:
    float applyActivationFunction(float value) const;
};