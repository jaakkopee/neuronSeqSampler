#pragma once
#include "ModularComponent.h"
#include "NeuronNetwork.h"
#include <memory>

/**
 * @brief Modular wrapper for NeuronNetwork
 */
class NeuronNetworkModule : public ModularComponent {
private:
    std::unique_ptr<NeuronNetwork> neuronNetwork;
    std::vector<float> rhythmInput;
    std::vector<float> controlInput;
    
public:
    NeuronNetworkModule() 
        : ModularComponent("Neuron Network", "NeuronNetwork") {
        
        // Define ports
        inputPorts.push_back(Port("Rhythm In", PortType::RHYTHM_IN, 0, true));
        inputPorts.push_back(Port("Control In", PortType::CONTROL_IN, 1, true));
        inputPorts.push_back(Port("Beat Phase In", PortType::CONTROL_IN, 2, true));
        outputPorts.push_back(Port("Neuron Out", PortType::NEURON_OUT, 0, false));
        outputPorts.push_back(Port("Trigger Out", PortType::CONTROL_OUT, 1, false));
        
        // Define parameters
        parameters.push_back(Parameter("Learning Rate", 0.01f, 0.0f, 1.0f, ""));
        parameters.push_back(Parameter("Weight Decay", 0.0005f, 0.0f, 0.01f, ""));
        parameters.push_back(Parameter("Max Weight", 1.0f, 0.1f, 10.0f, ""));
        parameters.push_back(Parameter("Mapping Gain", 0.2f, 0.0f, 2.0f, ""));
        parameters.push_back(Parameter("Onset Bias", 0.5f, 0.0f, 1.0f, ""));
        
        moduleColor = sf::Color(100, 200, 150);
        size = sf::Vector2f(160, 120);
    }
    
    void initialize() override {
        if (!neuronNetwork) {
            neuronNetwork = std::make_unique<NeuronNetwork>();
        }
    }
    
    void process(float deltaTime) override {
        if (!neuronNetwork || !enabled) return;
        
        // Update parameters
        auto* learningRateParam = getParameter("Learning Rate");
        if (learningRateParam) {
            neuronNetwork->setLearningRate(learningRateParam->value);
        }
        
        auto* weightDecayParam = getParameter("Weight Decay");
        if (weightDecayParam) {
            neuronNetwork->setWeightDecay(weightDecayParam->value);
        }
        
        auto* maxWeightParam = getParameter("Max Weight");
        if (maxWeightParam) {
            neuronNetwork->setMaxWeight(maxWeightParam->value);
        }
        
        auto* mappingGainParam = getParameter("Mapping Gain");
        if (mappingGainParam) {
            neuronNetwork->setMappingGain(mappingGainParam->value);
        }
        
        auto* onsetBiasParam = getParameter("Onset Bias");
        if (onsetBiasParam) {
            neuronNetwork->setOnsetBias(onsetBiasParam->value);
        }
        
        // Update network
        neuronNetwork->update(deltaTime);
    }
    
    void sendOutput(int portIndex, const std::vector<float>& data) override {
        // Handled by connections
    }
    
    void receiveInput(int portIndex, const std::vector<float>& data) override {
        if (portIndex == 0) {
            rhythmInput = data;
        } else if (portIndex == 1) {
            controlInput = data;
        }
    }
    
    void renderParameterWindow(sf::RenderWindow& window) override {
        // Custom parameter window for NeuronNetwork
    }
    
    NeuronNetwork* getNeuronNetwork() { return neuronNetwork.get(); }
};
