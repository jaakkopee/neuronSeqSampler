#include "Neuron.h"
#include "AudioManager.h"
#include "Quantizer.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <chrono>

Neuron::Neuron(int sampleIndex, float initialActivation, float threshold, 
               float decayRate, float activationIncreasePerIteration, float leakRateParam, ActivationFunction func,
               const std::string& sampleFilePath)
    : sampleIndex(sampleIndex)
    , activation(initialActivation)
    , threshold(threshold)
    , decayRate(decayRate)
    , leakRate(leakRateParam)
    , activationIncreasePerIteration(activationIncreasePerIteration)
    , externalInput(0.0f)
    , sampleFilePath(sampleFilePath)
    , activationFunc(func)
    , hasFired(false)
    , audioManager(nullptr)
    , quantizer(nullptr)
{
    activationHistory.reserve(maxHistoryLength);
    activationHistory.push_back(activation);
}

// Backward-compatible constructor delegating with default leakRate
Neuron::Neuron(int sampleIndex, float initialActivation, float threshold,
               float decayRate, float activationIncreasePerIteration,
               ActivationFunction func, const std::string& sampleFilePath)
    : Neuron(sampleIndex, initialActivation, threshold, decayRate,
             activationIncreasePerIteration, 0.05f, func, sampleFilePath) {}

void Neuron::setAudioManager(AudioManager* manager) {
    audioManager = manager;
}

void Neuron::setQuantizer(Quantizer* quantizerPtr) {
    quantizer = quantizerPtr;
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
        activation += decayRate;
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

    // Apply continuous leak towards zero to avoid saturation
    // Exponential decay model: reduces both positive and negative activation towards 0
    if (leakRate > 0.0f) {
        activation -= activation * leakRate;
    }
    
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
    if (!audioManager) {
        return;
    }
    
    // Check if quantization is enabled and available
    if (quantizer && quantizer->isQuantizationEnabled()) {
        // Get current time for quantization
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = now.time_since_epoch();
        double currentTime = std::chrono::duration<double>(duration).count();
        
        // Create a quantizer event for this sample trigger
        Quantizer::Event event(currentTime, sampleIndex, 1.0f);
        
        // Quantize the event
        if (quantizer->quantizeEvent(event)) {
            // If quantization occurred, calculate the delay
            double delay = event.timestamp - currentTime;
            
            // Only play if we're very close to a grid point (within 10ms tolerance)
            if (delay <= 0.01) {  // 10ms tolerance
                std::cout << "🎵 Quantized playback: on-grid (delay " << delay << "s) for sample " << sampleIndex << std::endl;
                audioManager->playSample(sampleIndex);
            } else {
                std::cout << "🎵 Quantized suppression: off-grid (delay " << delay << "s) for sample " << sampleIndex << std::endl;
                // Don't play - we're too far from a grid point
                return;
            }
        } else {
            // No quantization needed, play immediately
            audioManager->playSample(sampleIndex);
        }
    } else {
        // No quantization, play immediately
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