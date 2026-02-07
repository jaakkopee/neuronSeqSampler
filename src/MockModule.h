#pragma once
#include "ModularComponent.h"

/**
 * @brief Mock module for GUI demonstration
 * 
 * This is a placeholder module that demonstrates the GUI structure
 * without requiring full implementation of the complex audio components.
 */
class MockModule : public ModularComponent {
private:
    std::string moduleTypeName;
    
public:
    MockModule(const std::string& name, const std::string& type, const sf::Color& color) 
        : ModularComponent(name, type), moduleTypeName(type) {
        
        moduleColor = color;
        
        // Add some default ports based on type
        if (type == "AudioManager") {
            inputPorts.push_back(Port("Audio In", PortType::AUDIO_IN, 0, true));
            inputPorts.push_back(Port("Trigger", PortType::CONTROL_IN, 1, true));
            outputPorts.push_back(Port("Audio Out", PortType::AUDIO_OUT, 0, false));
            outputPorts.push_back(Port("Mix Out", PortType::AUDIO_OUT, 1, false));
            parameters.push_back(Parameter("Master Volume", 1.0f, 0.0f, 2.0f, ""));
        } 
        else if (type == "Recorder") {
            inputPorts.push_back(Port("Audio In", PortType::AUDIO_IN, 0, true));
            outputPorts.push_back(Port("Audio Out", PortType::AUDIO_OUT, 0, false));
            parameters.push_back(Parameter("Gain", 1.0f, 0.0f, 2.0f, "dB"));
        }
        else if (type == "NeuronNetwork") {
            inputPorts.push_back(Port("Rhythm In", PortType::RHYTHM_IN, 0, true));
            inputPorts.push_back(Port("Control In", PortType::CONTROL_IN, 1, true));
            outputPorts.push_back(Port("Neuron Out", PortType::NEURON_OUT, 0, false));
            parameters.push_back(Parameter("Learning Rate", 0.01f, 0.0f, 1.0f, ""));
        }
        else if (type == "BeatTracker") {
            inputPorts.push_back(Port("Onset In", PortType::ONSET_IN, 0, true));
            outputPorts.push_back(Port("Beat Phase", PortType::CONTROL_OUT, 0, false));
            outputPorts.push_back(Port("Tempo", PortType::CONTROL_OUT, 1, false));
            parameters.push_back(Parameter("BPM", 120.0f, 60.0f, 180.0f, "BPM"));
        }
        else if (type == "Rhythmogram") {
            inputPorts.push_back(Port("Audio In", PortType::AUDIO_IN, 0, true));
            outputPorts.push_back(Port("Rhythm Out", PortType::RHYTHM_OUT, 0, false));
            outputPorts.push_back(Port("Onset Out", PortType::ONSET_OUT, 1, false));
            parameters.push_back(Parameter("Threshold", 0.3f, 0.0f, 1.0f, ""));
        }
        else if (type == "Quantizer") {
            inputPorts.push_back(Port("Event In", PortType::CONTROL_IN, 0, true));
            outputPorts.push_back(Port("Quantized Out", PortType::CONTROL_OUT, 0, false));
            parameters.push_back(Parameter("Grid Size", 16.0f, 4.0f, 64.0f, ""));
        }
    }
    
    void process(float deltaTime) override {
        // Mock processing - does nothing for demo
        (void)deltaTime; // Suppress unused parameter warning
    }
    
    void sendOutput(int portIndex, const std::vector<float>& data) override {
        // Mock - does nothing for demo
        (void)portIndex;
        (void)data;
    }
    
    void receiveInput(int portIndex, const std::vector<float>& data) override {
        // Mock - does nothing for demo
        (void)portIndex;
        (void)data;
    }
    
    void renderParameterWindow(sf::RenderWindow& window) override {
        // Mock - basic rendering handled by GUI2
        (void)window;
    }
};
