#include "NeuronNetwork.h"
#include "AudioManager.h"
#include <algorithm>

NeuronNetwork::NeuronNetwork() 
    : audioManager(nullptr)
{
}

void NeuronNetwork::setAudioManager(AudioManager* manager) {
    audioManager = manager;
    // Update all existing neurons
    for (auto& neuron : neurons) {
        neuron->setAudioManager(manager);
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
    
    // Update all neurons first (apply activation_increase_per_iteration)
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
}