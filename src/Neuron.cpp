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

void Neuron::update() {
    // Apply per-iteration activation increase/decrease
    activation += activationIncreasePerIteration;
    
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