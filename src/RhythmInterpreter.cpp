#include "RhythmInterpreter.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <complex>
#include <iostream>
#include <fftw3.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

RhythmInterpreter::RhythmInterpreter(size_t sampleRate, size_t bufferSize)
    : sampleRate(sampleRate), bufferSize(bufferSize), bandCount(8), frameCounter(0), 
      GTFilterBank(sampleRate, 8) {
    initializeBands();
    // Initialize runtime vectors (don't override the parameter vectors set in initializeBands)
    filterOutputs.resize(bandCount, 0.0f);
    bandGains.resize(bandCount, 1.0f);
    filterGains.resize(bandCount, 1.0f);
    stuckCounters.resize(bandCount, 0);
    // qValues is already initialized in initializeBands() - don't override it
}

void RhythmInterpreter::processAudioFrame(const std::vector<float>& audioData) {
    frameCounter++;
    
    // Debug: Log audio data status occasionally
    if (frameCounter % 1000 == 0) {
        float totalEnergy = 0.0f;
        for (float sample : audioData) {
            totalEnergy += sample * sample;
        }
        std::cout << "DEBUG: Frame " << frameCounter << " - Audio size: " << audioData.size() 
                  << " - Energy: " << std::sqrt(totalEnergy / std::max(1.0f, (float)audioData.size())) << std::endl;
    }
    
    // Check if we have audio input
    if (audioData.empty()) {
        // No audio - set all outputs to zero
        std::fill(filterOutputs.begin(), filterOutputs.end(), 0.0f);
        if (frameCounter % 1000 == 0) {
            std::cout << "DEBUG: No audio data - setting outputs to zero" << std::endl;
        }
        return;
    }
    
    // Calculate overall audio energy for normalization
    float totalEnergy = 0.0f;
    for (float sample : audioData) {
        totalEnergy += sample * sample;
    }
    totalEnergy = std::sqrt(totalEnergy / audioData.size());
    
    // Normalize the audio data to prevent filter saturation
    std::vector<float> normalizedAudio = audioData;
    float normalizationFactor = 0.1f; // Strong normalization to keep outputs reasonable
    
    // Apply peak normalization if audio is too loud
    float maxSample = 0.0f;
    for (float sample : audioData) {
        maxSample = std::max(maxSample, std::abs(sample));
    }
    
    if (maxSample > 0.001f) { // Avoid division by zero
        float peakNormalization = std::min(1.0f, 0.2f / maxSample); // Normalize peak to 0.2 maximum
        for (size_t i = 0; i < normalizedAudio.size(); ++i) {
            normalizedAudio[i] *= (normalizationFactor * peakNormalization);
        }
    }
    
    // Process each band independently for rhythm pattern detection
    for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
        float freq = bandFrequencies[bandIndex];
        float bw = bandBandwidths[bandIndex];
        float q = qValues[bandIndex];
        
        // Use frequency-specific processing for distinct rhythm patterns with normalized audio
        std::vector<float> bandData = bandpassFilter(normalizedAudio, freq, bw, q);
        
        // Advanced rhythm detection with time-varying analysis
        float rhythmActivity = 0.0f;
        
        if (!bandData.empty()) {
            // Different processing based on frequency range for rhythm patterns
            if (freq < 200.0f) {
                // Low frequencies: Beat and kick drum detection
                float peakEnergy = 0.0f;
                for (size_t i = 0; i < bandData.size(); ++i) {
                    peakEnergy = std::max(peakEnergy, std::abs(bandData[i]));
                }
                rhythmActivity = peakEnergy * 1.0f; // Reduced from 2.0f to 1.0f
                
            } else if (freq < 1000.0f) {
                // Mid frequencies: Snare and vocal rhythms
                float changeRate = 0.0f;
                for (size_t i = 1; i < bandData.size(); ++i) {
                    float change = std::abs(bandData[i] - bandData[i-1]);
                    changeRate += change;
                }
                rhythmActivity = changeRate / bandData.size() * 1.5f; // Reduced from 3.0f to 1.5f
                
            } else if (freq < 4000.0f) {
                // High-mid frequencies: Hi-hat and percussion
                float transientEnergy = 0.0f;
                for (size_t i = 2; i < bandData.size(); ++i) {
                    float transient = std::abs(bandData[i] - 2.0f * bandData[i-1] + bandData[i-2]);
                    transientEnergy += transient;
                }
                rhythmActivity = transientEnergy / bandData.size() * 2.0f; // Reduced from 4.0f to 2.0f
                
            } else {
                // High frequencies: Cymbal crashes and sibilance
                float highFreqActivity = 0.0f;
                for (size_t i = 1; i < bandData.size(); ++i) {
                    float highpass = bandData[i] - bandData[i-1];
                    highFreqActivity += std::abs(highpass);
                }
                rhythmActivity = highFreqActivity / bandData.size() * 2.0f; // Reduced from 5.0f to 2.0f
            }
        }
        
        // Apply faster temporal smoothing for more responsive rhythm detection
        float smoothingFactor = 0.65f; // Much faster response (was 0.85f)
        static std::vector<float> smoothedOutputs(bandCount, 0.0f);
        if (smoothedOutputs.size() != bandCount) {
            smoothedOutputs.resize(bandCount, 0.0f);
        }
        
        // More aggressive adaptive smoothing for faster envelope following
        if (rhythmActivity > smoothedOutputs[bandIndex]) {
            // Very fast attack for immediate rhythmic response
            smoothedOutputs[bandIndex] = 0.2f * smoothedOutputs[bandIndex] + 0.8f * rhythmActivity;
        } else {
            // Faster decay for quicker envelope following
            smoothedOutputs[bandIndex] = smoothingFactor * smoothedOutputs[bandIndex] + (1.0f - smoothingFactor) * rhythmActivity;
        }
        
        // Apply frequency-dependent scaling
        float scaledActivity = smoothedOutputs[bandIndex] * bandScalings[bandIndex];
        
        // Apply band-specific limit but allow values above 1.0 for dynamic range
        scaledActivity = std::min(scaledActivity, bandLimits[bandIndex]);
        scaledActivity = std::max(0.0f, scaledActivity); // Only clamp minimum, not maximum
        
        // Store the rhythm pattern output
        filterOutputs[bandIndex] = scaledActivity;
    }

    // Apply filter gains to the outputs
    for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
        filterOutputs[bandIndex] *= filterGains[bandIndex];
        // Only prevent negative values, allow dynamic range above 1.0
        filterOutputs[bandIndex] = std::max(0.0f, filterOutputs[bandIndex]);
    }
}

std::vector<float> RhythmInterpreter::getFilterOutputs() const {
    return filterOutputs;
}

void RhythmInterpreter::initializeBands() {
    // Use proper frequency bands for audio processing (Hz)
    bandFrequencies = {0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f};
    bandBandwidths = {0.25f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f};
    bandScalings = {1.0f, 1.0f, 1.0f, 1.0f, 1.2f, 2.0f, 2.5f, 3.0f};
    bandLimits = {2.0f, 2.0f, 1.8f, 1.6f, 1.4f, 1.2f, 1.0f, 0.8f}; // Increased limits for more dynamic range
    // Lower Q-values for faster response and broader frequency capture
    qValues = {4.0f, 4.0f, 3.5f, 3.0f, 2.5f, 2.0f, 1.8f, 1.5f}; // Much lower Q for faster envelope following
}

// Removed setSensitivity and getSensitivity methods - direct access to bandGains only

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

// Removed adaptive sensitivity system - using direct user controls only

// Removed updateAdaptiveSensitivity and applyContrastEnhancement methods

std::vector<float> RhythmInterpreter::bandpassFilter(const std::vector<float>& data, float freq, float bw, float q) {
    // Proper Q-factor based bandpass filtering with +12dB gain boost
    std::vector<float> filteredData(data.size());
    
    if (data.empty()) {
        return filteredData;
    }
    
    // Calculate biquad bandpass filter coefficients with Q-factor
    float nyquist = sampleRate / 2.0f;
    float normalizedFreq = freq / nyquist;
    normalizedFreq = std::max(0.001f, std::min(normalizedFreq, 0.999f));
    
    // Angular frequency
    float omega = 2.0f * M_PI * normalizedFreq;
    float sinOmega = std::sin(omega);
    float cosOmega = std::cos(omega);
    
    // Q-factor constrained to reasonable range
    float effectiveQ = std::max(0.5f, std::min(q, 20.0f));
    float alpha = sinOmega / (2.0f * effectiveQ);
    
    // Biquad bandpass coefficients
    float b0 = alpha;
    float b1 = 0.0f;
    float b2 = -alpha;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * cosOmega;
    float a2 = 1.0f - alpha;
    
    // Normalize coefficients
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;
    
    // +12dB gain boost (linear scale: 12dB ≈ 4.0x)
    float gainBoost = 4.0f;
    b0 *= gainBoost;
    b1 *= gainBoost;
    b2 *= gainBoost;
    
    // Filter state variables
    float x1 = 0.0f, x2 = 0.0f;  // Input history
    float y1 = 0.0f, y2 = 0.0f;  // Output history
    
    // Apply biquad bandpass filter
    for (size_t i = 0; i < data.size(); ++i) {
        float x0 = data[i];
        
        // Biquad difference equation
        float y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        
        // Update history
        x2 = x1;
        x1 = x0;
        y2 = y1;
        y1 = y0;
        
        // Store rectified output for rhythm detection
        filteredData[i] = std::abs(y0);
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

GammaToneFilterBank::GammaToneFilterBank(size_t sampleRate, size_t bandCount)
    : sampleRate(sampleRate), bandCount(bandCount), fftSize(1024) {
    
    // Initialize band configuration with proper gamma-tone frequencies
    bands.resize(bandCount);
    
    // Use ERB (Equivalent Rectangular Bandwidth) scale for gamma-tone spacing
    float minFreq = 80.0f;   // Lowest frequency
    float maxFreq = 8000.0f; // Highest frequency
    
    for (size_t i = 0; i < bandCount; ++i) {
        float erb = minFreq + (maxFreq - minFreq) * (float(i) / (bandCount - 1));
        bands[i].centerFrequency = erb;
        bands[i].bandwidth = 24.7f * (4.37f * erb / 1000.0f + 1.0f); // ERB formula
        bands[i].gain = 1.0f;
        bands[i].order = 4; // Traditional gamma-tone filter order
        
        // Legacy state for time-domain fallback
        bands[i].prevInput1 = 0.0f;
        bands[i].prevInput2 = 0.0f;
        bands[i].prevOutput1 = 0.0f;
        bands[i].prevOutput2 = 0.0f;
    }
    
    // Initialize FFTW and generate frequency responses
    initializeFFTW();
    
    // Generate pre-computed frequency domain responses for each band
    for (size_t i = 0; i < bandCount; ++i) {
        generateGammaToneResponse(i);
    }
    
    // Initialize overlap buffer for overlap-add processing
    overlapBuffer.resize(fftSize, 0.0f);
}

GammaToneFilterBank::~GammaToneFilterBank() {
    cleanupFFTW();
}

void GammaToneFilterBank::initializeFFTW() {
    // Allocate FFTW arrays
    fftInput = (double*) fftw_malloc(sizeof(double) * fftSize);
    fftOutput = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (fftSize / 2 + 1));
    ifftInput = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (fftSize / 2 + 1));
    ifftOutput = (double*) fftw_malloc(sizeof(double) * fftSize);
    
    // Create FFTW plans
    forwardPlan = fftw_plan_dft_r2c_1d(fftSize, fftInput, fftOutput, FFTW_MEASURE);
    inversePlan = fftw_plan_dft_c2r_1d(fftSize, ifftInput, ifftOutput, FFTW_MEASURE);
}

void GammaToneFilterBank::cleanupFFTW() {
    fftw_destroy_plan(forwardPlan);
    fftw_destroy_plan(inversePlan);
    fftw_free(fftInput);
    fftw_free(fftOutput);
    fftw_free(ifftInput);
    fftw_free(ifftOutput);
}

std::complex<double> GammaToneFilterBank::gammaToneFrequencyResponse(double frequency, const GammaToneBand& band) {
    // Gamma-tone filter frequency response: H(f) = (1 + j*f/fc)^(-n)
    // where fc is center frequency, n is order (typically 4)
    
    double normalizedFreq = frequency / band.centerFrequency;
    std::complex<double> jw(0.0, normalizedFreq);
    std::complex<double> denominator = 1.0 + jw;
    
    // Raise to the power of -order
    std::complex<double> response = std::pow(denominator, -double(band.order));
    
    // Apply bandwidth scaling
    double bandwidthFactor = band.centerFrequency / band.bandwidth;
    response *= bandwidthFactor;
    
    return response * std::complex<double>(band.gain, 0.0);
}

void GammaToneFilterBank::generateGammaToneResponse(size_t bandIndex) {
    if (bandIndex >= bands.size()) return;
    
    GammaToneBand& band = bands[bandIndex];
    band.frequencyResponse.resize(fftSize / 2 + 1);
    
    double freqStep = double(sampleRate) / fftSize;
    
    for (size_t k = 0; k < fftSize / 2 + 1; ++k) {
        double frequency = k * freqStep;
        band.frequencyResponse[k] = gammaToneFrequencyResponse(frequency, band);
    }
}

std::vector<float> GammaToneFilterBank::FFTProcess(const std::vector<float>& input) {
    if (input.size() != fftSize) {
        // For simplicity, pad or truncate to fftSize
        std::vector<float> paddedInput(fftSize, 0.0f);
        size_t copySize = std::min(input.size(), fftSize);
        std::copy(input.begin(), input.begin() + copySize, paddedInput.begin());
        return FFTProcess(paddedInput);
    }
    
    std::vector<float> output(fftSize, 0.0f);
    
    // Copy input to FFTW input buffer
    for (size_t i = 0; i < fftSize; ++i) {
        fftInput[i] = input[i];
    }
    
    // Perform forward FFT
    fftw_execute(forwardPlan);
    
    // Process each gamma-tone band and accumulate results
    for (size_t band = 0; band < bandCount; ++band) {
        // Apply gamma-tone filter in frequency domain
        for (size_t k = 0; k < fftSize / 2 + 1; ++k) {
            std::complex<double> inputSpectrum(fftOutput[k][0], fftOutput[k][1]);
            std::complex<double> filtered = inputSpectrum * bands[band].frequencyResponse[k];
            
            ifftInput[k][0] = filtered.real();
            ifftInput[k][1] = filtered.imag();
        }
        
        // Perform inverse FFT for this band
        fftw_execute(inversePlan);
        
        // Accumulate the filtered result with normalization
        double scale = 1.0 / fftSize;
        for (size_t i = 0; i < fftSize; ++i) {
            output[i] += float(ifftOutput[i] * scale / bandCount);
        }
    }
    
    return output;
}

std::vector<float> GammaToneFilterBank::process(const std::vector<float>& input) {
    // Use FFT processing as the primary method
    return FFTProcess(input);
}