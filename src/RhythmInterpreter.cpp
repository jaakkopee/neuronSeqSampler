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
    
    // Initialize adaptive sensitivity system
    adaptiveSensitivities.resize(bandCount, 1.0f);
    energyBaselines.resize(bandCount, 0.0f);
    energyPeaks.resize(bandCount, 0.0f);
    noiseFloors.resize(bandCount, 0.001f);  // Small initial noise floor
    dynamicRanges.resize(bandCount, 1.0f);
    adaptationCounters.resize(bandCount, 0);
}

void RhythmInterpreter::processAudioFrame(const std::vector<float>& audioData) {
    frameCounter++;
    
    // Process each band
    for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
        // Bandpass filter the audio data for this band
        std::vector<float> bandData = bandpassFilter(audioData, bandFrequencies[bandIndex], bandBandwidths[bandIndex]);
        
        // Envelope detection
        std::vector<float> envelope = envelopeDetection(bandData, static_cast<int>(bandIndex));
        
        // Calculate raw energy for this band
        float rawEnergy = std::accumulate(envelope.begin(), envelope.end(), 0.0f) / static_cast<float>(envelope.size());
        
        // Update adaptive sensitivity
        updateAdaptiveSensitivity(bandIndex, rawEnergy);
        
        // Apply contrast enhancement
        float enhancedEnergy = applyContrastEnhancement(bandIndex, rawEnergy);
        
        // Apply user sensitivity and adaptive sensitivity
        float effectiveSensitivity = getSensitivity(bandIndex);
        float adjustedEnergy = enhancedEnergy * effectiveSensitivity;
        
        // Limit to band limit
        adjustedEnergy = std::min(adjustedEnergy, bandLimits[bandIndex]);
        
        // Apply filter gain
        filterOutputs[bandIndex] = adjustedEnergy * filterGains[bandIndex];
    }
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
        // Direct assignment - gain range is -3.0 to 10.0 as controlled by GUI
        bandGains[bandIndex] = gain;
        // Reset adaptive sensitivity when user changes it to allow re-adaptation
        if (bandIndex < adaptiveSensitivities.size()) {
            adaptiveSensitivities[bandIndex] = 1.0f;
            adaptationCounters[bandIndex] = 0;
        }
    }
}

float RhythmInterpreter::getSensitivity(size_t bandIndex) const {
    if (bandIndex < bandGains.size()) {
        // Return the effective sensitivity (user * adaptive)
        float userSensitivity = bandGains[bandIndex];
        float adaptiveSensitivity = (bandIndex < adaptiveSensitivities.size()) ? adaptiveSensitivities[bandIndex] : 1.0f;
        return userSensitivity * adaptiveSensitivity;
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

// Band frequency control
void RhythmInterpreter::setBandFrequency(size_t bandIndex, float frequency) {
    if (bandIndex < bandFrequencies.size()) {
        bandFrequencies[bandIndex] = frequency;
    }
}

float RhythmInterpreter::getBandFrequency(size_t bandIndex) const {
    if (bandIndex < bandFrequencies.size()) {
        return bandFrequencies[bandIndex];
    }
    return 1.0f;
}

// Band bandwidth control
void RhythmInterpreter::setBandBandwidth(size_t bandIndex, float bandwidth) {
    if (bandIndex < bandBandwidths.size()) {
        bandBandwidths[bandIndex] = bandwidth;
    }
}

float RhythmInterpreter::getBandBandwidth(size_t bandIndex) const {
    if (bandIndex < bandBandwidths.size()) {
        return bandBandwidths[bandIndex];
    }
    return 0.1f;
}

// Band scaling control
void RhythmInterpreter::setBandScaling(size_t bandIndex, float scaling) {
    if (bandIndex < bandScalings.size()) {
        bandScalings[bandIndex] = scaling;
    }
}

float RhythmInterpreter::getBandScaling(size_t bandIndex) const {
    if (bandIndex < bandScalings.size()) {
        return bandScalings[bandIndex];
    }
    return 1.0f;
}

// Band limit control
void RhythmInterpreter::setBandLimit(size_t bandIndex, float limit) {
    if (bandIndex < bandLimits.size()) {
        bandLimits[bandIndex] = limit;
    }
}

float RhythmInterpreter::getBandLimit(size_t bandIndex) const {
    if (bandIndex < bandLimits.size()) {
        return bandLimits[bandIndex];
    }
    return 1.0f;
}

// Q value control
void RhythmInterpreter::setQValue(size_t bandIndex, float q) {
    if (bandIndex < qValues.size()) {
        qValues[bandIndex] = q;
    }
}

float RhythmInterpreter::getQValue(size_t bandIndex) const {
    if (bandIndex < qValues.size()) {
        return qValues[bandIndex];
    }
    return 1.0f;
}

// Adaptive sensitivity system getters (read-only for monitoring)
float RhythmInterpreter::getAdaptiveSensitivity(size_t bandIndex) const {
    if (bandIndex < adaptiveSensitivities.size()) {
        return adaptiveSensitivities[bandIndex];
    }
    return 1.0f;
}

float RhythmInterpreter::getEnergyBaseline(size_t bandIndex) const {
    if (bandIndex < energyBaselines.size()) {
        return energyBaselines[bandIndex];
    }
    return 0.0f;
}

float RhythmInterpreter::getEnergyPeak(size_t bandIndex) const {
    if (bandIndex < energyPeaks.size()) {
        return energyPeaks[bandIndex];
    }
    return 0.0f;
}

float RhythmInterpreter::getNoiseFloor(size_t bandIndex) const {
    if (bandIndex < noiseFloors.size()) {
        return noiseFloors[bandIndex];
    }
    return 0.001f;
}

float RhythmInterpreter::getDynamicRange(size_t bandIndex) const {
    if (bandIndex < dynamicRanges.size()) {
        return dynamicRanges[bandIndex];
    }
    return 1.0f;
}

void RhythmInterpreter::updateAdaptiveSensitivity(size_t bandIndex, float rawEnergy) {
    if (bandIndex >= bandCount) return;
    
    // Update energy baselines and peaks
    float& baseline = energyBaselines[bandIndex];
    float& peak = energyPeaks[bandIndex];
    float& noiseFloor = noiseFloors[bandIndex];
    float& dynamicRange = dynamicRanges[bandIndex];
    int& adaptCounter = adaptationCounters[bandIndex];
    
    // Simple running average for baseline
    baseline = 0.99f * baseline + 0.01f * rawEnergy;
    
    // Update peak
    if (rawEnergy > peak) {
        peak = rawEnergy;
    } else {
        peak *= 0.995f; // Slow decay
    }
    
    // Estimate noise floor
    if (rawEnergy < noiseFloor) {
        noiseFloor = 0.99f * noiseFloor + 0.01f * rawEnergy;
    } else {
        noiseFloor *= 0.999f; // Slow decay
    }
    
    // Update dynamic range
    dynamicRange = peak - baseline;
    dynamicRange = std::max(dynamicRange, 0.001f); // Prevent too small range
    
    // Adaptation logic
    adaptCounter++;
    if (adaptCounter >= 100) { // Update every 100 frames
        // Adjust adaptive sensitivity based on recent energy levels
        if (rawEnergy < baseline + noiseFloor * 2.0f) {
            // Quiet period - increase sensitivity
            adaptiveSensitivities[bandIndex] *= 1.05f;
        } else if (rawEnergy > peak * 0.8f) {
            // Loud period - decrease sensitivity
            adaptiveSensitivities[bandIndex] *= 0.95f;
        }
        
        // Clamp adaptive sensitivity
        adaptiveSensitivities[bandIndex] = std::clamp(adaptiveSensitivities[bandIndex], 0.1f, 10.0f);
        
        adaptCounter = 0;
    }
}

float RhythmInterpreter::applyContrastEnhancement(size_t bandIndex, float energy) {
    if (bandIndex >= bandCount) return energy;
    
    // Apply adaptive sensitivity
    float enhanced = energy * adaptiveSensitivities[bandIndex];
    
    // Dynamic range compression/expansion for better contrast
    float baseline = energyBaselines[bandIndex];
    float range = dynamicRanges[bandIndex];
    
    if (range > 0.001f) {
        // Normalize to 0-1 based on current dynamic range
        float normalized = std::clamp((enhanced - baseline) / range, 0.0f, 1.0f);
        
        // Apply contrast curve - makes quiet sounds quieter and loud sounds louder
        float contrast = 2.0f;  // Contrast factor
        float curved = std::pow(normalized, 1.0f / contrast);
        
        // Map back to energy scale
        enhanced = baseline + curved * range;
    }
    
    // Noise gate - suppress very quiet signals
    float gateThreshold = noiseFloors[bandIndex] * 3.0f;
    if (enhanced < gateThreshold) {
        enhanced *= enhanced / gateThreshold;  // Gradual gate rather than hard cut
    }
    
    return enhanced;
}

std::vector<float> RhythmInterpreter::bandpassFilter(const std::vector<float>& data, float freq, float bw) {
    // Simplified envelope-based approach for rhythm detection
    // Instead of complex filtering, use a simple frequency-weighted envelope
    std::vector<float> filteredData(data.size());
    
    if (data.empty()) {
        return filteredData;
    }
    
    // Simple high-pass emphasis for higher frequencies
    float freqWeight = 1.0f + (freq - 1.0f) * 0.2f; // Boost higher frequencies slightly
    freqWeight = std::max(0.5f, std::min(freqWeight, 2.0f));
    
    // Simple envelope extraction with frequency weighting
    for (size_t i = 0; i < data.size(); ++i) {
        float sample = data[i] * freqWeight;
        filteredData[i] = std::abs(sample); // Rectification for envelope
    }
    
    return filteredData;
}

std::vector<float> RhythmInterpreter::envelopeDetection(const std::vector<float>& data, int bandIndex) {
    // Optimized envelope detection using simple smoothing
    std::vector<float> envelope(data.size());
    
    if (data.empty()) {
        return envelope;
    }
    
    // Simple first-order low-pass filter for envelope following
    float smoothingFactor = 0.95f; // Adjust for different time constants
    envelope[0] = std::abs(data[0]);
    
    for (size_t i = 1; i < data.size(); ++i) {
        float currentEnv = std::abs(data[i]);
        // Attack/decay envelope follower
        if (currentEnv > envelope[i-1]) {
            // Fast attack
            envelope[i] = 0.3f * envelope[i-1] + 0.7f * currentEnv;
        } else {
            // Slow decay
            envelope[i] = smoothingFactor * envelope[i-1] + (1.0f - smoothingFactor) * currentEnv;
        }
    }
    
    return envelope;
}

std::vector<float> RhythmInterpreter::zeroCrossingOnsetDetection(const std::vector<float>& data, int bandIndex) {
    // Simple zero-crossing based onset detection
    std::vector<float> onsetData(data.size(), 0.0f);
    
    for (size_t i = 1; i < data.size(); ++i) {
        if ((data[i - 1] < 0.0f && data[i] >= 0.0f) || (data[i - 1] > 0.0f && data[i] <= 0.0f)) {
            onsetData[i] = 1.0f; // Mark onset at zero crossing
        }
    }
    
    return onsetData;
}