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
    filterGains.resize(bandCount, 1.0f);
    stuckCounters.resize(bandCount, 0);
}

void RhythmInterpreter::processAudioFrame(const std::vector<float>& audioData) {
    // Calculate overall audio energy for this frame
    float totalEnergy = 0.0f;
    for (float sample : audioData) {
        totalEnergy += sample * sample;
    }
    totalEnergy = std::sqrt(totalEnergy / audioData.size());
    
    // For rhythm detection, we need to work with onset detection and temporal patterns
    // rather than traditional frequency filtering
    
    for (size_t band = 0; band < bandCount; ++band) {
        // Use the total energy and apply different temporal smoothing for each band
        float smoothingFactor = 0.95f - (band * 0.05f); // More responsive for higher bands
        
        // Simple onset detection based on energy changes
        static std::vector<float> previousEnergy(bandCount, 0.0f);
        float energyDelta = totalEnergy - previousEnergy[band];
        previousEnergy[band] = previousEnergy[band] * smoothingFactor + totalEnergy * (1.0f - smoothingFactor);
        
        // Detect energy increases (onsets)
        float onset = std::max(0.0f, energyDelta);
        
        // Apply band-specific scaling and processing
        float bandEnergy = onset * bandScalings[band];
        
        // Add some base level from total energy for continuous detection
        bandEnergy += totalEnergy * 0.1f * bandScalings[band];
        
        // Anti-stuck: if output stuck high, force cycling
        if (filterOutputs[band] > bandLimits[band] * 0.8f) stuckCounters[band]++;
        else stuckCounters[band] = 0;
        if (stuckCounters[band] > 20) {
            bandEnergy = 0.01f * (0.5f + 0.5f * std::sin(frameCounter * 0.1f + band));
            stuckCounters[band] = 0;
        }
        
        float output = bandEnergy * bandGains[band] * filterGains[band];
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
    qValues = {8.0f, 8.0f, 8.0f, 8.0f, 6.0f, 4.0f, 3.0f, 2.0f};
}

void RhythmInterpreter::setSensitivity(size_t bandIndex, float gain) {
    if (bandIndex < bandGains.size()) {
        bandGains[bandIndex] = gain;
    }
}

float RhythmInterpreter::getSensitivity(size_t bandIndex) const {
    if (bandIndex < bandGains.size()) {
        return bandGains[bandIndex];
    }
    return 1.0f;
}

void RhythmInterpreter::setFilterGain(size_t bandIndex, float gain) {
    if (bandIndex < filterGains.size()) {
        filterGains[bandIndex] = gain;
    }
}

float RhythmInterpreter::getFilterGain(size_t bandIndex) const {
    if (bandIndex < filterGains.size()) {
        return filterGains[bandIndex];
    }
    return 1.0f;
}

std::vector<float> RhythmInterpreter::bandpassFilter(const std::vector<float>& data, float freq, float bw) {
    // Simple resonant bandpass filter implementation
    // For rhythm detection, we need to work with envelope detection rather than direct filtering
    
    std::vector<float> filteredData(data.size());
    
    // For very low frequencies like rhythm detection, we use envelope-based detection
    // Calculate running RMS with a window appropriate for the frequency
    float windowSizeMs = 1000.0f / freq; // Window size in milliseconds based on frequency
    size_t windowSize = static_cast<size_t>((windowSizeMs / 1000.0f) * sampleRate);
    windowSize = std::max(windowSize, static_cast<size_t>(32)); // Minimum window
    windowSize = std::min(windowSize, data.size() / 2); // Maximum window
    
    // Calculate envelope using a sliding window RMS
    for (size_t i = 0; i < data.size(); ++i) {
        float sum = 0.0f;
        size_t count = 0;
        
        // Calculate RMS over the window
        for (size_t j = (i >= windowSize) ? i - windowSize : 0; j <= i && j < data.size(); ++j) {
            sum += data[j] * data[j];
            count++;
        }
        
        if (count > 0) {
            filteredData[i] = std::sqrt(sum / count);
        } else {
            filteredData[i] = 0.0f;
        }
    }
    
    return filteredData;
}