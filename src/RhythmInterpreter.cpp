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
        : sampleRate(sampleRate), bufferSize(bufferSize), bandCount(DEFAULT_BAND_COUNT), frameCounter(0), 
            GTFilterBank(sampleRate, DEFAULT_BAND_COUNT) {
        initializeBands();
    // Initialize runtime vectors (don't override the parameter vectors set in initializeBands)
    filterOutputs.resize(bandCount, 0.0f);
    bandGains.resize(bandCount, 1.0f);
    filterGains.resize(bandCount, 1.0f);
    rhythmogramScale = 1.0f;          // Default scale multiplier
    peakDecayRate = 0.85f;            // Default: 85% decay per frame (fast decay)
    stuckCounters.resize(bandCount, 0);
    // qValues is already initialized in initializeBands() - don't override it
    
    // Initialize onset detection
    onsetHistory.resize(bandCount);
    previousOutputs.resize(bandCount, 0.0f);
    onsetThreshold = 0.02f;           // Lower threshold: detect 2% increase as onset (was 0.1)
    onsetBufferSize = 100;            // Keep last 100 onsets per band
    currentTime = 0.0f;
    
    // Initialize auto-tempo parameters
    autoTempoEnabled = false;
    filterAdaptationEnabled = false;  // Filter adaptation separate from auto-tempo detection
    baseTempoFrequency = 2.0f;        // 2 Hz corresponds to 120 BPM (120/60 = 2)
    tempoSensitivity = 1.0f;
    detectedTempo = 120.0f;           // Default to 120 BPM
    tempoSmoothingFactor = 0.95f;     // Smooth tempo changes
    lastStableTempo = 120.0f;
    
    // Store default frequencies for tempo scaling
    defaultFrequencies = bandFrequencies;
}

void RhythmInterpreter::setTempoSensitivity(float sensitivity) {
    tempoSensitivity = sensitivity;
}

float RhythmInterpreter::getDetectedTempo() const {
    return detectedTempo;
}

float RhythmInterpreter::getTempoSensitivity() const {
    return tempoSensitivity;
}

std::vector<float> RhythmInterpreter::getBeatPositions() const {
    std::vector<float> currentBeatPositions;
    //calculate beat positions based on internal analysis
    for (size_t i = 0; i < bandCount; ++i) {
        if (filterOutputs[i] > 0.7f * bandLimits[i]) {
            // Detected a beat in this band
            float currentTime = static_cast<float>(frameCounter * bufferSize) / static_cast<float>(sampleRate);
            if (currentBeatPositions.empty() || (currentTime - currentBeatPositions.back()) > 0.3f) {
                currentBeatPositions.push_back(currentTime);
            }
        }
    }
    return currentBeatPositions;
}

void RhythmInterpreter::setAutoTempoEnabled(bool enabled) {
    autoTempoEnabled = enabled;
    if (!enabled) {
        // When auto-tempo is disabled, also disable filter adaptation
        filterAdaptationEnabled = false;
        // Restore default frequencies
        for (size_t i = 0; i < bandCount; ++i) {
            setBandFrequency(i, defaultFrequencies[i]);
        }
    }
}

bool RhythmInterpreter::isAutoTempoEnabled() const {
    return autoTempoEnabled;
}

void RhythmInterpreter::setFilterAdaptationEnabled(bool enabled) {
    filterAdaptationEnabled = enabled;
    if (!enabled) {
        // Restore default frequencies when adaptation is disabled
        for (size_t i = 0; i < bandCount; ++i) {
            setBandFrequency(i, defaultFrequencies[i]);
        }
    }
}

bool RhythmInterpreter::isFilterAdaptationEnabled() const {
    return filterAdaptationEnabled;
}

void RhythmInterpreter::setBaseTempoFrequency(float frequency) {
    baseTempoFrequency = frequency;
}

float RhythmInterpreter::getBaseTempoFrequency() const {
    return baseTempoFrequency;
}

std::vector<float> RhythmInterpreter::getBandFrequencies() const {
    return bandFrequencies;
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
    float normalizationFactor = 0.5f; // Less aggressive normalization for stronger outputs
    
    // Apply peak normalization if audio is too loud
    float maxSample = 0.0f;
    for (float sample : audioData) {
        maxSample = std::max(maxSample, std::abs(sample));
    }
    
    if (maxSample > 0.001f) { // Avoid division by zero
        float peakNormalization = std::min(1.0f, 0.8f / maxSample); // Allow higher peaks for stronger signals
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
        
        // Apply faster temporal smoothing for more responsive rhythm detection
        float smoothingFactor = 0.56f; // Much faster response (was 0.85f)
        static std::vector<float> smoothedOutputs(bandCount, 0.0f);
        if (smoothedOutputs.size() != bandCount) {
            smoothedOutputs.resize(bandCount, 0.0f);
        }
        
        // Ensure peak values vector is sized correctly
        if (peakValues.size() != bandCount) {
            peakValues.resize(bandCount, 0.0f);
        }
        
        float rhythmActivity = 0.0f;
        for (float sample : bandData) {
            rhythmActivity += std::abs(sample);
        }
        rhythmActivity /= static_cast<float>(bandData.size()); // Average absolute value
        
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
        scaledActivity = std::max(0.0f, scaledActivity);
        
        // Peak detection and fast decay for sharp peaks
        if (scaledActivity > peakValues[bandIndex]) {
            // New peak detected - set to current value
            peakValues[bandIndex] = scaledActivity;
        } else {
            // Decay existing peak using configurable rate
            peakValues[bandIndex] *= peakDecayRate;
        }
        
        // Use the peak value as output for sharp, short peaks
        filterOutputs[bandIndex] = peakValues[bandIndex];
    }

    // Apply filter gains to the outputs
    for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
        filterOutputs[bandIndex] *= filterGains[bandIndex];
        // Only prevent negative values, allow dynamic range above 1.0
        filterOutputs[bandIndex] = std::max(0.0f, filterOutputs[bandIndex]);
    }
    
    // Apply global rhythmogram scale to all outputs
    for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
        filterOutputs[bandIndex] *= rhythmogramScale;
        filterOutputs[bandIndex] = std::max(0.0f, filterOutputs[bandIndex]); // Ensure non-negative
    }
    
    // Detect onsets based on output changes
    detectOnsets();
    
    // Update current time
    currentTime += static_cast<float>(audioData.size()) / static_cast<float>(sampleRate);
    
    // Auto-tempo detection and frequency adjustment
    if (autoTempoEnabled) {
        // Detect tempo based on rhythmic activity patterns every few frames
        static int tempoAnalysisCounter = 0;
        tempoAnalysisCounter++;
        
        if (tempoAnalysisCounter >= 100) { // Analyze tempo every 100 frames for stability
            tempoAnalysisCounter = 0;
            
            // Calculate tempo from strongest rhythmic bands (low-mid frequencies)
            float tempoIndicator = 0.0f;
            float totalActivity = 0.0f;
            
            // Focus on bands 2-5 (0.5Hz-4Hz) which contain most rhythmic information
            for (size_t i = 2; i < 6 && i < bandCount; ++i) {
                float bandActivity = filterOutputs[i];
                tempoIndicator += bandActivity * bandFrequencies[i]; // Weighted by frequency
                totalActivity += bandActivity;
            }
            
            if (totalActivity > 0.1f) { // Only update if there's significant activity
                float estimatedFrequency = tempoIndicator / totalActivity;
                float estimatedTempo = estimatedFrequency * 60.0f; // Convert Hz to BPM
                
                // Clamp tempo to reasonable range (60-200 BPM)
                estimatedTempo = std::max(60.0f, std::min(200.0f, estimatedTempo));
                
                // Apply smoothing to prevent erratic changes
                detectedTempo = tempoSmoothingFactor * detectedTempo + 
                               (1.0f - tempoSmoothingFactor) * estimatedTempo;
                
                // Update filter frequencies based on detected tempo ONLY if adaptation is enabled
                if (filterAdaptationEnabled) {
                    float tempoRatio = detectedTempo / 120.0f; // Scale relative to 120 BPM base
                    
                    for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
                        float newFreq = defaultFrequencies[bandIndex] * tempoRatio;
                        // Clamp to reasonable frequency range
                        newFreq = std::max(0.1f, std::min(20.0f, newFreq));
                        setBandFrequency(bandIndex, newFreq);
                    }
                }
                
                lastStableTempo = detectedTempo;
            }
        }
    }
}

std::vector<float> RhythmInterpreter::getFilterOutputs() const {
    return filterOutputs;
}

void RhythmInterpreter::initializeBands() {
    // Initialize bands with logarithmic spacing between 0.125 Hz and 16 Hz
    initializeBandsLogarithmic(0.125f, 16.0f, bandCount);
}
void RhythmInterpreter::initializeBandsLogarithmic(float minFreq, float maxFreq, size_t count) {
    bandCount = std::max<size_t>(1, count);
    bandFrequencies.resize(bandCount);
    bandBandwidths.resize(bandCount);
    bandScalings.resize(bandCount);
    bandLimits.resize(bandCount);
    qValues.resize(bandCount);

    // Logarithmic spacing across [minFreq, maxFreq]
    float ratio = maxFreq / minFreq;
    for (size_t i = 0; i < bandCount; ++i) {
        float t = (bandCount == 1) ? 0.0f : static_cast<float>(i) / static_cast<float>(bandCount - 1);
        float f = minFreq * std::pow(ratio, t);
        bandFrequencies[i] = f;
        // Bandwidth proportional to frequency (moderate band width)
        bandBandwidths[i] = std::max(0.001f, f * 0.5f);
        bandScalings[i] = 1.0f;
        bandLimits[i] = 1.5f; // uniform dynamic range cap
        qValues[i] = 2.0f;    // moderate Q for responsive envelope
    }

    // Resize runtime/user control vectors
    bandGains.assign(bandCount, 1.0f);
    filterGains.assign(bandCount, 1.0f);
    filterOutputs.assign(bandCount, 0.0f);
    stuckCounters.assign(bandCount, 0);
    defaultFrequencies = bandFrequencies;
}

void RhythmInterpreter::setBandCount(size_t count) {
    // Reinitialize rhythmic bands and dependent state with logarithmic spacing
    initializeBandsLogarithmic(0.125f, 16.0f, count);
    
    // Resize onset detection buffers
    onsetHistory.resize(count);
    previousOutputs.resize(count, 0.0f);
    
    // Note: GammaToneFilterBank kept as-is since current pipeline uses time-domain bandpassFilter.
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

// Global rhythmogram scale control
void RhythmInterpreter::setRhythmogramScale(float scale) {
    rhythmogramScale = std::max(0.1f, std::min(20.0f, scale)); // Clamp to reasonable range
}

float RhythmInterpreter::getRhythmogramScale() const {
    return rhythmogramScale;
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

// ========================= ONSET DETECTION METHODS =========================

void RhythmInterpreter::detectOnsets() {
    // Clean up old onsets first (older than 5 seconds)
    const float maxOnsetAge = 5.0f;
    for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
        auto& history = onsetHistory[bandIndex];
        history.erase(
            std::remove_if(history.begin(), history.end(),
                [this, maxOnsetAge](const OnsetEvent& onset) {
                    return (currentTime - onset.timestamp) > maxOnsetAge;
                }),
            history.end()
        );
    }
    
    for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
        float currentOutput = filterOutputs[bandIndex];
        float previousOutput = previousOutputs[bandIndex];
        
        // Detect onset if current output exceeds previous by threshold
        float outputChange = currentOutput - previousOutput;
        
        // Enforce minimum absolute threshold to prevent constant triggering
        float minAbsoluteThreshold = 0.005f; // At least 0.5% change required
        float effectiveThreshold = std::max(onsetThreshold, minAbsoluteThreshold);
        
        // Use relative threshold - detect if increase is significant relative to previous level
        float relativeThreshold = effectiveThreshold;
        if (previousOutput > 0.01f) {
            // For signals with existing level, use percentage-based threshold
            relativeThreshold = std::max(effectiveThreshold, previousOutput * effectiveThreshold);
        }
        
        // Detect onset with both absolute and minimum level checks
        // Require significant positive change AND minimum output level
        if (outputChange > relativeThreshold && currentOutput > 0.01f) {
            // Onset detected - add to history
            OnsetEvent onset(currentTime, outputChange, bandIndex);
            onsetHistory[bandIndex].push_back(onset);
            
            // Maintain buffer size limit - remove oldest if exceeded
            if (onsetHistory[bandIndex].size() > onsetBufferSize) {
                onsetHistory[bandIndex].erase(onsetHistory[bandIndex].begin());
            }
        }
        
        // Update previous output for next frame
        previousOutputs[bandIndex] = currentOutput;
    }
}

std::vector<RhythmInterpreter::OnsetEvent> RhythmInterpreter::getOnsetHistory(size_t bandIndex) const {
    if (!isValidBandIndex(bandIndex)) {
        return std::vector<OnsetEvent>();
    }
    return onsetHistory[bandIndex];
}

std::vector<RhythmInterpreter::OnsetEvent> RhythmInterpreter::getAllOnsets() const {
    std::vector<OnsetEvent> allOnsets;
    for (size_t i = 0; i < bandCount; ++i) {
        allOnsets.insert(allOnsets.end(), onsetHistory[i].begin(), onsetHistory[i].end());
    }
    
    // Sort by timestamp
    std::sort(allOnsets.begin(), allOnsets.end(), 
        [](const OnsetEvent& a, const OnsetEvent& b) { return a.timestamp < b.timestamp; });
    
    return allOnsets;
}

void RhythmInterpreter::setOnsetThreshold(float threshold) {
    onsetThreshold = std::max(0.0f, std::min(1.0f, threshold));
}

float RhythmInterpreter::getOnsetThreshold() const {
    return onsetThreshold;
}

void RhythmInterpreter::setOnsetBufferSize(size_t size) {
    onsetBufferSize = size;
    
    // Trim existing buffers if needed
    for (auto& history : onsetHistory) {
        if (history.size() > size) {
            history.erase(history.begin(), history.begin() + (history.size() - size));
        }
    }
}

size_t RhythmInterpreter::getOnsetBufferSize() const {
    return onsetBufferSize;
}

void RhythmInterpreter::clearOnsetHistory() {
    for (auto& history : onsetHistory) {
        history.clear();
    }
}

void RhythmInterpreter::setPeakDecayRate(float rate) {
    peakDecayRate = std::clamp(rate, 0.0f, 1.0f);
}

float RhythmInterpreter::getPeakDecayRate() const {
    return peakDecayRate;
}
