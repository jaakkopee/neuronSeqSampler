#include "NeuronNetwork.h"
#include "AudioManager.h"
#include "RhythmInterpreter.h"
#include "BeatRoot.h"
#include <algorithm>
#include <cmath>
#include <iostream>

NeuronNetwork::NeuronNetwork() 
    : audioManager(nullptr), rhythmInterpreter(nullptr), quantizer(nullptr)
{
}

NeuronNetwork::~NeuronNetwork() {
    delete rhythmInterpreter;
    rhythmInterpreter = nullptr;
}

void NeuronNetwork::setAudioManager(AudioManager* manager) {
    audioManager = manager;
    // Update all existing neurons
    for (auto& neuron : neurons) {
        neuron->setAudioManager(manager);
    }
    
    // Initialize rhythm interpreter if audio manager is available
    if (manager && !rhythmInterpreter) {
        initializeRhythmInterpreter();
    }
}

void NeuronNetwork::setQuantizer(Quantizer* quantizerPtr) {
    quantizer = quantizerPtr;
    // Update all existing neurons
    for (auto& neuron : neurons) {
        neuron->setQuantizer(quantizerPtr);
    }
}

Neuron* NeuronNetwork::addNeuron(int sampleIndex, float initialActivation, 
                                 float threshold, float decayRate, 
                                 float activationIncreasePerIteration,
                                 ActivationFunction func) {
    auto neuron = std::make_unique<Neuron>(sampleIndex, initialActivation, 
                                          threshold, decayRate, activationIncreasePerIteration, func);
    if (audioManager) {
        neuron->setAudioManager(audioManager);
    }
    if (quantizer) {
        neuron->setQuantizer(quantizer);
    }
    
    Neuron* rawPtr = neuron.get();
    neurons.push_back(std::move(neuron));
    
    // updateNetworkSize removed: minimal RhythmInterpreter does not support this method
    
    return rawPtr;
}

Connection* NeuronNetwork::connect(Neuron* source, Neuron* target, float weight) {
    auto connection = std::make_unique<Connection>(source, target, weight);
    Connection* rawPtr = connection.get();
    connections.push_back(std::move(connection));
    return rawPtr;
}

void NeuronNetwork::activate() {
    // Reset all fired flags first
    resetFiredFlags();
    
    // update removed: minimal RhythmInterpreter does not support this method
    
    // Update all neurons (apply activation_increase_per_iteration)
    for (auto& neuron : neurons) {
        neuron->update();
    }
    
    // Then activate all connections
    for (auto& connection : connections) {
        connection->activate();
    }
}

void NeuronNetwork::resetFiredFlags() {
    for (auto& neuron : neurons) {
        neuron->resetFiredFlag();
    }
}

bool NeuronNetwork::removeNeuron(size_t index) {
    if (index >= neurons.size()) {
        return false;
    }
    
    Neuron* neuronToRemove = neurons[index].get();
    
    // Remove all connections that involve this neuron
    connections.erase(
        std::remove_if(connections.begin(), connections.end(),
            [neuronToRemove](const std::unique_ptr<Connection>& conn) {
                return conn->getSource() == neuronToRemove || conn->getTarget() == neuronToRemove;
            }),
        connections.end()
    );
    
    // Remove the neuron itself
    neurons.erase(neurons.begin() + index);
    
    // updateNetworkSize removed: minimal RhythmInterpreter does not support this method
    
    return true;
}

bool NeuronNetwork::removeConnection(size_t index) {
    if (index >= connections.size()) {
        return false;
    }
    
    connections.erase(connections.begin() + index);
    return true;
}

void NeuronNetwork::clearNetwork() {
    connections.clear();
    neurons.clear();
    delete rhythmInterpreter; // Also clear rhythm interpreter
    rhythmInterpreter = nullptr;
}

void NeuronNetwork::initializeRhythmInterpreter() {
    if (audioManager && !rhythmInterpreter) {
        // Use default sampleRate and bufferSize for minimal RhythmInterpreter
        size_t sampleRate = 44100;
        size_t bufferSize = 512;
        rhythmInterpreter = new RhythmInterpreter(sampleRate, bufferSize);
        std::cout << "🎵 Minimal RhythmInterpreter initialized" << std::endl;
    }
}

void NeuronNetwork::processAudioForRhythm(const std::vector<float>& audioData) {
    if (rhythmInterpreter) {
        rhythmInterpreter->processAudioFrame(audioData);
        // Apply rhythm filter outputs to connected neurons
        applyRhythmConnections();
    }
}

std::vector<float> NeuronNetwork::getProcessedAudioOutput() const {
    // Minimal RhythmInterpreter: getProcessedAudioOutput not supported
    return std::vector<float>(); // Return empty vector
}

// Rhythm-to-neuron connection matrix methods
void NeuronNetwork::setRhythmConnection(size_t filterIndex, size_t neuronIndex, float weight) {
    // Ensure matrix is large enough
    if (rhythmConnectionMatrix.size() <= filterIndex) {
        rhythmConnectionMatrix.resize(filterIndex + 1);
    }
    if (rhythmConnectionMatrix[filterIndex].size() <= neuronIndex) {
        rhythmConnectionMatrix[filterIndex].resize(neuronIndex + 1, 0.0f);
    }
    
    rhythmConnectionMatrix[filterIndex][neuronIndex] = weight;
}

float NeuronNetwork::getRhythmConnection(size_t filterIndex, size_t neuronIndex) const {
    if (filterIndex < rhythmConnectionMatrix.size() && 
        neuronIndex < rhythmConnectionMatrix[filterIndex].size()) {
        return rhythmConnectionMatrix[filterIndex][neuronIndex];
    }
    return 0.0f;
}

void NeuronNetwork::clearRhythmConnection(size_t filterIndex, size_t neuronIndex) {
    setRhythmConnection(filterIndex, neuronIndex, 0.0f);
}

void NeuronNetwork::applyRhythmConnections() {
    if (!rhythmInterpreter) return;
    
    // Get current filter outputs
    std::vector<float> filterOutputs = rhythmInterpreter->getFilterOutputs();
    
    // Apply connections to neurons
    for (size_t f = 0; f < filterOutputs.size() && f < rhythmConnectionMatrix.size(); ++f) {
        for (size_t n = 0; n < rhythmConnectionMatrix[f].size() && n < neurons.size(); ++n) {
            float connectionWeight = rhythmConnectionMatrix[f][n];
            if (std::abs(connectionWeight) > 0.001f) {
                // Apply rhythm filter output to neuron activation
                float rhythmInput = filterOutputs[f] * connectionWeight;
                neurons[n]->addExternalInput(rhythmInput);
            }
        }
    }
}