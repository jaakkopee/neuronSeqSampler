#pragma once
#include "ModularComponent.h"
#include "Recorder.h"
#include <memory>

/**
 * @brief Modular wrapper for Recorder
 */
class RecorderModule : public ModularComponent {
private:
    std::unique_ptr<Recorder> recorder;
    std::vector<float> audioInput;
    bool isRecording;
    
public:
    RecorderModule() 
        : ModularComponent("Recorder", "Recorder"), isRecording(false) {
        
        // Define ports
        inputPorts.push_back(Port("Audio In", PortType::AUDIO_IN, 0, true));
        inputPorts.push_back(Port("Record Trigger", PortType::CONTROL_IN, 1, true));
        outputPorts.push_back(Port("Audio Out", PortType::AUDIO_OUT, 0, false));
        outputPorts.push_back(Port("Recording Status", PortType::CONTROL_OUT, 1, false));
        
        // Define parameters
        parameters.push_back(Parameter("Sample Rate", 44100.0f, 8000.0f, 96000.0f, "Hz"));
        parameters.push_back(Parameter("Noise Gate Threshold", 0.01f, 0.0f, 1.0f, ""));
        parameters.push_back(Parameter("High Pass Frequency", 80.0f, 20.0f, 500.0f, "Hz"));
        
        moduleColor = sf::Color(200, 100, 100);
        size = sf::Vector2f(140, 100);
    }
    
    void initialize() override {
        if (!recorder) {
            recorder = std::make_unique<Recorder>();
        }
    }
    
    void process(float deltaTime) override {
        if (!recorder || !enabled) return;
        
        // Update parameters
        auto* noiseGateParam = getParameter("Noise Gate Threshold");
        if (noiseGateParam) {
            recorder->setNoiseGateThreshold(noiseGateParam->value);
        }
        
        auto* hpfParam = getParameter("High Pass Frequency");
        if (hpfParam) {
            recorder->setHighPassFrequency(hpfParam->value);
        }
    }
    
    void sendOutput(int portIndex, const std::vector<float>& data) override {
        // Handled by connections
    }
    
    void receiveInput(int portIndex, const std::vector<float>& data) override {
        if (portIndex == 0) {
            audioInput = data;
        } else if (portIndex == 1 && !data.empty()) {
            // Trigger recording on/off
            if (data[0] > 0.5f && !isRecording) {
                startRecording();
            } else if (data[0] <= 0.5f && isRecording) {
                stopRecording();
            }
        }
    }
    
    void renderParameterWindow(sf::RenderWindow& window) override {
        // Custom parameter window for Recorder
    }
    
    void startRecording() {
        if (recorder) {
            recorder->start();
            isRecording = true;
        }
    }
    
    void stopRecording() {
        if (recorder) {
            recorder->stop();
            isRecording = false;
        }
    }
    
    Recorder* getRecorder() { return recorder.get(); }
};
