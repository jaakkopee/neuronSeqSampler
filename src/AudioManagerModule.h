#pragma once
#include "ModularComponent.h"
#include "AudioManager.h"
#include <memory>

/**
 * @brief Modular wrapper for AudioManager
 */
class AudioManagerModule : public ModularComponent {
private:
    std::unique_ptr<AudioManager> audioManager;
    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;
    
public:
    AudioManagerModule() 
        : ModularComponent("Audio Manager", "AudioManager") {
        
        // Define ports
        inputPorts.push_back(Port("Audio In", PortType::AUDIO_IN, 0, true));
        inputPorts.push_back(Port("Trigger", PortType::CONTROL_IN, 1, true));
        outputPorts.push_back(Port("Audio Out", PortType::AUDIO_OUT, 0, false));
        outputPorts.push_back(Port("Mix Out", PortType::AUDIO_OUT, 1, false));
        
        // Define parameters
        parameters.push_back(Parameter("Master Volume", 1.0f, 0.0f, 2.0f, ""));
        parameters.push_back(Parameter("Sample Rate", 44100.0f, 8000.0f, 96000.0f, "Hz"));
        
        moduleColor = sf::Color(150, 100, 200);
        size = sf::Vector2f(140, 100);
    }
    
    void initialize() override {
        if (!audioManager) {
            audioManager = std::make_unique<AudioManager>();
        }
    }
    
    void process(float deltaTime) override {
        if (!audioManager || !enabled) return;
        
        // Update master volume from parameters
        auto* volParam = getParameter("Master Volume");
        if (volParam) {
            audioManager->setMasterVolume(volParam->value);
        }
        
        // Process audio
        if (!inputBuffer.empty()) {
            // Handle input audio if connected
        }
    }
    
    void sendOutput(int portIndex, const std::vector<float>& data) override {
        // Outputs are handled by connections in GUI2
    }
    
    void receiveInput(int portIndex, const std::vector<float>& data) override {
        if (portIndex == 0) {
            inputBuffer = data;
        }
    }
    
    void renderParameterWindow(sf::RenderWindow& window) override {
        // Custom parameter window rendering for AudioManager
        // This would typically use ImGui or custom SFML widgets
    }
    
    AudioManager* getAudioManager() { return audioManager.get(); }
};
