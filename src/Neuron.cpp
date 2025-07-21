#include "Neuron.h"
#include "AudioManager.h"
#include <cmath>
#include <algorithm>

Neuron::Neuron(int sampleIndex, float initialActivation, float threshold, 
               float decayRate, ActivationFunction func)
    : sampleIndex(sampleIndex)
    , activation(initialActivation)
    , threshold(threshold)
    , decayRate(decayRate)
    , activationFunc(func)
    , hasFired(false)
    , audioManager(nullptr)
{
    activationHistory.reserve(maxHistoryLength);
    activationHistory.push_back(activation);
}

void Neuron::setAudioManager(AudioManager* manager) {
    audioManager = manager;
}

float Neuron::activate(float inputValue) {
    activation += inputValue;
    
    if (activation > threshold) {
        playSample();
        hasFired = true;
        activation -= decayRate;
    }
    
    if (activation < -threshold) {
        activation += threshold;
    }
    
    // Maintain activation history
    if (activationHistory.size() >= maxHistoryLength) {
        activationHistory.erase(activationHistory.begin());
    }
    activationHistory.push_back(activation);
    
    // Apply activation function and return
    return applyActivationFunction(activation);
}

void Neuron::playSample() {
    if (audioManager) {
        audioManager->playSample(sampleIndex);
    }
}

float Neuron::applyActivationFunction(float value) {
    switch (activationFunc) {
        case ActivationFunction::Sigmoid:
            return 1.0f / (1.0f + std::exp(-value));
        case ActivationFunction::ReLU:
            return std::max(0.0f, value);
        case ActivationFunction::Tanh:
            return std::tanh(value);
        case ActivationFunction::Linear:
        default:
            return value;
    }
}