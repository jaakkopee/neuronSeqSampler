#include "Neuron.h"
#include "AudioManager.h"
#include <iostream>
#include <cmath>
#include <algorithm>

Neuron::Neuron(int sampleIndex, float initialActivation, float threshold, 
               float decayRate, float activationIncreasePerIteration, ActivationFunction func)
    : sampleIndex(sampleIndex)
    , activation(initialActivation)
    , threshold(threshold)
    , decayRate(decayRate)
    , activationIncreasePerIteration(activationIncreasePerIteration)
    , externalInput(0.0f)
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
    // Apply input value and external input (per-iteration increase is handled in update())
    activation += inputValue + externalInput;
    
    // Reset external input after use (it accumulates between calls)
    externalInput = 0.0f;
    
    // Use processed activation for threshold comparisons
    float processedActivation = applyActivationFunction(activation);
    
    if (processedActivation > threshold) {
        playSample();
        hasFired = true;
        activation -= decayRate;
    }
    
    if (processedActivation < -threshold) {
        activation += threshold;
    }
    
    // Maintain activation history (store raw activation for visualization)
    if (activationHistory.size() >= maxHistoryLength) {
        activationHistory.erase(activationHistory.begin());
    }
    activationHistory.push_back(activation);
    
    // Return processed activation value for neural network use
    return processedActivation;
}

void Neuron::update() {
    // Apply per-iteration activation increase/decrease
    activation += activationIncreasePerIteration;
    
    // Process accumulated external input (e.g., from rhythmogram)
    activation += externalInput;
    externalInput = 0.0f; // Reset external input after use
    
    // Use processed activation for threshold comparisons to affect self-oscillating behavior
    float processedActivation = applyActivationFunction(activation);
    
    if (processedActivation > threshold) {
        playSample();
        hasFired = true;
        activation -= decayRate;
    }
    
    if (processedActivation < -threshold) {
        activation += threshold;
    }
    
    // Maintain activation history (store raw activation for visualization)
    if (activationHistory.size() >= maxHistoryLength) {
        activationHistory.erase(activationHistory.begin());
    }
    activationHistory.push_back(activation);
}

void Neuron::addExternalInput(float input) {
    // Accumulate external input - will be used in next activate() call
    externalInput += input;
}

void Neuron::playSample() {
    if (audioManager) {
        audioManager->playSample(sampleIndex);
    }
}

void Neuron::playSample(float offsetSeconds) {
    if (audioManager) {
        // Always start from the beginning (0.0), but allow for time-based offset
        audioManager->playSample(sampleIndex, 0.0f);
    }
}

void Neuron::playSampleWithVolume(float volume) {
    if (audioManager) {
        audioManager->playSample(sampleIndex, 0.0f, volume);
    }
}

float Neuron::applyActivationFunction(float value) const {
    switch (activationFunc) {
        case ActivationFunction::Sigmoid:
            // Scale sigmoid to match linear range: sigmoid * 2 - 1 maps [0,1] to [-1,1], then scale by input magnitude
            return (2.0f / (1.0f + std::exp(-value)) - 1.0f) * std::abs(value) * 2.0f;
        case ActivationFunction::ReLU:
            return std::max(0.0f, value);
        case ActivationFunction::Tanh:
            // Scale tanh output to match input magnitude while preserving shape
            return std::tanh(value) * std::abs(value) * 2.0f;
        case ActivationFunction::Linear:
        default:
            return value;
    }
}