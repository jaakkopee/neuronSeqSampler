#pragma once
#include "ModularComponent.h"
#include "Quantizer.h"
#include <memory>

/**
 * @brief Modular wrapper for Quantizer
 */
class QuantizerModule : public ModularComponent {
private:
    std::unique_ptr<Quantizer> quantizer;
    std::vector<float> eventInput;
    
public:
    QuantizerModule() 
        : ModularComponent("Quantizer", "Quantizer") {
        
        // Define ports
        inputPorts.push_back(Port("Event In", PortType::CONTROL_IN, 0, true));
        inputPorts.push_back(Port("Beat Phase In", PortType::CONTROL_IN, 1, true));
        outputPorts.push_back(Port("Quantized Out", PortType::CONTROL_OUT, 0, false));
        outputPorts.push_back(Port("Timing Out", PortType::CONTROL_OUT, 1, false));
        
        // Define parameters
        parameters.push_back(Parameter("BPM", 120.0f, 30.0f, 300.0f, "BPM"));
        parameters.push_back(Parameter("Grid Resolution", 3.0f, 0.0f, 5.0f, "")); // 0-5 for enum
        parameters.push_back(Parameter("Quantize Strength", 1.0f, 0.0f, 1.0f, ""));
        parameters.push_back(Parameter("Swing", 0.5f, 0.0f, 1.0f, ""));
        parameters.push_back(Parameter("Lookahead", 100.0f, 0.0f, 500.0f, "ms"));
        
        moduleColor = sf::Color(100, 150, 200);
        size = sf::Vector2f(140, 110);
    }
    
    void initialize() override {
        if (!quantizer) {
            auto* bpmParam = getParameter("BPM");
            float bpm = bpmParam ? bpmParam->value : 120.0f;
            quantizer = std::make_unique<Quantizer>(bpm, 44100);
        }
    }
    
    void process(float deltaTime) override {
        if (!quantizer || !enabled) return;
        
        // Update parameters
        auto* bpmParam = getParameter("BPM");
        if (bpmParam) {
            quantizer->setTempo(bpmParam->value);
        }
        
        auto* gridParam = getParameter("Grid Resolution");
        if (gridParam) {
            int gridValue = static_cast<int>(gridParam->value);
            quantizer->setGridResolution(static_cast<Quantizer::GridResolution>(gridValue));
        }
        
        auto* strengthParam = getParameter("Quantize Strength");
        if (strengthParam) {
            quantizer->setQuantizationStrength(strengthParam->value);
        }
        
        auto* swingParam = getParameter("Swing");
        if (swingParam) {
            quantizer->setSwing(swingParam->value);
        }
        
        auto* lookaheadParam = getParameter("Lookahead");
        if (lookaheadParam) {
            quantizer->setLookahead(lookaheadParam->value);
        }
    }
    
    void sendOutput(int portIndex, const std::vector<float>& data) override {
        // Handled by connections
    }
    
    void receiveInput(int portIndex, const std::vector<float>& data) override {
        if (portIndex == 0) {
            eventInput = data;
        }
    }
    
    void renderParameterWindow(sf::RenderWindow& window) override {
        // Custom parameter window for Quantizer
    }
    
    Quantizer* getQuantizer() { return quantizer.get(); }
};
