#pragma once
#include "ModularComponent.h"
#include "RhythmInterpreter.h"
#include <memory>

/**
 * @brief Modular wrapper for RhythmInterpreter (Rhythmogram)
 */
class RhythmogramModule : public ModularComponent {
private:
    std::unique_ptr<RhythmInterpreter> rhythmInterpreter;
    std::vector<float> audioInput;
    
public:
    RhythmogramModule() 
        : ModularComponent("Rhythmogram", "RhythmInterpreter") {
        
        // Define ports
        inputPorts.push_back(Port("Audio In", PortType::AUDIO_IN, 0, true));
        outputPorts.push_back(Port("Rhythm Out", PortType::RHYTHM_OUT, 0, false));
        outputPorts.push_back(Port("Onset Out", PortType::ONSET_OUT, 1, false));
        outputPorts.push_back(Port("Band 0-7", PortType::CONTROL_OUT, 2, false));
        
        // Define parameters
        parameters.push_back(Parameter("Sample Rate", 44100.0f, 8000.0f, 96000.0f, "Hz"));
        parameters.push_back(Parameter("Buffer Size", 512.0f, 128.0f, 4096.0f, "samples"));
        parameters.push_back(Parameter("Onset Threshold", 0.3f, 0.0f, 1.0f, ""));
        parameters.push_back(Parameter("Smoothing", 0.9f, 0.0f, 1.0f, ""));
        
        moduleColor = sf::Color(150, 200, 150);
        size = sf::Vector2f(140, 100);
    }
    
    void initialize() override {
        if (!rhythmInterpreter) {
            auto* sampleRateParam = getParameter("Sample Rate");
            auto* bufferSizeParam = getParameter("Buffer Size");
            size_t sr = sampleRateParam ? static_cast<size_t>(sampleRateParam->value) : 44100;
            size_t bs = bufferSizeParam ? static_cast<size_t>(bufferSizeParam->value) : 512;
            rhythmInterpreter = std::make_unique<RhythmInterpreter>(sr, bs);
        }
    }
    
    void process(float deltaTime) override {
        if (!rhythmInterpreter || !enabled) return;
        
        // Process audio input through rhythmogram
        if (!audioInput.empty()) {
            rhythmInterpreter->processAudio(audioInput.data(), audioInput.size());
        }
    }
    
    void sendOutput(int portIndex, const std::vector<float>& data) override {
        // Handled by connections
    }
    
    void receiveInput(int portIndex, const std::vector<float>& data) override {
        if (portIndex == 0) {
            audioInput = data;
        }
    }
    
    void renderParameterWindow(sf::RenderWindow& window) override {
        // Custom parameter window for Rhythmogram
    }
    
    RhythmInterpreter* getRhythmInterpreter() { return rhythmInterpreter.get(); }
};
