#pragma once
#include "ModularComponent.h"
#include "BeatTracker.h"
#include <memory>

/**
 * @brief Modular wrapper for BeatTracker
 */
class BeatTrackerModule : public ModularComponent {
private:
    std::unique_ptr<BeatTracker> beatTracker;
    std::vector<float> onsetInput;
    
public:
    BeatTrackerModule() 
        : ModularComponent("Beat Tracker", "BeatTracker") {
        
        // Define ports
        inputPorts.push_back(Port("Onset In", PortType::ONSET_IN, 0, true));
        inputPorts.push_back(Port("Audio In", PortType::AUDIO_IN, 1, true));
        outputPorts.push_back(Port("Beat Phase", PortType::CONTROL_OUT, 0, false));
        outputPorts.push_back(Port("Tempo", PortType::CONTROL_OUT, 1, false));
        outputPorts.push_back(Port("Beat Trigger", PortType::CONTROL_OUT, 2, false));
        
        // Define parameters
        parameters.push_back(Parameter("Min Tempo", 60.0f, 30.0f, 300.0f, "BPM"));
        parameters.push_back(Parameter("Max Tempo", 180.0f, 30.0f, 300.0f, "BPM"));
        parameters.push_back(Parameter("Agent Count", 5.0f, 1.0f, 20.0f, ""));
        parameters.push_back(Parameter("Adaptation Rate", 0.1f, 0.0f, 1.0f, ""));
        parameters.push_back(Parameter("Confidence Threshold", 0.5f, 0.0f, 1.0f, ""));
        
        moduleColor = sf::Color(200, 150, 100);
        size = sf::Vector2f(150, 120);
    }
    
    void initialize() override {
        if (!beatTracker) {
            beatTracker = std::make_unique<BeatTracker>(44100);
        }
    }
    
    void process(float deltaTime) override {
        if (!beatTracker || !enabled) return;
        
        // Update parameters
        auto* minTempoParam = getParameter("Min Tempo");
        auto* maxTempoParam = getParameter("Max Tempo");
        if (minTempoParam && maxTempoParam) {
            beatTracker->setTempoRange(minTempoParam->value, maxTempoParam->value);
        }
        
        auto* agentCountParam = getParameter("Agent Count");
        if (agentCountParam) {
            beatTracker->setAgentCount(static_cast<int>(agentCountParam->value));
        }
        
        auto* adaptRateParam = getParameter("Adaptation Rate");
        if (adaptRateParam) {
            beatTracker->setAdaptationRate(adaptRateParam->value);
        }
        
        // Process onset input
        if (!onsetInput.empty()) {
            for (float onset : onsetInput) {
                beatTracker->processOnset(onset);
            }
        }
        
        beatTracker->update();
    }
    
    void sendOutput(int portIndex, const std::vector<float>& data) override {
        // Handled by connections
    }
    
    void receiveInput(int portIndex, const std::vector<float>& data) override {
        if (portIndex == 0 || portIndex == 1) {
            onsetInput = data;
        }
    }
    
    void renderParameterWindow(sf::RenderWindow& window) override {
        // Custom parameter window for BeatTracker
    }
    
    BeatTracker* getBeatTracker() { return beatTracker.get(); }
};
