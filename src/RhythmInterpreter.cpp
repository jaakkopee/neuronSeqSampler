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
        float rawBandEnergy = onset * bandScalings[band];
        
        // Add some base level from total energy for continuous detection
        rawBandEnergy += totalEnergy * 0.1f * bandScalings[band];
        
        // Update adaptive sensitivity for this band
        updateAdaptiveSensitivity(band, rawBandEnergy);
        
        // Apply contrast enhancement heuristics
        float enhancedEnergy = applyContrastEnhancement(band, rawBandEnergy);
        
        // Anti-stuck: if output stuck high, force cycling
        if (filterOutputs[band] > bandLimits[band] * 0.8f) stuckCounters[band]++;
        else stuckCounters[band] = 0;
        if (stuckCounters[band] > 20) {
            enhancedEnergy = 0.01f * (0.5f + 0.5f * std::sin(frameCounter * 0.1f + band));
            stuckCounters[band] = 0;
        }
        
        // Apply user sensitivity multiplier and filter gain
        float output = enhancedEnergy * bandGains[band] * filterGains[band];
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

void RhythmInterpreter::updateAdaptiveSensitivity(size_t bandIndex, float rawEnergy) {
    if (bandIndex >= bandCount) return;
    
    const float adaptationRate = 0.001f;  // Slow adaptation
    const float peakDecay = 0.999f;       // Peak values decay slowly
    const float baselineRate = 0.0005f;   // Very slow baseline adaptation
    
    // Update noise floor (minimum energy seen recently)
    noiseFloors[bandIndex] = std::min(noiseFloors[bandIndex] * 0.9999f + rawEnergy * 0.0001f, rawEnergy);
    
    // Update energy peaks (maximum energy seen recently)  
    if (rawEnergy > energyPeaks[bandIndex]) {
        energyPeaks[bandIndex] = rawEnergy;
    } else {
        energyPeaks[bandIndex] *= peakDecay;  // Slowly decay peaks
    }
    
    // Update baseline (average quiet period energy)
    if (rawEnergy < energyBaselines[bandIndex] * 2.0f) {  // Only update during quiet periods
        energyBaselines[bandIndex] = energyBaselines[bandIndex] * (1.0f - baselineRate) + rawEnergy * baselineRate;
    }
    
    // Calculate dynamic range
    dynamicRanges[bandIndex] = std::max(0.001f, energyPeaks[bandIndex] - energyBaselines[bandIndex]);
    
    // Update adaptive sensitivity every 100 frames to avoid too frequent changes
    adaptationCounters[bandIndex]++;
    if (adaptationCounters[bandIndex] >= 100) {
        adaptationCounters[bandIndex] = 0;
        
        // Calculate optimal sensitivity for maximum contrast
        float targetDynamicRange = 0.8f;  // We want 80% of the output range to be used
        float currentUtilization = dynamicRanges[bandIndex] / bandLimits[bandIndex];
        
        if (currentUtilization < 0.1f) {
            // Too quiet - increase sensitivity
            adaptiveSensitivities[bandIndex] = std::min(10.0f, adaptiveSensitivities[bandIndex] * 1.1f);
        } else if (currentUtilization > 0.9f) {
            // Too loud - decrease sensitivity  
            adaptiveSensitivities[bandIndex] = std::max(0.1f, adaptiveSensitivities[bandIndex] * 0.9f);
        }
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