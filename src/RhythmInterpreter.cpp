#include "RhythmInterpreter.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>

RhythmInterpreter::RhythmInterpreter(size_t sampleRate, size_t bufferSize)
    : sampleRate(sampleRate), bufferSize(bufferSize), bandCount(8), frameCounter(0) {
    initializeBands();
    filterOutputs.resize(bandCount, 0.0f);
    bandGains.resize(bandCount, 1.0f);
    stuckCounters.resize(bandCount, 0);
}

void RhythmInterpreter::processAudioFrame(const std::vector<float>& audioData) {
    for (size_t band = 0; band < bandCount; ++band) {
        float filtered = bandpassFilter(audioData, bandFrequencies[band], bandBandwidths[band]);
        float rms = std::sqrt(std::accumulate(audioData.begin(), audioData.end(), 0.0f,
            [](float acc, float v) { return acc + v * v; }) / audioData.size());
        float envelope = std::accumulate(audioData.begin(), audioData.end(), 0.0f,
            [](float acc, float v) { return acc + std::abs(v); }) / audioData.size();
        float energy = 0.7f * rms + 0.3f * envelope;

        // Anti-stuck: if output stuck high, force cycling
        if (filterOutputs[band] > bandLimits[band] * 0.8f) stuckCounters[band]++;
        else stuckCounters[band] = 0;
        if (stuckCounters[band] > 20) {
            energy = 0.01f * (0.5f + 0.5f * std::sin(frameCounter * 0.1f + band));
            stuckCounters[band] = 0;
        }
        float output = energy * bandGains[band] * bandScalings[band];
        filterOutputs[band] = std::clamp(output, 0.0f, bandLimits[band]);
    }
    frameCounter++;
}

std::vector<float> RhythmInterpreter::getFilterOutputs() const {
    return filterOutputs;
}

void RhythmInterpreter::initializeBands() {
    bandFrequencies = {0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f};
    bandBandwidths = {0.03f, 0.06f, 0.12f, 0.25f, 0.5f, 1.5f, 3.0f, 6.0f};
    bandScalings = {1.0f, 1.0f, 1.0f, 1.0f, 1.2f, 2.0f, 2.5f, 3.0f};
    bandLimits = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.7f, 0.8f, 0.9f};
}

float RhythmInterpreter::bandpassFilter(const std::vector<float>& data, float freq, float bw) {
    // Placeholder: just return mean for now
    return std::accumulate(data.begin(), data.end(), 0.0f) / data.size();
}