#include "RhythmInterpreter.h"
#include "NeuronNetwork.h"
#include "AudioManager.h"
#include <algorithm>
#include <random>
#include <cmath>
#include <numeric>
#include <iostream>

// Default frequency bands with exponential distribution starting from 1.0 Hz
const std::vector<float> RhythmInterpreter::DEFAULT_FREQUENCIES = {
    1.0f,     // Ultra-low (subsonic/infrasonic rhythms)
    3.36f,    // Very low (very slow rhythmic patterns) 
    11.3f,    // Low (slow bass rhythms)
    38.0f,    // Sub bass (kick fundamentals)
    127.8f,   // Bass (kick harmonics, low snare)
    429.7f,   // Mids (snare snap, mid percussion)
    1445.7f,  // Presence (hi-hat attack, upper percussion)
    8000.0f   // Air (high frequency content, cymbals)
};

const std::vector<float> RhythmInterpreter::DEFAULT_BANDWIDTHS = {
    0.5f,     // Narrow for ultra-low (0.5 Hz around 1.0 Hz)
    1.2f,     // Narrow for very low (1.2 Hz around 3.36 Hz)
    4.0f,     // Medium for low (4.0 Hz around 11.3 Hz)
    13.0f,    // Medium for sub bass (13 Hz around 38 Hz)
    43.0f,    // Medium-wide for bass (43 Hz around 128 Hz)
    145.0f,   // Wide for mids (145 Hz around 430 Hz)
    480.0f,   // Wide for presence (480 Hz around 1446 Hz)
    2500.0f   // Very wide for air (2500 Hz around 8000 Hz)
};

const std::vector<float> RhythmInterpreter::DEFAULT_RESONANCES = {
    2.0f,     // Moderate resonance for ultra-low
    2.5f,     // Slightly higher for very low
    3.0f,     // Medium resonance for low
    4.0f,     // Higher resonance for sub bass (punch)
    5.0f,     // High resonance for bass (tight)
    3.5f,     // Medium-high for mids (clarity)
    2.5f,     // Lower for presence (smooth)
    1.5f      // Low resonance for air (gentle)
};

// ============================================================================
// AdaptiveFilter Implementation
// ============================================================================

AdaptiveFilter::AdaptiveFilter(float freq, float bw, float adaptRate, float res)
    : centerFrequency(freq), bandwidth(bw), adaptationRate(adaptRate), resonance(res),
      currentEnergy(0.0f), adaptiveGain(1.0f) {
    
    // Initialize simple bandpass filter coefficients
    coefficients.resize(5); // Biquad filter
    delayLine.resize(4, 0.0f); // z^-1 and z^-2 delays for input and output
    
    updateFilterCoefficients();
}

void AdaptiveFilter::updateFilterCoefficients() {
    // Calculate biquad coefficients for bandpass filter with resonance
    float omega = 2.0f * M_PI * centerFrequency / 44100.0f; // Assume 44.1kHz
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
    // Apply biquad filter: y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
    float output = coefficients[0] * input + 
                   coefficients[1] * delayLine[0] + 
                   coefficients[2] * delayLine[1] - 
                   coefficients[3] * delayLine[2] - 
                   coefficients[4] * delayLine[3];
    
    // Update delay line
    delayLine[1] = delayLine[0]; // x[n-2] = x[n-1]
    delayLine[0] = input;        // x[n-1] = x[n]
    delayLine[3] = delayLine[2]; // y[n-2] = y[n-1]
    delayLine[2] = output;       // y[n-1] = y[n]
    
    // Apply adaptive gain
    output *= adaptiveGain;
    
    // Update energy estimate
    currentEnergy = 0.95f * currentEnergy + 0.05f * (output * output);
    
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
    
    audioBuffer.resize(BUFFER_SIZE, 0.0f);
    onsetBuffer.resize(HISTORY_LENGTH, 0.0f);
    tempoHistory.resize(HISTORY_LENGTH, 120.0f);
}

void RhythmDetector::processAudioChunk(const std::vector<float>& audioData) {
    // Copy audio data to internal buffer
    size_t copySize = std::min(audioData.size(), BUFFER_SIZE);
    std::copy(audioData.begin(), audioData.begin() + copySize, audioBuffer.begin());
    
    detectOnsets();
    analyzeTempo();
    calculateComplexity();
}

void RhythmDetector::detectOnsets() {
    // Simple onset detection using energy and spectral flux
    float currentEnergy = 0.0f;
    float spectralFlux = 0.0f;
    
    // Calculate energy
    for (float sample : audioBuffer) {
        currentEnergy += sample * sample;
    }
    currentEnergy /= audioBuffer.size();
    
    // Simple spectral flux approximation using high frequency content
    for (size_t i = 1; i < audioBuffer.size(); ++i) {
        float diff = audioBuffer[i] - audioBuffer[i-1];
        spectralFlux += diff * diff;
    }
    spectralFlux /= audioBuffer.size();
    
    // Combine energy and flux for onset strength
    float onsetStrength = currentEnergy + 0.5f * spectralFlux;
    
    // Add to onset buffer
    onsetBuffer.erase(onsetBuffer.begin());
    onsetBuffer.push_back(onsetStrength);
    
    // Calculate beat strength as peak detection
    beatStrength = 0.0f;
    if (onsetStrength > onsetThreshold) {
        // Check if this is a local maximum
        bool isPeak = true;
        size_t checkRange = std::min(size_t(5), onsetBuffer.size() / 2);
        
        for (size_t i = onsetBuffer.size() - checkRange; i < onsetBuffer.size() - 1; ++i) {
            if (onsetBuffer[i] >= onsetStrength) {
                isPeak = false;
                break;
            }
        }
        
        if (isPeak) {
            beatStrength = std::clamp(onsetStrength, 0.0f, 1.0f);
        }
    }
}

void RhythmDetector::analyzeTempo() {
    // Simple tempo estimation using onset intervals
    std::vector<float> intervals;
    
    // Find peaks in onset buffer
    std::vector<size_t> peakIndices;
    for (size_t i = 1; i < onsetBuffer.size() - 1; ++i) {
        if (onsetBuffer[i] > onsetBuffer[i-1] && 
            onsetBuffer[i] > onsetBuffer[i+1] && 
            onsetBuffer[i] > onsetThreshold) {
            peakIndices.push_back(i);
        }
    }
    
    // Calculate intervals between peaks
    for (size_t i = 1; i < peakIndices.size(); ++i) {
        float interval = static_cast<float>(peakIndices[i] - peakIndices[i-1]);
        intervals.push_back(interval);
    }
    
    if (!intervals.empty()) {
        // Estimate tempo from average interval
        float avgInterval = std::accumulate(intervals.begin(), intervals.end(), 0.0f) / intervals.size();
        float estimatedTempo = 60.0f / (avgInterval * 0.01f); // Assuming 100Hz update rate
        
        // Smooth tempo estimate
        currentTempo += tempoSmoothingFactor * (estimatedTempo - currentTempo);
        currentTempo = std::clamp(currentTempo, 60.0f, 200.0f);
    }
    
    // Update tempo history
    tempoHistory.erase(tempoHistory.begin());
    tempoHistory.push_back(currentTempo);
}

void RhythmDetector::calculateComplexity() {
    // Calculate rhythmic complexity based on tempo variance and onset patterns
    float tempoVariance = 0.0f;
    float avgTempo = std::accumulate(tempoHistory.begin(), tempoHistory.end(), 0.0f) / tempoHistory.size();
    
    for (float tempo : tempoHistory) {
        float diff = tempo - avgTempo;
        tempoVariance += diff * diff;
    }
    tempoVariance /= tempoHistory.size();
    
    // Normalize complexity measure
    rhythmicComplexity = std::clamp(tempoVariance / 100.0f, 0.0f, 1.0f);
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
    std::fill(audioBuffer.begin(), audioBuffer.end(), 0.0f);
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

std::vector<float> ConnectionMatrix::transform(const std::vector<float>& filterOutputs) const {
    std::vector<float> neuronInputs(numNeurons, 0.0f);
    
    for (size_t f = 0; f < std::min(filterOutputs.size(), numFilters); ++f) {
        for (size_t n = 0; n < numNeurons; ++n) {
            neuronInputs[n] += weights[f][n] * filterOutputs[f];
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
      enabled(true), globalGain(0.0f) { // Set to 0 for analysis-only mode
    
    initializeFilterBank();
    rhythmDetector = std::make_unique<RhythmDetector>();
    
    if (neuronNetwork) {
        size_t numNeurons = neuronNetwork->getNeurons().size();
        connectionMatrix = std::make_unique<ConnectionMatrix>(filterBank.size(), numNeurons);
    }
    
    audioBuffer.resize(bufferSize);
    filterOutputs.resize(filterBank.size());
    filterGains.resize(filterBank.size(), 1.0f); // Initialize all filter gains to 1.0
    filterSoloEnabled.resize(filterBank.size(), false); // Initialize all solo states to false
    anyFilterSoloed = false; // No filters soloed initially
    audioOutputEnabled = true; // Audio output ENABLED by default for debugging
    processedAudioBuffer.resize(bufferSize); // Initialize processed audio buffer
    if (neuronNetwork) {
        neuronInputs.resize(neuronNetwork->getNeurons().size());
    }
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

void RhythmInterpreter::processAudioFrame(const std::vector<float>& audioData) {
    static int debugCounter = 0;
    
    // Debug: Always log first few calls and periodically after
    if (debugCounter < 5 || debugCounter % 1000 == 0) {
        std::cout << "🎛️  processAudioFrame called #" << debugCounter 
                  << " - enabled: " << enabled 
                  << ", neuronNetwork: " << (neuronNetwork ? "yes" : "no")
                  << ", audioData size: " << audioData.size() << std::endl;
    }
    
    if (!enabled || !neuronNetwork) {
        std::cout << "⚠️  processAudioFrame early return - enabled: " << enabled 
                  << ", neuronNetwork: " << (neuronNetwork ? "valid" : "null") << std::endl;
        return;
    }
    
    // Debug: Check if we're receiving audio data
    debugCounter++;
    if (debugCounter % 100 == 0 && !audioData.empty()) {
        float maxSample = 0.0f;
        for (float sample : audioData) {
            maxSample = std::max(maxSample, std::abs(sample));
        }
        std::cout << "📢 Audio input: " << audioData.size() << " samples, max: " << maxSample << std::endl;
    }
    
    // Process audio through rhythm detector
    rhythmDetector->processAudioChunk(audioData);
    
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
        for (size_t j = 0; j < audioData.size(); ++j) {
            float filteredSample = filterBank[i]->process(audioData[j]);
            maxFilteredSample = std::max(maxFilteredSample, std::abs(filteredSample));
            filterAudioOutputs[i][j] = filteredSample * globalGain * filterGains[i];
            sum += filteredSample;
        }
        filterOutputs[i] = (sum / audioData.size()) * globalGain * filterGains[i];
        
        // Debug: Log filter output levels occasionally
        if (debugCounter % 1000 == 0 && i == 0) { // Only log for first filter to avoid spam
            std::cout << "🎛️  Filter " << i << " - Raw sum: " << sum << ", Max sample: " << maxFilteredSample 
                      << ", Final output: " << filterOutputs[i] << ", Gain: " << filterGains[i] << std::endl;
        }
        
        // Adapt filter based on rhythm strength
        float rhythmStrength = rhythmDetector->getBeatStrength();
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
            std::cout << "🎛️  Filter solo active: " << soloCount << " bands soloed" << std::endl;
        }
    } else {
        // Output original audio (pass-through when no filters are soloed)
        processedAudioBuffer = audioData;
        
        // Debug: Output for pass-through
        if (debugCounter % 100 == 0 && !audioData.empty()) {
            std::cout << "🔄 Pass-through mode: " << audioData.size() << " samples" << std::endl;
        }
    }
    
    // Debugging: Log audio processing details every 1000 frames to reduce spam
    if (debugCounter % 1000 == 0) {
        std::cout << "🎛️  Global Gain: " << globalGain << ", Filter Gains: [";
        for (size_t i = 0; i < std::min(filterGains.size(), size_t(3)); ++i) {
            std::cout << filterGains[i];
            if (i < std::min(filterGains.size(), size_t(3)) - 1) std::cout << ", ";
        }
        if (filterGains.size() > 3) std::cout << "...";
        std::cout << "]" << std::endl;
        
        float maxOutput = 0.0f;
        for (float sample : processedAudioBuffer) {
            maxOutput = std::max(maxOutput, std::abs(sample));
        }
        std::cout << "🎛️  Processed Audio Buffer Size: " << processedAudioBuffer.size() 
                  << ", Max Level: " << maxOutput << std::endl;
    }
}

void RhythmInterpreter::update() {
    if (!enabled || !neuronNetwork || !connectionMatrix) return;
    
    // Transform filter outputs to neuron inputs through connection matrix
    neuronInputs = connectionMatrix->transform(filterOutputs);
    
    // Apply inputs to neurons (this would need integration with NeuronNetwork)
    const auto& neurons = neuronNetwork->getNeurons();
    for (size_t i = 0; i < std::min(neuronInputs.size(), neurons.size()); ++i) {
        // This would require adding a method to Neuron class to accept external input
        // For now, we'll store the inputs for potential future integration
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

// Status methods
bool RhythmInterpreter::isRhythmDetected() const {
    return rhythmDetector && rhythmDetector->isBeatDetected();
}

float RhythmInterpreter::getCurrentTempo() const {
    return rhythmDetector ? rhythmDetector->getCurrentTempo() : 120.0f;
}

float RhythmInterpreter::getOverallRhythmStrength() const {
    return rhythmDetector ? rhythmDetector->getGrooveStrength() : 0.0f;
}

void RhythmInterpreter::setFilterGain(size_t filterIndex, float gain) {
    if (filterIndex < filterGains.size()) {
        filterGains[filterIndex] = std::max(0.0f, std::min(2.0f, gain)); // Clamp between 0.0 and 2.0
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