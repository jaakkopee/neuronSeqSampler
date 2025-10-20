#pragma once
#include <vector>
#include <cstddef>

class RhythmInterpreter {
public:
    RhythmInterpreter(size_t sampleRate, size_t bufferSize);
    void processAudioFrame(const std::vector<float>& audioData);
    std::vector<float> getFilterOutputs() const;
    void setSensitivity(size_t bandIndex, float gain, float sigmoidGain);
    float getSensitivity(size_t bandIndex) const;
    void setFilterGain(size_t bandIndex, float gain);
    float getFilterGain(size_t bandIndex) const;
    
    // Additional parameter getters/setters for GUI control
    void setBandFrequency(size_t bandIndex, float frequency);
    float getBandFrequency(size_t bandIndex) const;
    void setBandBandwidth(size_t bandIndex, float bandwidth);
    float getBandBandwidth(size_t bandIndex) const;
    void setBandScaling(size_t bandIndex, float scaling);
    float getBandScaling(size_t bandIndex) const;
    void setBandLimit(size_t bandIndex, float limit);
    float getBandLimit(size_t bandIndex) const;
    void setQValue(size_t bandIndex, float q);
    float getQValue(size_t bandIndex) const;
    
    // Adaptive sensitivity system getters
    float getAdaptiveSensitivity(size_t bandIndex) const;
    float getEnergyBaseline(size_t bandIndex) const;
    float getEnergyPeak(size_t bandIndex) const;
    float getNoiseFloor(size_t bandIndex) const;
    float getDynamicRange(size_t bandIndex) const;

private:
    size_t sampleRate;
    size_t bufferSize;
    size_t bandCount;
    int frameCounter;
    std::vector<float> bandFrequencies;
    std::vector<float> bandBandwidths;
    std::vector<float> bandScalings;
    std::vector<float> bandLimits;
    std::vector<float> bandGains;     // Sensitivity gains
    std::vector<float> filterGains;   // Output filter gains
    std::vector<float> qValues;
    std::vector<float> filterOutputs;
    std::vector<int> stuckCounters;
    
    // Adaptive sensitivity system
    std::vector<float> adaptiveSensitivities;  // Current adaptive sensitivity for each band
    std::vector<float> energyBaselines;        // Running average of quiet periods
    std::vector<float> energyPeaks;            // Running maximum of active periods  
    std::vector<float> noiseFloors;            // Estimated noise floor for each band
    std::vector<float> dynamicRanges;          // Current dynamic range estimates
    std::vector<int> adaptationCounters;       // Frames since last adaptation update

    void initializeBands();
    void updateAdaptiveSensitivity(size_t bandIndex, float rawEnergy);
    float applyContrastEnhancement(size_t bandIndex, float energy);
    std::vector<float> bandpassFilter(const std::vector<float>& data, float freq, float bw);
    std::vector<float> zeroCrossingOnsetDetection(const std::vector<float>& data, int bandIndex);
    std::vector<float> envelopeDetection(const std::vector<float>& data, int bandIndex);
};
