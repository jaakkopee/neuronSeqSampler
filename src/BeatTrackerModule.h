#pragma once
#include "ModularComponent.h"
#include "BeatTracker.h"
#include <memory>

/**
 * @brief Modular wrapper for BeatTracker
 * 
 * Exposes beat tracking functionality with GUI controls for:
 * - Tempo detection and tracking
 * - Phase-based learning gain modulation
 * - Agent-based beat hypothesis testing
 * - Pattern detection
 */
class BeatTrackerModule : public ModularComponent {
private:
    std::unique_ptr<BeatTracker> beatTracker;
    std::vector<float> networkFirings;
    std::vector<float> inputOnsets;
    size_t sampleRate;
    size_t frameSize;
    
public:
    BeatTrackerModule(size_t sr = 44100, size_t fs = 512) 
        : ModularComponent("Beat Tracker", "BeatTracker"),
          sampleRate(sr),
          frameSize(fs) {
        
        // Define ports
        inputPorts.push_back(Port("Network Firings", PortType::CONTROL_IN, 0, true));
        inputPorts.push_back(Port("Input Onsets", PortType::AUDIO_IN, 1, true));
        outputPorts.push_back(Port("Beat Phase", PortType::CONTROL_OUT, 0, false));
        outputPorts.push_back(Port("Tempo", PortType::CONTROL_OUT, 1, false));
        outputPorts.push_back(Port("Learning Gain", PortType::CONTROL_OUT, 2, false));
        outputPorts.push_back(Port("Confidence", PortType::CONTROL_OUT, 3, false));
        
        // Define parameters matching BeatTracker API
        parameters.push_back(Parameter("Enabled", 1.0f, 0.0f, 1.0f, ""));
        parameters.push_back(Parameter("Global Tempo", 120.0f, 40.0f, 320.0f, "BPM"));
        parameters.push_back(Parameter("Beat Boost", 5.0f, 1.0f, 20.0f, "x"));
        parameters.push_back(Parameter("Phase Window", 0.1f, 0.01f, 0.5f, ""));
        parameters.push_back(Parameter("Boost Target", 0.0f, 0.0f, 2.0f, ""));  // 0=Learning, 1=Activation, 2=Weights
        
        moduleColor = sf::Color(200, 150, 100);
        size = sf::Vector2f(180, 140);
    }
    
    void initialize() override {
        if (!beatTracker) {
            beatTracker = std::make_unique<BeatTracker>(sampleRate, frameSize);
        }
    }
    
    void process(float deltaTime) override {
        if (!beatTracker) return;
        
        // Update enabled state
        auto* enabledParam = getParameter("Enabled");
        if (enabledParam) {
            beatTracker->setEnabled(enabledParam->value > 0.5f);
            enabled = enabledParam->value > 0.5f;
        }
        
        if (!enabled) return;
        
        // Update global tempo
        auto* tempoParam = getParameter("Global Tempo");
        if (tempoParam) {
            beatTracker->setGlobalTempo(tempoParam->value);
        }
        
        // Update beat boost
        auto* boostParam = getParameter("Beat Boost");
        if (boostParam) {
            beatTracker->setBeatBoost(boostParam->value);
        }
        
        // Update phase window
        auto* windowParam = getParameter("Phase Window");
        if (windowParam) {
            beatTracker->setPhaseWindow(windowParam->value);
        }
        
        // Update boost target
        auto* targetParam = getParameter("Boost Target");
        if (targetParam) {
            int targetValue = static_cast<int>(targetParam->value);
            if (targetValue == 0) {
                beatTracker->setBoostTarget(BoostTarget::Learning);
            } else if (targetValue == 1) {
                beatTracker->setBoostTarget(BoostTarget::Activation);
            } else {
                beatTracker->setBoostTarget(BoostTarget::ConnectionWeights);
            }
        }
        
        // Process if we have both network firings and input onsets
        if (!networkFirings.empty() && !inputOnsets.empty()) {
            beatTracker->update(networkFirings, inputOnsets);
        }
    }
    
    void sendOutput(int portIndex, const std::vector<float>& data) override {
        // Output data is retrieved via getters in the GUI
    }
    
    void receiveInput(int portIndex, const std::vector<float>& data) override {
        if (portIndex == 0) {
            // Network firings input
            networkFirings = data;
        } else if (portIndex == 1) {
            // Input onsets
            inputOnsets = data;
        }
    }
    
    // GUI-accessible getters
    float getCurrentPhase() const {
        return beatTracker ? beatTracker->getCurrentPhase() : 0.0f;
    }
    
    float getDetectedTempo() const {
        return beatTracker ? beatTracker->getDetectedTempo() : 120.0f;
    }
    
    float getPhaseBasedLearningGain() const {
        return beatTracker ? beatTracker->getPhaseBasedLearningGain() : 1.0f;
    }
    
    float getPhaseConfidence() const {
        return beatTracker ? beatTracker->getPhaseConfidence() : 0.0f;
    }
    
    size_t getAgentCount() const {
        return beatTracker ? beatTracker->getAgentCount() : 0;
    }
    
    Pattern getStrongestPattern() const {
        return beatTracker ? beatTracker->getStrongestPattern() : Pattern();
    }
    
    float getBeatBoost() const {
        return beatTracker ? beatTracker->getBeatBoost() : 1.0f;
    }
    
    float getPhaseWindow() const {
        return beatTracker ? beatTracker->getPhaseWindow() : 0.1f;
    }
    
    BoostTarget getBoostTarget() const {
        return beatTracker ? beatTracker->getBoostTarget() : BoostTarget::Learning;
    }
    
    void resetTracker() {
        if (beatTracker) {
            beatTracker->reset();
        }
    }
    
    BeatTracker* getBeatTracker() { return beatTracker.get(); }
};
