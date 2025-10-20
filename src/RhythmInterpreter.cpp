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

void RhythmInterpreter::setSensitivity(size_t bandIndex, float gain, float sigmoidGain) {
    if (bandIndex < bandGains.size()) {
        // run through a sigmoid to map gain from [0.0, 2.0] to [0.5, 2.0] smoothly
        float minGain = 0.5f;
        float maxGain = 2.0f;
        float sigmoidGain = minGain + (maxGain - minGain) / (1.0f + std::exp(- (gain - 1.0f) * 5.0f));
        bandGains[bandIndex] = sigmoidGain;
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
    // Simple resonant bandpass filter implementation
    // For rhythm detection, we need to work with envelope detection rather than direct filtering
    // This means we will not apply a traditional filter but rather detect the envelope of the signal
    // use an resonant filters. First the coefficients
    float c0 = 1.0f / std::tan(M_PI * bw / sampleRate);
    for (size_t i = 0; i < qValues.size(); ++i) {
        if (bandFrequencies[i] == freq) {
            c0 = 1.0f / std::tan(M_PI * bw / sampleRate);
            break;
        }
    }
    float a0 = 1.0f / (1.0f + c0 / qValues[0] + c0 * c0);
    float a1 = 2.0f * a0;
    float a2 = a0;
    float b1 = 2.0f * a0 * (1.0f - c0 * c0);
    float b2 = a0 * (1.0f - c0 / qValues[0] + c0 * c0);

    std::vector<float> filteredData(data.size());
    
    // Simple Direct Form II implementation
    for (size_t i = 0; i < data.size(); ++i) {
        // Filter processing
        std::vector<float> x(3, 0.0f); // input samples
        std::vector<float> y(3, 0.0f); // output samples
        x[0] = data[i];
        y[0] = a0 * x[0] + a1 * x[1] + a2 * x[2] - b1 * y[1] - b2 * y[2];
        // Shift samples
        x[2] = x[1];
        x[1] = x[0];
        y[2] = y[1];
        y[1] = y[0];
    }

    return filteredData;
}

std::vector<float> RhythmInterpreter::envelopeDetection(const std::vector<float>& data, int bandIndex) {
    // Simple envelope detection using RMS over a sliding window
    std::vector<float> envelope(data.size());
    
    size_t windowSize = static_cast<size_t>((0.1f * sampleRate)); // 100ms window
    windowSize = std::max(windowSize, static_cast<size_t>(32));
    windowSize = std::min(windowSize, data.size() / 2);
    
    for (size_t i = 0; i < data.size(); ++i) {
        float sum = 0.0f;
        size_t count = 0;
        
        for (size_t j = (i >= windowSize) ? i - windowSize : 0; j <= i && j < data.size(); ++j) {
            sum += data[j] * data[j];
            count++;
        }
        
        if (count > 0) {
            envelope[i] = std::sqrt(sum / count);
        } else {
            envelope[i] = 0.0f;
        }
    }
    
    return envelope;
}
