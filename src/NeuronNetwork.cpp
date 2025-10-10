#include "NeuronNetwork.h"
#include "AudioManager.h"
#include "RhythmInterpreter.h"
#include <algorithm>
#include <cmath>
#include <iostream>

NeuronNetwork::NeuronNetwork() 
    : audioManager(nullptr), rhythmInterpreter(nullptr)
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

Neuron* NeuronNetwork::addNeuron(int sampleIndex, float initialActivation, 
                                 float threshold, float decayRate, 
                                 float activationIncreasePerIteration,
                                 ActivationFunction func) {
    auto neuron = std::make_unique<Neuron>(sampleIndex, initialActivation, 
                                          threshold, decayRate, activationIncreasePerIteration, func);
    if (audioManager) {
        neuron->setAudioManager(audioManager);
    }
    
    Neuron* rawPtr = neuron.get();
    neurons.push_back(std::move(neuron));
    
    // Update rhythm interpreter connection matrix size
    if (rhythmInterpreter) {
        rhythmInterpreter->updateNetworkSize();
    }
    
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
    
    // Update rhythm interpreter first (processes audio and sends inputs to neurons)
    if (rhythmInterpreter) {
        rhythmInterpreter->update();
    }
    
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
    
    // Update rhythm interpreter connection matrix size
    if (rhythmInterpreter) {
        rhythmInterpreter->updateNetworkSize();
    }
    
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
        rhythmInterpreter = new RhythmInterpreter(this, audioManager);
        std::cout << "🎵 RhythmInterpreter initialized with filter bank" << std::endl;
        
        // Set up some default connections for testing if there are neurons
        if (!neurons.empty()) {
            // Connect each filter to neurons in a round-robin fashion
            size_t numFilters = rhythmInterpreter->getNumFilters();
            for (size_t f = 0; f < numFilters; ++f) {
                for (size_t n = 0; n < neurons.size(); ++n) {
                    // Create interesting patterns: different filters affect different neurons
                    float weight = 0.1f * std::sin(f + n); // Create varied connection strengths
                    rhythmInterpreter->setConnectionWeight(f, n, weight);
                }
            }
            std::cout << "🔗 Filter-neuron connections established" << std::endl;
        }
    }
}

void NeuronNetwork::processAudioForRhythm(const std::vector<float>& audioData) {
    if (rhythmInterpreter) {
        rhythmInterpreter->processAudioFrame(audioData);
        
        // Apply rhythm interpreter outputs to neurons
        auto neuronInputs = rhythmInterpreter->getCurrentNeuronInputs();
        static int debugCounter = 0;
        bool hasSignificantInput = false;
        
        for (size_t i = 0; i < std::min(neuronInputs.size(), neurons.size()); ++i) {
            if (std::abs(neuronInputs[i]) > 0.001f) { // Only apply significant inputs
                neurons[i]->addExternalInput(neuronInputs[i]);
                hasSignificantInput = true;
            }
        }
        
        // Debug output every 100 frames (~2.3 seconds at 44.1kHz with 512 sample buffers)
        if (++debugCounter % 100 == 0 && hasSignificantInput) {
            float tempo = rhythmInterpreter->getCurrentTempo();
            float rhythmStrength = rhythmInterpreter->getOverallRhythmStrength();
            std::cout << "🎵 Rhythm: Tempo=" << tempo << " BPM, Strength=" << rhythmStrength 
                      << ", Inputs: " << neuronInputs.size() << std::endl;
        }
    }
}

std::vector<float> NeuronNetwork::getProcessedAudioOutput() const {
    if (rhythmInterpreter) {
        return rhythmInterpreter->getProcessedAudioOutput();
    }
    return std::vector<float>(); // Return empty vector if no rhythm interpreter
}