#pragma once
#include <vector>
#include <cstddef>

class RhythmInterpreter {
public:
    RhythmInterpreter(size_t sampleRate, size_t bufferSize);
    void processAudioFrame(const std::vector<float>& audioData);
    std::vector<float> getFilterOutputs() const;
    void setSensitivity(size_t bandIndex, float gain);
    float getSensitivity(size_t bandIndex) const;
    void setFilterGain(size_t bandIndex, float gain);
    float getFilterGain(size_t bandIndex) const;

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

    void initializeBands();
    std::vector<float> bandpassFilter(const std::vector<float>& data, float freq, float bw);
};
