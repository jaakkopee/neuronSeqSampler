#include "RhythmInterpreter.h"
#include "NeuronNetwork.h"
#include "AudioManager.h"
#include "BeatRoot.h"
#include "Debug.h"
#include <algorithm>
#include <random>
#include <cmath>
#include <numeric>
#include <iostream>

/*
 * RHYTHMOGRAM IMPLEMENTATION (Todd, 1994)
 * 
 * This RhythmInterpreter implements the rhythmogram approach for visualizing 
 * rhythmic structure in musical performances. The rhythmogram uses a low-frequency
 * filterbank arranged logarithmically to capture different levels of rhythmic
 * hierarchy:
 * 
 * - 0.125Hz: Long-term phrase structure (8-beat groups, 2-measure phrases)
 * - 0.25Hz:  Whole note level rhythmic structure
 * - 0.5Hz:   Half note level rhythmic structure  
 * - 1.0Hz:   Quarter note level (basic beat)
 * - 2.0Hz:   Eighth note subdivisions
 * - 4.0Hz:   Sixteenth note subdivisions
 * - 8.0Hz:   Thirty-second note subdivisions
 * - 16.0Hz:  Micro-rhythmic variations and onset detection
 * 
 * Fast musical events (e.g., sixteenth notes) activate higher frequency filters,
 * while slower structural elements (e.g., whole notes, phrases) activate lower
 * frequency filters. This creates a tree-like representation of rhythmic hierarchy.
 * 
 * Reference: Todd, N. P. M. (1994). The auditory "primal sketch": A multiscale 
 * model of rhythmic grouping. Journal of New Music Research, 23(1), 25-70.
 */

// Rhythmogram frequency bands following Todd (1994) principles
// Low-frequency filterbank arranged logarithmically to capture rhythmic hierarchy
const std::vector<float> RhythmInterpreter::DEFAULT_FREQUENCIES = {
    0.125f,   // Very long structures (8-beat phrases, 2-measure groups)
    0.25f,    // Whole notes (4-beat rhythmic units)
    0.5f,     // Half notes (2-beat rhythmic units) 
    1.0f,     // Quarter notes (1-beat rhythmic units)
    2.0f,     // Eighth notes (sub-beat rhythmic units)
    4.0f,     // Sixteenth notes (fast rhythmic subdivisions)
    8.0f,     // Thirty-second notes (very fast subdivisions)
    16.0f     // Micro-rhythmic variations and onset detection
};

const std::vector<float> RhythmInterpreter::DEFAULT_BANDWIDTHS = {
    0.03f,    // Very narrow for 0.125Hz (captures 8-beat phrase structure)
    0.06f,    // Narrow for 0.25Hz (captures whole note timing)
    0.12f,    // Narrow for 0.5Hz (captures half note timing)
    0.25f,    // Narrow for 1.0Hz (captures quarter note timing)
    0.5f,     // Medium for 2.0Hz (captures eighth note timing)
    1.0f,     // Medium for 4.0Hz (captures sixteenth note timing)
    2.0f,     // Medium-wide for 8.0Hz (captures thirty-second notes)
    4.0f      // Wide for 16.0Hz (captures micro-timing and onsets)
};

const std::vector<float> RhythmInterpreter::DEFAULT_RESONANCES = {
    8.0f,     // High Q for 0.125Hz (precise long-term structure detection)
    6.0f,     // High Q for 0.25Hz (precise whole note detection)
    5.0f,     // High Q for 0.5Hz (precise half note detection)
    4.0f,     // Medium-high Q for 1.0Hz (quarter note detection)
    3.5f,     // Medium Q for 2.0Hz (eighth note detection)
    3.0f,     // Medium Q for 4.0Hz (sixteenth note detection)
    2.5f,     // Lower Q for 8.0Hz (thirty-second note detection)
    2.0f      // Moderate Q for 16.0Hz (onset detection, allow some bandwidth)
};

// ============================================================================
// AdaptiveFilter Implementation
// ============================================================================

AdaptiveFilter::AdaptiveFilter(float freq, float bw, float adaptRate, float res)
    : centerFrequency(freq), bandwidth(bw), adaptationRate(adaptRate), resonance(res),
      currentEnergy(0.0f), adaptiveGain(1.0f), sampleCount(0.0f), smoothedOutput(0.0f), previousEnergy(0.0f) {
    
    // Initialize simple bandpass filter coefficients
    coefficients.resize(5); // Biquad filter
    delayLine.resize(4, 0.0f); // z^-1 and z^-2 delays for input and output
    
    updateFilterCoefficients();
}

void AdaptiveFilter::updateFilterCoefficients() {
    // For rhythmogram analysis, use different approaches based on frequency range
    float omega = 2.0f * M_PI * centerFrequency / 44100.0f; // Assume 44.1kHz
    
    // For very low frequencies (< 4Hz), omega becomes extremely small and causes numerical issues
    // Instead, we'll detect these in the process() method using envelope following
    if (centerFrequency < 4.0f) {
        // Mark as low-frequency rhythmogram filter - will be handled specially in process()
        // Set coefficients to pass-through for now
        coefficients[0] = 1.0f; // b0 - pass input
        coefficients[1] = 0.0f; // b1
        coefficients[2] = 0.0f; // b2
        coefficients[3] = 0.0f; // a1 
        coefficients[4] = 0.0f; // a2
        return;
    }
    
    // For higher frequencies (>= 4Hz), use traditional biquad bandpass filter
    float Q = resonance; // Use resonance as Q factor
    float alpha = sin(omega) / (2.0f * Q);
    
    float b0 = alpha;
    float b1 = 0.0f;
    float b2 = -alpha;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * cos(omega);
    float a2 = 1.0f - alpha;
    
    // Normalize coefficients
    coefficients[0] = b0 / a0; // b0
    coefficients[1] = b1 / a0; // b1
    coefficients[2] = b2 / a0; // b2
    coefficients[3] = a1 / a0; // a1
    coefficients[4] = a2 / a0; // a2
}

float AdaptiveFilter::process(float input) {
    float output = 0.0f;
    
    // Apply biquad bandpass filter first for high frequencies (>= 4Hz)
    float filteredInput = input;
    if (centerFrequency >= 4.0f) {
        // Apply biquad bandpass filter
        float filtered = coefficients[0] * input + 
                        coefficients[1] * delayLine[0] + 
                        coefficients[2] * delayLine[1] -
                        coefficients[3] * delayLine[2] - 
                        coefficients[4] * delayLine[3];
        
        // Update delay line
        delayLine[1] = delayLine[0];
        delayLine[0] = input;
        delayLine[3] = delayLine[2];
        delayLine[2] = filtered;
        
        filteredInput = filtered;
    }
    
    if (centerFrequency < 4.0f) {
        // RHYTHMOGRAM LOW-FREQUENCY PROCESSING
        // For frequencies < 4Hz, use envelope following and rhythm detection
        
        // Calculate input energy (envelope following)
        float inputEnergy = input * input;
        
        // Update energy estimate with time constant based on target frequency
        // Lower frequencies need longer time constants to capture slower rhythmic patterns
        float timeConstant = 0.99f - (centerFrequency / 4.0f) * 0.1f; // 0.99 to 0.89
        currentEnergy = timeConstant * currentEnergy + (1.0f - timeConstant) * inputEnergy;
        
        // For rhythmogram, we want to detect rhythmic modulation at the target frequency
        // Use a simple oscillating reference to detect correlation with the target rhythm
        sampleCount += 1.0f;
        
        // Generate reference oscillation at target frequency 
        float referenceFreq = centerFrequency; // Hz
        float referencePhase = 2.0f * M_PI * referenceFreq * sampleCount / 44100.0f;
        float reference = (sin(referencePhase) + 1.0f) * 0.5f; // 0 to 1 range
        
        // Correlate energy envelope with reference rhythm
        float correlation = currentEnergy * reference;
        
        // Apply smoothing to the correlation to get rhythmogram output
        float smoothing = 0.95f; // Slower decay for rhythmogram visualization
        smoothedOutput = smoothing * smoothedOutput + (1.0f - smoothing) * correlation;
        
        output = smoothedOutput * adaptiveGain;
        
    } else {
        // ENHANCED HIGH-FREQUENCY PROCESSING (>= 4Hz) for rhythm detection
        
        if (centerFrequency >= 8.0f) {
            // ONSET DETECTION for 8Hz+ (32nd notes and micro-rhythmic onsets)
            
            // Calculate input envelope
            float envelope = std::abs(input);
            
            // Smooth the envelope for better onset detection
            float smoothedEnv = 0.8f * delayLine[0] + 0.2f * envelope;
            delayLine[0] = smoothedEnv;
            
            // Onset strength using rate of change
            float onsetStrength = std::max(0.0f, smoothedEnv - delayLine[1] * 0.9f);
            delayLine[1] = smoothedEnv;
            
            // Apply frequency-specific scaling (higher frequencies get more boost)
            float freqBoost = 1.0f + (centerFrequency - 8.0f) / 8.0f; // 1.0 to 2.0 range
            output = onsetStrength * freqBoost;
            
        } else {
            // ENVELOPE DETECTION for 4-8Hz (16th notes) and 8Hz+ (32nd/onset)
            // Use the bandpass-filtered input for proper frequency-specific envelope detection
            
            // Simple envelope following - start with basic approach
            float envelope = std::abs(filteredInput);
            
            // Apply extremely aggressive boost for high frequencies since bandpass output is extremely weak
            float boost = 10000.0f; // Massive boost for visibility
            if (centerFrequency >= 8.0f) {
                boost = 50000.0f; // Enormous boost for highest frequencies (8Hz, 16Hz)
            }
            envelope *= boost;
            
            // Use moderate attack and release
            float attack = 0.1f;   
            float decay = 0.9f;    
            
            if (envelope > currentEnergy) {
                currentEnergy = attack * envelope + (1.0f - attack) * currentEnergy;
            } else {
                currentEnergy = decay * currentEnergy;
            }
            
            // Larger clamp range for heavily boosted signals
            currentEnergy = std::clamp(currentEnergy, 0.0f, 5.0f);
            
            // For all high frequencies, just use the envelope - no complex onset detection for now
            output = currentEnergy;
        }
        
        // Apply adaptive gain (conservative)
        output *= adaptiveGain;
        
        // Massive visibility boost for high frequencies
        if (centerFrequency >= 4.0f) {
            output *= 100.0f; // 100x additional boost for all high frequencies
        }
    }
    
    return output;
}

void AdaptiveFilter::adaptToRhythm(float rhythmStrength) {
    // Adapt filter gain based on rhythm strength
    float targetGain = 1.0f + 0.5f * rhythmStrength; // Boost during strong rhythm
    adaptiveGain += adaptationRate * (targetGain - adaptiveGain);
    
    // Keep gain in reasonable bounds
    adaptiveGain = std::clamp(adaptiveGain, 0.1f, 2.0f);
}

void AdaptiveFilter::setCenterFrequency(float freq) {
    centerFrequency = freq;
    updateFilterCoefficients();
}

void AdaptiveFilter::setBandwidth(float bw) {
    bandwidth = bw;
    updateFilterCoefficients();
}

void AdaptiveFilter::setResonance(float res) {
    resonance = std::clamp(res, 0.1f, 100.0f); // Clamp to reasonable Q range
    updateFilterCoefficients();
}

// ============================================================================
// RhythmDetector Implementation
// ============================================================================

RhythmDetector::RhythmDetector() 
    : currentTempo(120.0f), beatStrength(0.0f), rhythmicComplexity(0.0f),
      onsetThreshold(0.1f), tempoSmoothingFactor(0.1f) {
    
    microRhythmBuffer.resize(HISTORY_LENGTH, 0.0f);
    beatBuffer.resize(HISTORY_LENGTH, 0.0f);
    onsetBuffer.resize(HISTORY_LENGTH, 0.0f);
    tempoHistory.resize(HISTORY_LENGTH, 120.0f);
}

void RhythmDetector::processFilterOutputs(float onsetOutput, float microRhythmOutput, float beatOutput) {
    // Store filter outputs in buffers for analysis
    microRhythmBuffer.erase(microRhythmBuffer.begin());
    microRhythmBuffer.push_back(microRhythmOutput);
    
    beatBuffer.erase(beatBuffer.begin());
    beatBuffer.push_back(beatOutput);
    
    onsetBuffer.erase(onsetBuffer.begin());
    onsetBuffer.push_back(onsetOutput);
    
    detectOnsets();
    analyzeTempo();
    calculateComplexity();
}

void RhythmDetector::detectOnsets() {
    // Use filter outputs for sophisticated onset detection
    float currentOnset = onsetBuffer.back();
    float currentMicroRhythm = microRhythmBuffer.back();
    float currentBeat = beatBuffer.back();
    
    // Combine filter outputs for onset strength
    // Onset filter (16Hz) provides sharp transient detection
    // Micro-rhythm filter (8Hz) provides rhythmic texture
    // Beat filter (1Hz) provides low-frequency emphasis
    float combinedOnsetStrength = 0.6f * currentOnset + 0.3f * currentMicroRhythm + 0.1f * currentBeat;
    
    // Calculate beat strength using peak detection across filter outputs
    beatStrength = 0.0f;
    if (combinedOnsetStrength > onsetThreshold) {
        // Check if this is a local maximum in the onset buffer
        bool isPeak = true;
        size_t checkRange = std::min(size_t(5), onsetBuffer.size() / 2);
        
        for (size_t i = onsetBuffer.size() - checkRange; i < onsetBuffer.size() - 1; ++i) {
            if (onsetBuffer[i] >= currentOnset) {
                isPeak = false;
                break;
            }
        }
        
        // Also check micro-rhythm consistency for stronger detection
        bool microRhythmPeak = false;
        if (microRhythmBuffer.size() >= 3) {
            float prevMicroRhythm = microRhythmBuffer[microRhythmBuffer.size() - 2];
            float prev2MicroRhythm = microRhythmBuffer[microRhythmBuffer.size() - 3];
            microRhythmPeak = (currentMicroRhythm > prevMicroRhythm) && (prevMicroRhythm > prev2MicroRhythm);
        }
        
        if (isPeak || microRhythmPeak) {
            // Weight beat strength by filter output combination
            beatStrength = std::clamp(combinedOnsetStrength * (1.0f + 0.5f * currentBeat), 0.0f, 1.0f);
        }
    }
}

void RhythmDetector::analyzeTempo() {
    // Enhanced tempo estimation using rhythmogram filter outputs
    std::vector<float> intervals;
    
    // Find peaks in onset buffer with adaptive threshold based on filter combination
    std::vector<size_t> peakIndices;
    
    // Calculate dynamic threshold using micro-rhythm and beat context
    float avgMicroRhythm = std::accumulate(microRhythmBuffer.begin(), microRhythmBuffer.end(), 0.0f) / microRhythmBuffer.size();
    float avgBeat = std::accumulate(beatBuffer.begin(), beatBuffer.end(), 0.0f) / beatBuffer.size();
    float dynamicThreshold = onsetThreshold * (1.0f + 0.5f * avgMicroRhythm + 0.3f * avgBeat);
    
    // Peak detection with filter-enhanced criteria
    for (size_t i = 2; i < onsetBuffer.size() - 2; ++i) {
        float currentOnset = onsetBuffer[i];
        float currentMicroRhythm = microRhythmBuffer[i];
        
        // Multi-criteria peak detection
        bool isOnsetPeak = (currentOnset > onsetBuffer[i-1] && currentOnset > onsetBuffer[i+1] && currentOnset > dynamicThreshold);
        bool isMicroRhythmSupported = (currentMicroRhythm > 0.3f * avgMicroRhythm);
        bool isLocalMaximum = (currentOnset > onsetBuffer[i-2] && currentOnset > onsetBuffer[i+2]);
        
        if (isOnsetPeak && (isMicroRhythmSupported || isLocalMaximum)) {
            peakIndices.push_back(i);
        }
    }
    
    // Calculate intervals between peaks
    for (size_t i = 1; i < peakIndices.size(); ++i) {
        float interval = static_cast<float>(peakIndices[i] - peakIndices[i-1]);
        intervals.push_back(interval);
    }
    
    if (!intervals.empty() && intervals.size() >= 3) {
        // Debug: Show intervals occasionally
        static int debugCounter = 0;
        debugCounter++;
        if (debugCounter % 50 == 0) {
            DEBUG_PRINT_STREAM("🎯 Filter-based: " << peakIndices.size() << " peaks, " << intervals.size() << " intervals");
        }
        
        // Use median with additional filtering for better tempo estimation
        std::vector<float> sortedIntervals = intervals;
        std::sort(sortedIntervals.begin(), sortedIntervals.end());
        float medianInterval = sortedIntervals[sortedIntervals.size() / 2];
        
        // Filter out unrealistic intervals with beat filter validation
        if (medianInterval < 3.0f) {
            return; // Skip this tempo estimate - likely noise
        }
        
        // Beat filter provides low-frequency validation for tempo stability
        float beatConsistency = 1.0f;
        if (beatBuffer.size() >= 10) {
            float recentBeatAvg = 0.0f;
            for (size_t i = beatBuffer.size() - 10; i < beatBuffer.size(); ++i) {
                recentBeatAvg += beatBuffer[i];
            }
            recentBeatAvg /= 10.0f;
            beatConsistency = std::clamp(recentBeatAvg * 2.0f, 0.3f, 1.0f);
        }
        
        // Improved tempo calculation with filter-based frame rate estimation
        float framesPerSecond = 12.0f + 8.0f * avgMicroRhythm; // Adaptive rate based on activity
        float timePerFrame = 1.0f / framesPerSecond;
        float estimatedTempo = 60.0f / (medianInterval * timePerFrame);
        
        // Tempo validation using beat filter
        estimatedTempo *= beatConsistency;
        
        // Handle tempo doubling/halving detection with filter context
        std::vector<float> candidates = {estimatedTempo, estimatedTempo * 2.0f, estimatedTempo * 0.5f};
        float bestTempo = estimatedTempo;
        float minDistance = std::abs(estimatedTempo - currentTempo);
        
        for (float candidate : candidates) {
            if (candidate >= 60.0f && candidate <= 300.0f) {
                float distance = std::abs(candidate - currentTempo);
                if (distance < minDistance) {
                    minDistance = distance;
                    bestTempo = candidate;
                }
            }
        }
        
        // Filter-based confidence calculation
        float confidence = beatConsistency / (1.0f + minDistance / 20.0f);
        float adaptiveSmoothingFactor = tempoSmoothingFactor * (1.0f + confidence * 4.0f);
        
        // Smooth tempo estimate
        currentTempo += adaptiveSmoothingFactor * (bestTempo - currentTempo);
        currentTempo = std::clamp(currentTempo, 60.0f, 300.0f);
    }
    
    // Update tempo history
    tempoHistory.erase(tempoHistory.begin());
    tempoHistory.push_back(currentTempo);
}

void RhythmDetector::calculateComplexity() {
    // Calculate rhythmic complexity using filter outputs and tempo variance
    float tempoVariance = 0.0f;
    float avgTempo = std::accumulate(tempoHistory.begin(), tempoHistory.end(), 0.0f) / tempoHistory.size();
    
    for (float tempo : tempoHistory) {
        float diff = tempo - avgTempo;
        tempoVariance += diff * diff;
    }
    tempoVariance /= tempoHistory.size();
    
    // Add micro-rhythm variation for enhanced complexity measure
    float microRhythmVariance = 0.0f;
    if (microRhythmBuffer.size() > 1) {
        float avgMicroRhythm = std::accumulate(microRhythmBuffer.begin(), microRhythmBuffer.end(), 0.0f) / microRhythmBuffer.size();
        for (float microRhythm : microRhythmBuffer) {
            float diff = microRhythm - avgMicroRhythm;
            microRhythmVariance += diff * diff;
        }
        microRhythmVariance /= microRhythmBuffer.size();
    }
    
    // Combine tempo and micro-rhythm variance for complexity
    float combinedComplexity = 0.7f * (tempoVariance / 100.0f) + 0.3f * (microRhythmVariance * 10.0f);
    rhythmicComplexity = std::clamp(combinedComplexity, 0.0f, 1.0f);
}

bool RhythmDetector::isBeatDetected() const {
    return beatStrength > 0.3f;
}

float RhythmDetector::getGrooveStrength() const {
    // Combine beat strength and complexity for groove measure
    return beatStrength * (1.0f - 0.5f * rhythmicComplexity);
}

std::vector<float> RhythmDetector::getRhythmPattern() const {
    // Return recent onset pattern for visualization
    return onsetBuffer;
}

void RhythmDetector::reset() {
    std::fill(microRhythmBuffer.begin(), microRhythmBuffer.end(), 0.0f);
    std::fill(beatBuffer.begin(), beatBuffer.end(), 0.0f);
    std::fill(onsetBuffer.begin(), onsetBuffer.end(), 0.0f);
    std::fill(tempoHistory.begin(), tempoHistory.end(), 120.0f);
    currentTempo = 120.0f;
    beatStrength = 0.0f;
    rhythmicComplexity = 0.0f;
}

// ============================================================================
// ConnectionMatrix Implementation
// ============================================================================

ConnectionMatrix::ConnectionMatrix(size_t filters, size_t neurons, float lr)
    : numFilters(filters), numNeurons(neurons), learningRate(lr), adaptiveMode(false) {
    
    weights.resize(numFilters);
    for (auto& row : weights) {
        row.resize(numNeurons, 0.0f);
    }
}

void ConnectionMatrix::setWeight(size_t filterIndex, size_t neuronIndex, float weight) {
    if (filterIndex < numFilters && neuronIndex < numNeurons) {
        weights[filterIndex][neuronIndex] = weight;
    }
}

float ConnectionMatrix::getWeight(size_t filterIndex, size_t neuronIndex) const {
    if (filterIndex < numFilters && neuronIndex < numNeurons) {
        return weights[filterIndex][neuronIndex];
    }
    return 0.0f;
}

std::vector<float> ConnectionMatrix::transform(const std::vector<float>& filterOutputs, float rhythmogramScale) const {
    std::vector<float> neuronInputs(numNeurons, 0.0f);
    
    // Scale rhythmogram outputs to meaningful neural activation levels
    // Typical rhythmogram values: 0.001-0.01, Neural threshold: 1.0
    // User-configurable scaling factor (0.0-20.0, default 5.0) ensures rhythmogram can trigger neural activation
    
    for (size_t f = 0; f < std::min(filterOutputs.size(), numFilters); ++f) {
        for (size_t n = 0; n < numNeurons; ++n) {
            // Apply connection weight and rhythmogram scaling for meaningful neural input
            // Note: Zero weights (from disabled toggles) will naturally produce zero input
            float scaledInput = weights[f][n] * filterOutputs[f] * rhythmogramScale;
            neuronInputs[n] += scaledInput;
        }
    }
    
    return neuronInputs;
}

void ConnectionMatrix::adaptWeights(const std::vector<float>& filterOutputs, 
                                  const std::vector<float>& neuronFeedback) {
    if (!adaptiveMode) return;
    
    // Simple Hebbian learning: strengthen connections when both sides are active
    for (size_t f = 0; f < std::min(filterOutputs.size(), numFilters); ++f) {
        for (size_t n = 0; n < std::min(neuronFeedback.size(), numNeurons); ++n) {
            float deltaWeight = learningRate * filterOutputs[f] * neuronFeedback[n];
            weights[f][n] += deltaWeight;
            weights[f][n] = std::clamp(weights[f][n], -1.0f, 1.0f);
        }
    }
}

void ConnectionMatrix::randomizeWeights(float minWeight, float maxWeight) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(minWeight, maxWeight);
    
    for (auto& row : weights) {
        for (float& weight : row) {
            weight = dist(gen);
        }
    }
}

void ConnectionMatrix::clearWeights() {
    for (auto& row : weights) {
        std::fill(row.begin(), row.end(), 0.0f);
    }
}

void ConnectionMatrix::resizeMatrix(size_t newNumNeurons) {
    numNeurons = newNumNeurons;
    
    // Resize all rows to accommodate new number of neurons
    for (auto& row : weights) {
        row.resize(numNeurons, 0.0f); // New connections start with 0 weight
    }
}

// ============================================================================
// RhythmInterpreter Implementation
// ============================================================================

RhythmInterpreter::RhythmInterpreter(NeuronNetwork* network, AudioManager* audioMgr, 
                                   size_t sampleRate, size_t bufferSize)
    : neuronNetwork(network), audioManager(audioMgr), 
      sampleRate(sampleRate), bufferSize(bufferSize),
      enabled(true), globalGain(0.0f), // Set to 0 for analysis-only mode
      rhythmogramScale(5.0f), // Default rhythmogram scale of 5.0
      bpm(120.0f), // Default BPM of 120
      autodetectTempo(true), // Default autodetect ON for better user experience
      useBeatRoot(false), // BeatRoot disabled by default
      beatRootSensitivity(1.0f) { // Default sensitivity
    
    initializeFilterBank();
    updateFilterBankForBPM(); // Apply initial BPM scaling to filters
    rhythmDetector = std::make_unique<RhythmDetector>();
    beatRoot = std::make_unique<BeatRoot>(neuronNetwork); // Initialize BeatRoot
    
    // Initialize Lightweight Auditory Processing Pipeline (optimized biological model)
    initializeLightweightAuditoryPipeline();
    
    if (neuronNetwork) {
        size_t numNeurons = neuronNetwork->getNeurons().size();
        connectionMatrix = std::make_unique<ConnectionMatrix>(filterBank.size(), numNeurons);
    }
    
    audioBuffer.resize(bufferSize);
    filterOutputs.resize(filterBank.size());
    filterGains.resize(filterBank.size(), 1.0f); // Initialize all filter gains to 1.0
    
    // Massive boost for high-frequency filter gains to ensure visibility
    for (size_t i = 5; i < filterGains.size(); ++i) {
        filterGains[i] = 1000.0f; // 1000x boost for high-frequency filters (5, 6, 7)
    }
    filterSoloEnabled.resize(filterBank.size(), false); // Initialize all solo states to false
    anyFilterSoloed = false; // No filters soloed initially
    audioOutputEnabled = true; // Audio output ENABLED by default for debugging
    processedAudioBuffer.resize(bufferSize); // Initialize processed audio buffer
    if (neuronNetwork) {
        neuronInputs.resize(neuronNetwork->getNeurons().size());
    }
}

RhythmInterpreter::~RhythmInterpreter() {
    // Default destructor implementation - required to be in .cpp file
    // for unique_ptr<BeatRoot> to work with forward declaration
}

void RhythmInterpreter::initializeFilterBank() {
    filterBank.clear();
    
    for (size_t i = 0; i < DEFAULT_FREQUENCIES.size(); ++i) {
        auto filter = std::make_unique<AdaptiveFilter>(
            DEFAULT_FREQUENCIES[i], 
            DEFAULT_BANDWIDTHS[i],
            0.1f, // Default adaptation rate
            DEFAULT_RESONANCES[i] // Default resonance
        );
        filterBank.push_back(std::move(filter));
    }
}

void RhythmInterpreter::initializeAuditoryPipeline() {
    // Initialize Outer Ear Filter (high-pass around 1kHz, models ear canal resonance)
    outerEarFilter = std::make_unique<OuterEarFilter>(static_cast<float>(sampleRate));
    
    // Initialize Gammatone Filter Bank (20Hz - 8kHz, 64 channels)
    const int numChannels = 64;
    const float minFreq = 20.0f;
    const float maxFreq = 8000.0f;
    gammatoneBank = std::make_unique<GammatoneFilterBank>(minFreq, maxFreq, numChannels, static_cast<float>(sampleRate));
    
    // Initialize Inner Hair Cells (one per gammatone channel)
    innerHairCells.clear();
    innerHairCells.resize(numChannels);
    for (int i = 0; i < numChannels; ++i) {
        innerHairCells[i] = std::make_unique<InnerHairCell>(1000.0f, static_cast<float>(sampleRate));
    }
    
    // Initialize Multi-Scale Filter (rhythmic analysis across multiple time scales)
    multiScaleFilter = std::make_unique<MultiScaleFilter>(DEFAULT_FREQUENCIES, static_cast<float>(sampleRate));
    
    // Initialize Peak Detector (find rhythmic peaks in multi-scale output)
    peakDetector = std::make_unique<RhythmPeakDetector>(static_cast<float>(sampleRate));
}

void RhythmInterpreter::initializeLightweightAuditoryPipeline() {
    // Initialize only the outer ear filter for lightweight processing
    // This provides some biological enhancement without the computational cost
    outerEarFilter = std::make_unique<OuterEarFilter>(static_cast<float>(sampleRate));
}

std::vector<float> RhythmInterpreter::processAuditoryPipeline(const std::vector<float>& audioData) {
    // Step 1: Outer Ear Filtering (models ear canal resonance and filtering)
    std::vector<float> outerEarOutput = outerEarFilter->process(audioData);
    
    // Step 2: Gammatone Filter Bank (cochlear frequency decomposition)
    std::vector<std::vector<float>> gammatoneOutputs = gammatoneBank->process(outerEarOutput);
    
    // Step 3: Inner Hair Cell Processing (transduction and compression)
    std::vector<std::vector<float>> hairCellOutputs(gammatoneOutputs.size());
    for (size_t channel = 0; channel < gammatoneOutputs.size(); ++channel) {
        hairCellOutputs[channel] = innerHairCells[channel]->process(gammatoneOutputs[channel]);
    }
    
    // Step 4: Signal Sum (combine all cochlear channels into unified signal)
    std::vector<float> combinedSignal(audioData.size(), 0.0f);
    for (const auto& channelOutput : hairCellOutputs) {
        for (size_t i = 0; i < combinedSignal.size() && i < channelOutput.size(); ++i) {
            combinedSignal[i] += channelOutput[i];
        }
    }
    
    // Normalize combined signal
    if (!combinedSignal.empty()) {
        float maxVal = *std::max_element(combinedSignal.begin(), combinedSignal.end(),
            [](float a, float b) { return std::abs(a) < std::abs(b); });
        if (maxVal > 0.0f) {
            float scale = 1.0f / std::abs(maxVal);
            for (float& sample : combinedSignal) {
                sample *= scale;
            }
        }
    }
    
    // Step 5: Multi-Scale Filtering (rhythmic analysis across time scales)
    std::vector<float> multiScaleOutput = multiScaleFilter->process(combinedSignal);
    
    // Step 6: Peak Detection and Summation (extract rhythmic features)
    std::vector<float> rhythmFeatures = peakDetector->process(multiScaleOutput);
    
    return rhythmFeatures;
}

void RhythmInterpreter::processAudioFrame(const std::vector<float>& audioData) {
    static int debugCounter = 0;
    
    // Debug: Always log first few calls and periodically after
    if (debugCounter < 5 || debugCounter % 1000 == 0) {
        DEBUG_PRINT_STREAM("🎛️  processAudioFrame called #" << debugCounter 
                  << " - enabled: " << enabled 
                  << ", neuronNetwork: " << (neuronNetwork ? "yes" : "no")
                  << ", audioData size: " << audioData.size());
    }
    
    if (!enabled || !neuronNetwork) {
        DEBUG_PRINT_STREAM("⚠️  processAudioFrame early return - enabled: " << enabled 
                  << ", neuronNetwork: " << (neuronNetwork ? "valid" : "null"));
        return;
    }
    
    // Debug: Check if we're receiving audio data
    debugCounter++;
    if (debugCounter % 50 == 0 && !audioData.empty()) { // More frequent logging
        float maxSample = 0.0f;
        for (float sample : audioData) {
            maxSample = std::max(maxSample, std::abs(sample));
        }
        DEBUG_PRINT_STREAM("📢 Audio input: " << audioData.size() << " samples, max: " << maxSample);
        
        // Always show high-frequency filter status when we have audio
        if (maxSample > 0.0001f) { // Only when there's actual audio
            for (size_t i = 5; i < filterBank.size() && i < filterOutputs.size(); ++i) {
                DEBUG_PRINT_STREAM("🔍 HF Filter " << i << " (" << DEFAULT_FREQUENCIES[i] << "Hz) output: " << filterOutputs[i]);
            }
        }
    }
    
    // Process audio through rhythm analysis systems
    if (useBeatRoot && beatRoot) {
        // Use BeatRoot for advanced beat tracking (simple RhythmDetector bypassed to save resources)
        static int beatRootDebugCounter = 0;
        beatRootDebugCounter++;
        if (beatRootDebugCounter % 100 == 0) {
            DEBUG_PRINT("🎯 BeatRoot ENABLED - Simple rhythm detector bypassed to save resources");
        }
        
        float deltaTime = audioData.size() / static_cast<float>(sampleRate);
        beatRoot->processAudioFrame(audioData, deltaTime);
        
        // Update BPM from BeatRoot if autodetect is enabled
        if (autodetectTempo) {
            float detectedTempo = beatRoot->getCurrentTempo();
            bool isStable = beatRoot->hasStableTempo();
            float beatStrength = beatRoot->getBeatStrength();
            size_t numAgents = beatRoot->getNumActiveAgents();
            
            DEBUG_PRINT_STREAM("🎯 AutodetectTempo ENABLED - checking BeatRoot tempo updates");
            
            static int tempoDebugCounter = 0;
            tempoDebugCounter++;
            if (tempoDebugCounter % 20 == 0) {  // More frequent updates
                DEBUG_PRINT_STREAM("🎯 BeatRoot: detected=" << detectedTempo << " BPM, strength=" 
                          << beatStrength << ", agents=" << numAgents 
                          << ", stable=" << (isStable ? "YES" : "NO")
                          << ", current_bpm=" << bpm);
            }
            
            if (detectedTempo >= 30.0f && detectedTempo <= 300.0f) {
                if (isStable) {
                    if (std::abs(detectedTempo - bpm) > 1.0f) {
                        ESSENTIAL_PRINT_STREAM("🎯 BeatRoot updating BPM: " << bpm << " → " << detectedTempo);
                        bpm = detectedTempo;
                        updateFilterBankForBPM();
                    }
                } else {
                    DEBUG_PRINT_STREAM("🎯 BeatRoot: Tempo not stable yet - not updating BPM");
                }
            } else {
                DEBUG_PRINT_STREAM("🎯 BeatRoot: Tempo out of range (" << detectedTempo << ") - not updating BPM");
            }
        } else {
            DEBUG_PRINT_STREAM("🎯 AutodetectTempo DISABLED - using simple rhythm detector");
        }
    } else {
        // Use simple rhythm detector (BeatRoot bypassed to save resources)
        static int bypassDebugCounter = 0;
        bypassDebugCounter++;
        if (bypassDebugCounter % 100 == 0) {
            DEBUG_PRINT("🎵 BeatRoot DISABLED - Using simple rhythm detector to save resources");
        }
        
        // Only process rhythm detector when BeatRoot is not active (mutual exclusivity)
        if (!useBeatRoot && rhythmDetector) {
            // Use filter outputs for rhythmogram-based tempo detection
            float onsetOutput = (filterOutputs.size() > 7) ? filterOutputs[7] : 0.0f;     // 16Hz - onset detection
            float microRhythmOutput = (filterOutputs.size() > 6) ? filterOutputs[6] : 0.0f; // 8Hz - micro-rhythm
            float beatOutput = (filterOutputs.size() > 2) ? filterOutputs[2] : 0.0f;      // 1Hz - beat
            
            rhythmDetector->processFilterOutputs(onsetOutput, microRhythmOutput, beatOutput);
        }
        
        // Update BPM from rhythm detector if autodetect is enabled (only when BeatRoot is not active)
        if (autodetectTempo && !useBeatRoot && rhythmDetector) {
            float detectedTempo = rhythmDetector->getCurrentTempo();
            
            static int tempoDebugCounter = 0;
            tempoDebugCounter++;
            if (tempoDebugCounter % 20 == 0) {  // More frequent debug output
                DEBUG_PRINT_STREAM("🎵 Autodetect: detected=" << detectedTempo << " BPM, current=" << bpm << " BPM");
            }
            
            if (detectedTempo >= 30.0f && detectedTempo <= 300.0f) { // Expanded valid BPM range
                // Only update if the detected tempo is significantly different to avoid jitter
                if (std::abs(detectedTempo - bpm) > 0.5f) {
                    ESSENTIAL_PRINT_STREAM("🎵 Updating BPM: " << bpm << " → " << detectedTempo);
                    bpm = detectedTempo;
                    updateFilterBankForBPM(); // Update filter frequencies when BPM changes
                }
            } else {
                if (tempoDebugCounter % 20 == 0) {  // More frequent debug output
                    DEBUG_PRINT_STREAM("🎵 Rejected tempo " << detectedTempo << " BPM (out of range 30-300)");
                }
            }
        }
    }
    
    // ============================================================================
    // LIGHTWEIGHT BIOLOGICAL AUDITORY PROCESSING (TEMPORARILY DISABLED FOR DEBUGGING)
    // Outer ear filtering for biological enhancement without computational overhead
    // ============================================================================
    
    std::vector<float> processedAudio = audioData;
    // Temporarily disable outer ear filter to debug high-frequency issue
    // if (outerEarFilter) {
    //     processedAudio = outerEarFilter->process(audioData);
    // }
    
    // Process audio through filterbank and generate output
    std::fill(filterOutputs.begin(), filterOutputs.end(), 0.0f);
    processedAudioBuffer.clear();
    processedAudioBuffer.resize(audioData.size(), 0.0f);
    
    // Temporary buffers to store per-filter processed audio
    std::vector<std::vector<float>> filterAudioOutputs(filterBank.size());
    for (size_t i = 0; i < filterBank.size(); ++i) {
        filterAudioOutputs[i].resize(audioData.size());
    }
    
    // Process each filter
    for (size_t i = 0; i < filterBank.size() && i < filterOutputs.size(); ++i) {
        float sum = 0.0f;
        float maxFilteredSample = 0.0f;
        float lastFilteredSample = 0.0f; // Keep track of the last sample for envelope filters
        
        for (size_t j = 0; j < audioData.size(); ++j) {
            // Use original audioData directly to debug high-frequency issue
            float filteredSample = filterBank[i]->process(audioData[j]);
            maxFilteredSample = std::max(maxFilteredSample, std::abs(filteredSample));
            filterAudioOutputs[i][j] = filteredSample * globalGain * filterGains[i];
            sum += filteredSample;
            lastFilteredSample = filteredSample; // Update last sample
        }
        
        // For GUI display, use different approaches based on filter type:
        if (i >= 5) { // High-frequency filters (5, 6, 7) use envelope detection
            // Use the last processed sample which contains the current envelope state
            filterOutputs[i] = lastFilteredSample * filterGains[i];
        } else {
            // Low-frequency filters use average (rhythmogram correlation)
            filterOutputs[i] = (sum / audioData.size()) * filterGains[i];
        }
        
        // Debug: Log filter output levels frequently for high-frequency filters
        if (debugCounter % 50 == 0 && i >= 5) { // Very frequent logging for high-frequency filters
            float maxAudio = audioData.empty() ? 0.0f : *std::max_element(audioData.begin(), audioData.end(),
                [](float a, float b) { return std::abs(a) < std::abs(b); });
            ESSENTIAL_PRINT_STREAM("🔥 HF Filter " << i << " (" << DEFAULT_FREQUENCIES[i] << "Hz)"
                      << " - Input: " << maxAudio
                      << ", Raw: " << lastFilteredSample 
                      << ", FINAL: " << filterOutputs[i] << ", Gain: " << filterGains[i]);
        }
        
        // Adapt filter based on rhythm strength
        float rhythmStrength;
        if (useBeatRoot && beatRoot) {
            rhythmStrength = beatRoot->getBeatStrength();
        } else {
            rhythmStrength = rhythmDetector ? rhythmDetector->getBeatStrength() : 0.0f;
        }
        filterBank[i]->adaptToRhythm(rhythmStrength);
    }
    
    // Generate processed audio output based on solo state
    if (anyFilterSoloed) {
        // Only output soloed filters
        for (size_t i = 0; i < filterBank.size() && i < filterSoloEnabled.size(); ++i) {
            if (filterSoloEnabled[i]) {
                for (size_t j = 0; j < audioData.size(); ++j) {
                    processedAudioBuffer[j] += filterAudioOutputs[i][j];
                }
            }
        }
        
        // Debug: Output when filters are soloed
        if (debugCounter % 100 == 0) {
            int soloCount = 0;
            for (bool solo : filterSoloEnabled) if (solo) soloCount++;
            DEBUG_PRINT_STREAM("🎛️  Filter solo active: " << soloCount << " bands soloed");
        }
    } else {
        // Output original audio (pass-through when no filters are soloed)
        processedAudioBuffer = audioData;
        
        // Debug: Output for pass-through
        if (debugCounter % 100 == 0 && !audioData.empty()) {
            DEBUG_PRINT_STREAM("🔄 Pass-through mode: " << audioData.size() << " samples");
        }
    }
    
    // Debugging: Log audio processing details every 1000 frames to reduce spam
    if (debugCounter % 1000 == 0) {
        DEBUG_PRINT_STREAM("🎛️  Global Gain: " << globalGain << ", Filter Gains: [" <<
            (filterGains.size() > 0 ? std::to_string(filterGains[0]) : "") <<
            (filterGains.size() > 1 ? ", " + std::to_string(filterGains[1]) : "") <<
            (filterGains.size() > 2 ? ", " + std::to_string(filterGains[2]) : "") <<
            (filterGains.size() > 3 ? "..." : "") << "]");
        
        float maxOutput = 0.0f;
        for (float sample : processedAudioBuffer) {
            maxOutput = std::max(maxOutput, std::abs(sample));
        }
        DEBUG_PRINT_STREAM("🎛️  Processed Audio Buffer Size: " << processedAudioBuffer.size() 
                  << ", Max Level: " << maxOutput);
    }
}

void RhythmInterpreter::update() {
    if (!enabled || !neuronNetwork || !connectionMatrix) return;
    
    // Transform filter outputs to neuron inputs through connection matrix
    neuronInputs = connectionMatrix->transform(filterOutputs, rhythmogramScale);
    
    // Apply rhythmogram inputs to neurons based on matrix connections
    const auto& neurons = neuronNetwork->getNeurons();
    static int debugCounter = 0;
    bool hasSignificantInput = false;
    
    // Debug: Show connection matrix transform results occasionally
    if (debugCounter % 200 == 0) {
        DEBUG_PRINT_STREAM("🔄 Connection matrix transform: " << filterOutputs.size() 
                  << " filters → " << neuronInputs.size() << " neurons");
        
        // Show matrix weights for debugging
        if (g_debugMode) {
            bool foundAnyWeights = false;
            for (size_t f = 0; f < std::min(filterOutputs.size(), size_t(8)); ++f) {
                if (filterOutputs[f] > 0.00001f) {  // Only check filters with output
                    for (size_t n = 0; n < neuronInputs.size(); ++n) {
                        float weight = connectionMatrix->getWeight(f, n);
                        if (weight > 0.001f) {
                            DEBUG_PRINT_STREAM("   Filter " << f << " → Neuron " << (n+1) 
                                          << ": output=" << filterOutputs[f] 
                                          << " * weight=" << weight 
                                          << " * scale=500" 
                                          << " = " << (filterOutputs[f] * weight * 500.0f));
                            foundAnyWeights = true;
                        }
                    }
                }
            }
            if (!foundAnyWeights) {
                DEBUG_PRINT("   ❌ No connection weights found! Matrix may be uninitialized.");
                // Show first few filter outputs and all weights for first neuron
                for (size_t f = 0; f < std::min(size_t(3), filterOutputs.size()); ++f) {
                    float weight0 = (neuronInputs.size() > 0) ? connectionMatrix->getWeight(f, 0) : 0.0f;
                    DEBUG_PRINT_STREAM("      Filter " << f << ": output=" << filterOutputs[f] << " weight[0]=" << weight0);
                }
            }
            
            for (size_t i = 0; i < std::min(size_t(3), neuronInputs.size()); ++i) {
                DEBUG_PRINT_STREAM("   Neuron " << (i+1) << " input: " << neuronInputs[i]);
            }
        }
    }
    
    for (size_t i = 0; i < std::min(neuronInputs.size(), neurons.size()); ++i) {
        // Send rhythmogram-derived input to each neuron based on connection matrix
        if (std::abs(neuronInputs[i]) > 0.005f) { // Apply inputs that can meaningfully affect neural activation
            neurons[i]->addExternalInput(neuronInputs[i]);
            hasSignificantInput = true;
            
            // Debug: Log neuron input occasionally
            if (++debugCounter % 50 == 0) {
                std::cout << "🎯 Neuron " << (i+1) << " receiving scaled rhythmogram input: " 
                         << neuronInputs[i] << " (activation: " << neurons[i]->getActivation() 
                         << ", threshold: " << neurons[i]->getThreshold() << ")" << std::endl;
            }
        }
    }
    
    // Debug: Show when rhythmogram is actively driving neurons
    if (hasSignificantInput && debugCounter % 200 == 0) {
        std::cout << "🎵 Rhythmogram → Neurons: Active connections driving neural activation" << std::endl;
    }
    
    // Adaptive weight learning if enabled
    if (connectionMatrix->isAdaptiveMode()) {
        std::vector<float> neuronFeedback(neurons.size());
        for (size_t i = 0; i < neurons.size(); ++i) {
            neuronFeedback[i] = neurons[i]->getActivation();
        }
        connectionMatrix->adaptWeights(filterOutputs, neuronFeedback);
    }
}

// Configuration methods
void RhythmInterpreter::setFilterFrequency(size_t filterIndex, float frequency) {
    if (filterIndex < filterBank.size()) {
        filterBank[filterIndex]->setCenterFrequency(frequency);
    }
}

void RhythmInterpreter::setFilterBandwidth(size_t filterIndex, float bandwidth) {
    if (filterIndex < filterBank.size()) {
        filterBank[filterIndex]->setBandwidth(bandwidth);
    }
}

void RhythmInterpreter::setFilterAdaptation(size_t filterIndex, float rate) {
    if (filterIndex < filterBank.size()) {
        filterBank[filterIndex]->setAdaptationRate(rate);
    }
}

void RhythmInterpreter::setConnectionWeight(size_t filterIndex, size_t neuronIndex, float weight) {
    if (connectionMatrix) {
        connectionMatrix->setWeight(filterIndex, neuronIndex, weight);
    }
}

void RhythmInterpreter::enableAdaptiveConnections(bool enable) {
    if (connectionMatrix) {
        connectionMatrix->setAdaptiveMode(enable);
    }
}

void RhythmInterpreter::randomizeConnections() {
    if (connectionMatrix) {
        connectionMatrix->randomizeWeights();
    }
}

void RhythmInterpreter::updateNetworkSize() {
    if (connectionMatrix && neuronNetwork) {
        size_t currentNeurons = neuronNetwork->getNeurons().size();
        if (currentNeurons != connectionMatrix->getNumNeurons()) {
            connectionMatrix->resizeMatrix(currentNeurons);
            neuronInputs.resize(currentNeurons);
        }
    }
}

// Status methods - Use BeatRoot when active, fallback to simple RhythmDetector when disabled
bool RhythmInterpreter::isRhythmDetected() const {
    if (useBeatRoot && beatRoot) {
        return beatRoot->isBeatDetected();
    }
    return rhythmDetector && rhythmDetector->isBeatDetected();
}

float RhythmInterpreter::getCurrentTempo() const {
    if (useBeatRoot && beatRoot) {
        return beatRoot->getCurrentTempo();
    }
    return rhythmDetector ? rhythmDetector->getCurrentTempo() : 120.0f;
}

float RhythmInterpreter::getOverallRhythmStrength() const {
    if (useBeatRoot && beatRoot) {
        return beatRoot->getBeatStrength();
    }
    return rhythmDetector ? rhythmDetector->getGrooveStrength() : 0.0f;
}

void RhythmInterpreter::setFilterGain(size_t filterIndex, float gain) {
    if (filterIndex < filterGains.size()) {
        filterGains[filterIndex] = std::max(0.0f, std::min(5.0f, gain)); // Clamp between 0.0 and 5.0
    }
}

float RhythmInterpreter::getFilterGain(size_t filterIndex) const {
    if (filterIndex < filterGains.size()) {
        return filterGains[filterIndex];
    }
    return 1.0f; // Default gain
}

void RhythmInterpreter::setFilterResonance(size_t filterIndex, float resonance) {
    if (filterIndex < filterBank.size()) {
        filterBank[filterIndex]->setResonance(resonance);
    }
}

float RhythmInterpreter::getFilterResonance(size_t filterIndex) const {
    if (filterIndex < filterBank.size()) {
        return filterBank[filterIndex]->getResonance();
    }
    return 1.0f; // Default resonance
}

void RhythmInterpreter::setRhythmogramScale(float scale) {
    rhythmogramScale = std::max(0.0f, std::min(20.0f, scale)); // Clamp between 0.0 and 20.0
}

void RhythmInterpreter::setBPM(float beatsPerMinute) {
    // Only allow manual BPM setting if autodetect is disabled
    if (!autodetectTempo) {
        bpm = std::max(30.0f, std::min(260.0f, beatsPerMinute)); // Clamp between 30.0 and 260.0 BPM
        updateFilterBankForBPM(); // Update filter frequencies when BPM changes
    }
}

void RhythmInterpreter::setAutodetectTempo(bool enable) {
    autodetectTempo = enable;
    std::cout << "🎵 Autodetect Tempo: " << (enable ? "ON" : "OFF") << std::endl;
}

void RhythmInterpreter::updateFilterBankForBPM() {
    if (filterBank.empty()) return;
    
    // Calculate tempo scaling factor relative to default 120 BPM
    // At 120 BPM: quarter note = 1.0 Hz, so scaling factor = bpm/120
    float tempoScale = bpm / 120.0f;
    
    ESSENTIAL_PRINT_STREAM("🎵 BPM changed to " << bpm << " (scale factor: " << tempoScale << "x)");
    
    // Scale all Todd frequencies proportionally to tempo
    // This keeps the rhythmic hierarchy relative to the current tempo
    for (size_t i = 0; i < filterBank.size() && i < DEFAULT_FREQUENCIES.size(); ++i) {
        float scaledFrequency = DEFAULT_FREQUENCIES[i] * tempoScale;
        float scaledBandwidth = DEFAULT_BANDWIDTHS[i] * tempoScale; // Bandwidth scales with frequency
        float resonance = DEFAULT_RESONANCES[i]; // Resonance (Q factor) remains constant
        
        filterBank[i]->setCenterFrequency(scaledFrequency);
        filterBank[i]->setBandwidth(scaledBandwidth);
        filterBank[i]->setResonance(resonance);
        
        // Debug output for key frequencies
        if (i == 3) { // Quarter note level
            std::cout << "🎵   Quarter note freq: " << DEFAULT_FREQUENCIES[i] << "Hz → " << scaledFrequency << "Hz" << std::endl;
        }
    }
}

void RhythmInterpreter::setFilterSolo(size_t filterIndex, bool solo) {
    if (filterIndex < filterSoloEnabled.size()) {
        filterSoloEnabled[filterIndex] = solo;
        
        // Update anyFilterSoloed flag
        anyFilterSoloed = false;
        for (bool soloed : filterSoloEnabled) {
            if (soloed) {
                anyFilterSoloed = true;
                break;
            }
        }
    }
}

bool RhythmInterpreter::getFilterSolo(size_t filterIndex) const {
    if (filterIndex < filterSoloEnabled.size()) {
        return filterSoloEnabled[filterIndex];
    }
    return false;
}

void RhythmInterpreter::clearAllSolos() {
    std::fill(filterSoloEnabled.begin(), filterSoloEnabled.end(), false);
    anyFilterSoloed = false;
}

std::vector<float> RhythmInterpreter::getSoloedFilterOutput() const {
    std::vector<float> output(filterOutputs.size(), 0.0f);
    
    if (!anyFilterSoloed) {
        // No filters soloed, return all filter outputs
        return filterOutputs;
    }
    
    // Return only soloed filter outputs
    for (size_t i = 0; i < filterOutputs.size() && i < filterSoloEnabled.size(); ++i) {
        if (filterSoloEnabled[i]) {
            output[i] = filterOutputs[i];
        }
    }
    
    return output;
}

std::vector<float> RhythmInterpreter::getProcessedAudioOutput() const {
    return processedAudioBuffer;
}

// ============================================================================
// BeatRoot Integration Methods
// ============================================================================

void RhythmInterpreter::setUseBeatRoot(bool enable) {
    useBeatRoot = enable;
    if (beatRoot) {
        beatRoot->setEnabled(enable);
    }
    std::cout << "🎯 BeatRoot system: " << (enable ? "ENABLED" : "DISABLED") << std::endl;
}

void RhythmInterpreter::setBeatRootSensitivity(float sensitivity) {
    beatRootSensitivity = std::clamp(sensitivity, 0.1f, 2.0f);
    if (useBeatRoot && beatRoot) {
        beatRoot->setSensitivity(beatRootSensitivity);
    }
    DEBUG_PRINT_STREAM("🎯 BeatRoot sensitivity: " << beatRootSensitivity);
}

void RhythmInterpreter::initializeBeatRoot(float tempo) {
    if (!useBeatRoot || !beatRoot) {
        ESSENTIAL_PRINT("🎯 BeatRoot initialization skipped (disabled)");
        return;
    }
    
    if (tempo > 0.0f) {
        // Initialize with specific tempo
        beatRoot->initialize(tempo);
        ESSENTIAL_PRINT_STREAM("🎯 BeatRoot manually initialized with tempo: " << tempo << " BPM");
    } else {
        // Enable auto-initialization
        beatRoot->setAutoInitialize(true);
        ESSENTIAL_PRINT("🎯 BeatRoot auto-initialization enabled");
    }
}

void RhythmInterpreter::resetBeatRoot() {
    if (useBeatRoot && beatRoot) {
        beatRoot->reset();
        std::cout << "🎯 BeatRoot system reset" << std::endl;
    } else {
        std::cout << "🎯 BeatRoot reset skipped (disabled)" << std::endl;
    }
}

bool RhythmInterpreter::isBeatRootBeatDetected() const {
    return useBeatRoot && beatRoot && beatRoot->isBeatDetected();
}

float RhythmInterpreter::getBeatRootOnsetStrength() const {
    return (useBeatRoot && beatRoot) ? beatRoot->getOnsetStrength() : 0.0f;
}

size_t RhythmInterpreter::getBeatRootNumAgents() const {
    return (useBeatRoot && beatRoot) ? beatRoot->getNumActiveAgents() : 0;
}

bool RhythmInterpreter::hasBeatRootStableTempo() const {
    return useBeatRoot && beatRoot && beatRoot->hasStableTempo();
}

float RhythmInterpreter::getBeatRootCurrentTempo() const {
    return (useBeatRoot && beatRoot) ? beatRoot->getCurrentTempo() : 120.0f;
}

// BeatRoot advanced parameter control
void RhythmInterpreter::setBeatRootOnsetThreshold(float threshold) {
    if (beatRoot) {
        beatRoot->setOnsetThreshold(threshold);
    }
}

float RhythmInterpreter::getBeatRootOnsetThreshold() const {
    return beatRoot ? beatRoot->getOnsetThreshold() : 0.3f;
}

void RhythmInterpreter::setBeatRootBeatTolerance(float tolerance) {
    if (beatRoot) {
        beatRoot->setBeatTolerance(tolerance);
    }
}

float RhythmInterpreter::getBeatRootBeatTolerance() const {
    return beatRoot ? beatRoot->getBeatTolerance() : 0.1f;
}

void RhythmInterpreter::setBeatRootMaxAgents(size_t maxAgents) {
    if (beatRoot) {
        beatRoot->setMaxAgents(maxAgents);
    }
}

size_t RhythmInterpreter::getBeatRootMaxAgents() const {
    return beatRoot ? beatRoot->getMaxAgents() : 8;
}

void RhythmInterpreter::setBeatRootAgentSpawnThreshold(float threshold) {
    if (beatRoot) {
        beatRoot->setAgentSpawnThreshold(threshold);
    }
}

float RhythmInterpreter::getBeatRootAgentSpawnThreshold() const {
    return beatRoot ? beatRoot->getAgentSpawnThreshold() : 0.15f;
}

void RhythmInterpreter::setBeatRootAutoInitialize(bool enable) {
    if (beatRoot) {
        beatRoot->setAutoInitialize(enable);
    }
}

bool RhythmInterpreter::getBeatRootAutoInitialize() const {
    return beatRoot ? beatRoot->getAutoInitialize() : true;
}

// ============================================================================
// Biological Auditory Processing Implementations
// ============================================================================

// OuterEarFilter Implementation
OuterEarFilter::OuterEarFilter(float fs) : sampleRate(fs) {
    // Initialize high-pass filter coefficients (ear canal resonance ~1-4kHz)
    coefficients.resize(3); // Simple high-pass IIR
    delayLine.resize(2, 0.0f);
    
    // High-pass filter at 200Hz (models outer ear filtering)
    float fc = 200.0f / sampleRate;
    float alpha = 1.0f / (1.0f + 2.0f * M_PI * fc);
    coefficients[0] = alpha;      // b0
    coefficients[1] = -alpha;     // b1  
    coefficients[2] = alpha;      // a1
}

std::vector<float> OuterEarFilter::process(const std::vector<float>& input) {
    std::vector<float> output(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = processSample(input[i]);
    }
    return output;
}

float OuterEarFilter::processSample(float input) {
    // Simple high-pass filter
    float output = coefficients[0] * input + coefficients[1] * delayLine[0] - coefficients[2] * delayLine[1];
    delayLine[1] = delayLine[0];
    delayLine[0] = output;
    return output;
}

// GammatoneFilter Implementation  
GammatoneFilter::GammatoneFilter(float freq, float bw, float fs) 
    : centerFreq(freq), bandwidth(bw), sampleRate(fs), delayLine(4, std::complex<float>(0,0)) {
    
    // Calculate gammatone filter pole
    float erb = 24.7f * (4.37f * freq / 1000.0f + 1.0f); // ERB calculation
    float b = 1.019f * 2.0f * M_PI * erb;
    pole = std::complex<float>(-b, 2.0f * M_PI * freq);
}

float GammatoneFilter::process(float input) {
    // 4th order gammatone filter implementation
    std::complex<float> x(input, 0.0f);
    std::complex<float> y = x;
    
    // Apply 4 cascaded complex poles
    for (int i = 0; i < 4; ++i) {
        y = y + pole * delayLine[i] / sampleRate;
        delayLine[i] = y;
    }
    
    return y.real(); // Take real part as output
}

void GammatoneFilter::reset() {
    std::fill(delayLine.begin(), delayLine.end(), std::complex<float>(0,0));
}

// GammatoneFilterBank Implementation
GammatoneFilterBank::GammatoneFilterBank(float minF, float maxF, size_t numBands, float fs) 
    : minFreq(minF), maxFreq(maxF), numFilters(numBands) {
    
    // Create logarithmically spaced center frequencies
    centerFrequencies.resize(numBands);
    filters.resize(numBands);
    
    for (size_t i = 0; i < numBands; ++i) {
        float logMin = std::log(minF);
        float logMax = std::log(maxF);
        float logFreq = logMin + (logMax - logMin) * i / (numBands - 1);
        float centerFreq = std::exp(logFreq);
        centerFrequencies[i] = centerFreq;
        
        float bandwidth = centerFreq * 0.1f; // 10% bandwidth
        filters[i] = std::make_unique<GammatoneFilter>(centerFreq, bandwidth, fs);
    }
}

std::vector<std::vector<float>> GammatoneFilterBank::process(const std::vector<float>& input) {
    std::vector<std::vector<float>> outputs(numFilters);
    
    for (size_t i = 0; i < numFilters; ++i) {
        outputs[i].resize(input.size());
        for (size_t j = 0; j < input.size(); ++j) {
            outputs[i][j] = filters[i]->process(input[j]);
        }
    }
    
    return outputs;
}

std::vector<float> GammatoneFilterBank::processSample(float input) {
    std::vector<float> outputs(numFilters);
    for (size_t i = 0; i < numFilters; ++i) {
        outputs[i] = filters[i]->process(input);
    }
    return outputs;
}

// InnerHairCell Implementation
InnerHairCell::InnerHairCell(float cutoff, float fs) : cutoffFreq(cutoff), lowpassState(0.0f) {
    // Calculate low-pass filter coefficient
    alpha = 2.0f * M_PI * cutoffFreq / fs;
    if (alpha > 1.0f) alpha = 1.0f;
}

std::vector<float> InnerHairCell::process(const std::vector<float>& input) {
    std::vector<float> output(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = processSample(input[i]);
    }
    return output;
}

float InnerHairCell::processSample(float input) {
    // Half-wave rectification + low-pass filtering
    float rectified = std::max(0.0f, input);
    lowpassState += alpha * (rectified - lowpassState);
    return lowpassState;
}

void InnerHairCell::reset() {
    lowpassState = 0.0f;
}

// MultiScaleFilter Implementation
MultiScaleFilter::MultiScaleFilter(const std::vector<float>& timeScales, float fs) : scales(timeScales) {
    delayLines.resize(scales.size());
    outputs.resize(scales.size(), 0.0f);
    
    for (size_t i = 0; i < scales.size(); ++i) {
        size_t delaySize = static_cast<size_t>(fs / scales[i]);
        delayLines[i].resize(delaySize, 0.0f);
    }
}

std::vector<float> MultiScaleFilter::process(const std::vector<float>& input) {
    std::vector<float> combinedOutput(input.size(), 0.0f);
    
    for (size_t i = 0; i < input.size(); ++i) {
        std::vector<float> scaleOutputs = processSample(input[i]);
        for (float output : scaleOutputs) {
            combinedOutput[i] += output;
        }
    }
    
    return combinedOutput;
}

std::vector<float> MultiScaleFilter::processSample(float input) {
    std::vector<float> scaleOutputs(scales.size());
    
    for (size_t i = 0; i < scales.size(); ++i) {
        // Circular delay line
        auto& delay = delayLines[i];
        if (!delay.empty()) {
            outputs[i] = delay.back(); // Output delayed signal
            
            // Shift delay line
            for (size_t j = delay.size() - 1; j > 0; --j) {
                delay[j] = delay[j-1];
            }
            delay[0] = input; // Input new sample
        }
        scaleOutputs[i] = outputs[i];
    }
    
    return scaleOutputs;
}

// RhythmPeakDetector Implementation
RhythmPeakDetector::RhythmPeakDetector(float fs, size_t history) 
    : historySize(history), adaptiveThreshold(0.1f) {
    peakHistory.reserve(history);
    thresholds.resize(8, 0.05f); // Initialize thresholds for 8 scale bands
}

std::vector<float> RhythmPeakDetector::process(const std::vector<float>& multiScaleOutput) {
    return detectPeaks(multiScaleOutput);
}

std::vector<float> RhythmPeakDetector::detectPeaks(const std::vector<float>& multiScaleOutput) {
    std::vector<float> peaks(multiScaleOutput.size());
    
    for (size_t i = 0; i < multiScaleOutput.size(); ++i) {
        float sample = multiScaleOutput[i];
        
        // Simple peak detection: value above adaptive threshold
        if (sample > adaptiveThreshold) {
            peaks[i] = sample;
            
            // Add to peak history for threshold adaptation
            peakHistory.push_back(sample);
            if (peakHistory.size() > historySize) {
                peakHistory.erase(peakHistory.begin());
            }
            
            // Adapt threshold based on recent peak levels
            if (peakHistory.size() > 10) {
                float avgPeak = std::accumulate(peakHistory.begin(), peakHistory.end(), 0.0f) / peakHistory.size();
                adaptiveThreshold = 0.3f * avgPeak; // Threshold is 30% of average peak
            }
        } else {
            peaks[i] = 0.0f;
        }
    }
    
    return peaks;
}

float RhythmPeakDetector::getSummatedRhythm() const {
    if (peakHistory.empty()) return 0.0f;
    return std::accumulate(peakHistory.begin(), peakHistory.end(), 0.0f) / peakHistory.size();
}

void RhythmPeakDetector::adaptThresholds(const std::vector<float>& recentActivity) {
    // Adapt thresholds based on recent activity levels
    for (size_t i = 0; i < thresholds.size() && i < recentActivity.size(); ++i) {
        thresholds[i] = 0.5f * thresholds[i] + 0.5f * (0.3f * recentActivity[i]);
    }
}