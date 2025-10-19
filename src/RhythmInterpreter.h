#pragma once
#include <vector>
#include <cstddef>

class RhythmInterpreter {
public:
    RhythmInterpreter(size_t sampleRate, size_t bufferSize);
    void processAudioFrame(const std::vector<float>& audioData);
    std::vector<float> getFilterOutputs() const;

private:
    size_t sampleRate;
    size_t bufferSize;
    size_t bandCount;
    int frameCounter;
    std::vector<float> bandFrequencies;
    std::vector<float> bandBandwidths;
    std::vector<float> bandScalings;
    std::vector<float> bandLimits;
    std::vector<float> bandGains;
    std::vector<float> filterOutputs;
    std::vector<int> stuckCounters;

    void initializeBands();
    float bandpassFilter(const std::vector<float>& data, float freq, float bw);
};
