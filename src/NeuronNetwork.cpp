#include "NeuronNetwork.h"
#include "AudioManager.h"
#include "RhythmInterpreter.h"
#include "BeatTracker.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

namespace {
constexpr float TWO_PI = 2.0f * static_cast<float>(std::acos(-1.0f));
constexpr float STM_NEURON_DECAY = 0.70f;
constexpr float STM_NEURON_ACTIVATION_BLEND = 0.20f;
constexpr float STM_NEURON_LTM_BLEND = 0.10f;
constexpr float LTM_NEURON_DECAY = 0.96f;
constexpr float LTM_NEURON_STM_BLEND = 0.04f;
constexpr float NEURON_LTM_DRIVE_BLEND = 0.60f;
constexpr float NEURON_STM_DRIVE_BLEND = 0.40f;
constexpr float NEURON_MEMORY_DRIVE_SCALE = 0.08f;
constexpr float STM_CONNECTION_DECAY = 0.75f;
constexpr float STM_CONNECTION_WEIGHT_BLEND = 0.25f;
constexpr float LTM_CONNECTION_DECAY = 0.97f;
constexpr float LTM_CONNECTION_STM_BLEND = 0.03f;
constexpr float CONNECTION_STM_WEIGHT_BLEND = 0.65f;
constexpr float CONNECTION_LTM_WEIGHT_BLEND = 0.35f;
constexpr float CONNECTION_CURRENT_WEIGHT_BLEND = 0.985f;
constexpr float CONNECTION_CANDIDATE_WEIGHT_BLEND = 0.015f;
constexpr float CONNECTION_SCORE_WEIGHT = 0.60f;
constexpr float CONNECTION_SCORE_STM = 0.25f;
constexpr float CONNECTION_SCORE_LTM = 0.15f;
constexpr float CONNECTION_PRUNE_BASE = 0.08f;
constexpr float CONNECTION_PRUNE_RHYTHM_SCALE = 0.20f;
constexpr float NEURON_SCORE_STM = 0.70f;
constexpr float NEURON_SCORE_LTM = 0.30f;
constexpr float NEURON_PRUNE_BASE = 0.05f;
constexpr float NEURON_PRUNE_RHYTHM_SCALE = 0.20f;
}

NeuronNetwork::NeuronNetwork() 
    : audioManager(nullptr), rhythmInterpreter(nullptr), quantizer(nullptr)
{
    // Initialize beat tracker (44100 Hz, 512 frame size as defaults)
    beatTracker = std::make_unique<BeatTracker>(44100, 512);
}

NeuronNetwork::~NeuronNetwork() {
    delete rhythmInterpreter;
    rhythmInterpreter = nullptr;
}

void NeuronNetwork::setAudioManager(AudioManager* manager) {
    audioManager = manager;
    // Update all existing neurons
    for (auto& neuron : neurons) {
        neuron->setAudioManager(manager);
    }
    
    // Initialize rhythm interpreter if audio manager is available
    if (manager && !rhythmInterpreter) {
        initializeRhythmInterpreter();
    }
}

void NeuronNetwork::setQuantizer(Quantizer* quantizerPtr) {
    quantizer = quantizerPtr;
    // Update all existing neurons
    for (auto& neuron : neurons) {
        neuron->setQuantizer(quantizerPtr);
    }
}

Neuron* NeuronNetwork::addNeuron(int sampleIndex, float initialActivation, 
                                 float threshold, float decayRate, 
                                 float activationIncreasePerIteration,
                                 ActivationFunction func,
                                 const std::string& sampleFilePath) {
    auto neuron = std::make_unique<Neuron>(sampleIndex, initialActivation, 
                                          threshold, decayRate, activationIncreasePerIteration, func, sampleFilePath);
    if (audioManager) {
        neuron->setAudioManager(audioManager);
    }
    if (quantizer) {
        neuron->setQuantizer(quantizer);
    }
    
    Neuron* rawPtr = neuron.get();
    neurons.push_back(std::move(neuron));
    ensureMemoryStateSize();
    
    // updateNetworkSize removed: minimal RhythmInterpreter does not support this method
    
    return rawPtr;
}

Connection* NeuronNetwork::connect(Neuron* source, Neuron* target, float weight) {
    auto connection = std::make_unique<Connection>(source, target, weight);
    Connection* rawPtr = connection.get();
    connections.push_back(std::move(connection));
    ensureMemoryStateSize();
    return rawPtr;
}

void NeuronNetwork::activate() {
    // Reset all fired flags first
    resetFiredFlags();
    
    // update removed: minimal RhythmInterpreter does not support this method
    
    // Update all neurons (apply activation_increase_per_iteration)
    for (auto& neuron : neurons) {
        neuron->update();
    }
    
    // Get beat phase boost for connection weights if in ConnectionWeights mode
    float weightMultiplier = 1.0f;
    if (beatTracker && beatTracker->isEnabled() && 
        beatTracker->getBoostTarget() == BoostTarget::ConnectionWeights) {
        weightMultiplier = beatTracker->getPhaseBasedLearningGain();
    }
    
    // Then activate all connections with phase-based weight modulation
    for (auto& connection : connections) {
        connection->activate(weightMultiplier);
    }
}

void NeuronNetwork::resetFiredFlags() {
    for (auto& neuron : neurons) {
        neuron->resetFiredFlag();
    }
}

bool NeuronNetwork::removeNeuron(size_t index) {
    if (index >= neurons.size()) {
        return false;
    }
    
    Neuron* neuronToRemove = neurons[index].get();
    
    // Remove all connections that involve this neuron
    connections.erase(
        std::remove_if(connections.begin(), connections.end(),
            [neuronToRemove](const std::unique_ptr<Connection>& conn) {
                return conn->getSource() == neuronToRemove || conn->getTarget() == neuronToRemove;
            }),
        connections.end()
    );
    
    // Remove the neuron itself
    neurons.erase(neurons.begin() + index);
    ensureMemoryStateSize();
    
    // updateNetworkSize removed: minimal RhythmInterpreter does not support this method
    
    return true;
}

bool NeuronNetwork::removeConnection(size_t index) {
    if (index >= connections.size()) {
        return false;
    }
    
    connections.erase(connections.begin() + index);
    ensureMemoryStateSize();
    return true;
}

void NeuronNetwork::clearNetwork() {
    connections.clear();
    neurons.clear();
    stmNeuronPhase.clear();
    ltmNeuronPhase.clear();
    stmConnectionPhase.clear();
    ltmConnectionPhase.clear();
    convergenceFrameCounter = 0;
    delete rhythmInterpreter; // Also clear rhythm interpreter
    rhythmInterpreter = nullptr;
}

void NeuronNetwork::initializeRhythmInterpreter() {
    if (audioManager && !rhythmInterpreter) {
        // Use default sampleRate and bufferSize for minimal RhythmInterpreter
        size_t sampleRate = 44100;
        size_t bufferSize = 512;
        rhythmInterpreter = new RhythmInterpreter(sampleRate, bufferSize);
        std::cout << "🎵 Minimal RhythmInterpreter initialized" << std::endl;
    }
}

void NeuronNetwork::processAudioForRhythm(const std::vector<float>& audioData) {
    if (rhythmInterpreter) {
        rhythmInterpreter->processAudioFrame(audioData);
        
        // Update beat tracker with network firings and input onsets
        if (beatTracker) {
            // Sync global tempo from auto-detected tempo if enabled
            if (rhythmInterpreter->isAutoTempoEnabled()) {
                float autoTempo = rhythmInterpreter->getDetectedTempo();
                beatTracker->setGlobalTempo(autoTempo);
            }
            
            // Collect network firing activity
            std::vector<float> networkFirings;
            for (const auto& neuron : neurons) {
                networkFirings.push_back(neuron->getHasFired() ? 1.0f : 0.0f);
            }
            
            // Get input onset activity from rhythm interpreter
            std::vector<float> inputOnsets = rhythmInterpreter->getFilterOutputs();
            
            // Update beat tracker
            beatTracker->update(networkFirings, inputOnsets);
        }
        
        // Apply rhythm filter outputs to neurons
        applyRhythmConnections();
        
        // Update weights if learning is enabled
        if (learningEnabled) {
            learnFromRhythm();
        }

        // Exchange phase-coupled STM/LTM activations and adapt to minimal topology
        ensureMemoryStateSize();
        exchangeMemoryPhaseState();
        convergeSTMTopology();
    }
}

std::vector<float> NeuronNetwork::getProcessedAudioOutput() const {
    // Minimal RhythmInterpreter: getProcessedAudioOutput not supported
    return std::vector<float>(); // Return empty vector
}

// Rhythm-to-neuron connection matrix methods
void NeuronNetwork::setRhythmConnection(size_t filterIndex, size_t neuronIndex, float weight) {
    // Safety checks to prevent bad allocations
    const size_t MAX_REASONABLE_SIZE = 1000; // Reasonable upper limit
    
    if (filterIndex >= MAX_REASONABLE_SIZE || neuronIndex >= MAX_REASONABLE_SIZE) {
        std::cerr << "❌ setRhythmConnection: Invalid indices - filterIndex: " 
                  << filterIndex << ", neuronIndex: " << neuronIndex << std::endl;
        return;
    }
    
    try {
        // Ensure matrix is large enough
        if (rhythmConnectionMatrix.size() <= filterIndex) {
            rhythmConnectionMatrix.resize(filterIndex + 1);
        }
        if (rhythmConnectionMatrix[filterIndex].size() <= neuronIndex) {
            rhythmConnectionMatrix[filterIndex].resize(neuronIndex + 1, 0.0f);
        }
        
        rhythmConnectionMatrix[filterIndex][neuronIndex] = weight;
    } catch (const std::exception& e) {
        std::cerr << "❌ setRhythmConnection: Exception during resize: " << e.what() << std::endl;
        std::cerr << "   filterIndex: " << filterIndex << ", neuronIndex: " << neuronIndex << std::endl;
    }
}

float NeuronNetwork::getRhythmConnection(size_t filterIndex, size_t neuronIndex) const {
    if (filterIndex < rhythmConnectionMatrix.size() && 
        neuronIndex < rhythmConnectionMatrix[filterIndex].size()) {
        return rhythmConnectionMatrix[filterIndex][neuronIndex];
    }
    return 0.0f;
}

void NeuronNetwork::clearRhythmConnection(size_t filterIndex, size_t neuronIndex) {
    setRhythmConnection(filterIndex, neuronIndex, 0.0f);
}

void NeuronNetwork::applyRhythmConnections() {
    if (!rhythmInterpreter) return;

    // Get current filter outputs
    std::vector<float> filterOutputs = rhythmInterpreter->getFilterOutputs();

    // Ensure matrix sized to current band/neuron counts
    ensureMatrixSize(filterOutputs.size(), neurons.size());
    
    // Get beat phase boost if beat tracker is enabled and set to Activation mode
    float phaseBoost = 1.0f;
    if (beatTracker && beatTracker->isEnabled() && 
        beatTracker->getBoostTarget() == BoostTarget::Activation) {
        phaseBoost = beatTracker->getPhaseBasedLearningGain();
    }
    
    // Apply rhythm inputs directly to neurons
    for (size_t f = 0; f < filterOutputs.size() && f < rhythmConnectionMatrix.size(); ++f) {
        for (size_t n = 0; n < rhythmConnectionMatrix[f].size() && n < neurons.size(); ++n) {
            float mapWeight = rhythmConnectionMatrix[f][n];
            if (std::abs(mapWeight) > 0.001f) {
                float rhythmInput = filterOutputs[f] * mapWeight * mappingGain * phaseBoost;
                if (neurons[n]) {
                    neurons[n]->addExternalInput(rhythmInput);
                }
            }
        }
    }
}

void NeuronNetwork::ensureMatrixSize(size_t bandCount, size_t neuronCount) {
    if (rhythmConnectionMatrix.size() < bandCount) {
        rhythmConnectionMatrix.resize(bandCount);
    }
    for (size_t f = 0; f < bandCount; ++f) {
        if (rhythmConnectionMatrix[f].size() < neuronCount) {
            rhythmConnectionMatrix[f].resize(neuronCount, 0.0f);
        }
    }
    // Ensure neuron band map sized and initialized (per neuron)
    if (neuronBandMap.size() < neuronCount) {
        size_t start = neuronBandMap.size();
        neuronBandMap.resize(neuronCount);
        size_t bands = bandCount > 0 ? bandCount : 1;
        for (size_t n = start; n < neuronCount; ++n) {
            neuronBandMap[n] = n % bands; // default cyclic assignment
        }
    }
}

void NeuronNetwork::ensureMemoryStateSize() {
    if (stmNeuronPhase.size() != neurons.size()) {
        stmNeuronPhase.resize(neurons.size(), 0.0f);
    }
    if (ltmNeuronPhase.size() != neurons.size()) {
        ltmNeuronPhase.resize(neurons.size(), 0.0f);
    }
    if (stmConnectionPhase.size() != connections.size()) {
        stmConnectionPhase.resize(connections.size(), 0.0f);
    }
    if (ltmConnectionPhase.size() != connections.size()) {
        ltmConnectionPhase.resize(connections.size(), 0.0f);
    }
}

void NeuronNetwork::exchangeMemoryPhaseState() {
    if (neurons.empty()) return;

    const float phase = beatTracker ? beatTracker->getCurrentPhase() : 0.0f;
    const float phaseEnvelope = 0.5f + 0.5f * std::cos(TWO_PI * phase);

    // Bidirectional STM <-> LTM phase exchange for neurons
    for (size_t n = 0; n < neurons.size(); ++n) {
        float currentActivation = neurons[n] ? std::abs(neurons[n]->getActivation()) : 0.0f;
        currentActivation = std::clamp(currentActivation, 0.0f, 1.0f);

        float stmUpdated = STM_NEURON_DECAY * stmNeuronPhase[n]
                         + STM_NEURON_ACTIVATION_BLEND * currentActivation
                         + STM_NEURON_LTM_BLEND * ltmNeuronPhase[n];
        stmUpdated = std::clamp(stmUpdated, 0.0f, 1.0f);
        float ltmUpdated = LTM_NEURON_DECAY * ltmNeuronPhase[n] + LTM_NEURON_STM_BLEND * stmUpdated;
        stmNeuronPhase[n] = stmUpdated;
        ltmNeuronPhase[n] = std::clamp(ltmUpdated, 0.0f, 1.0f);

        // Feed reciprocal memory phase into neuron as bounded external drive
        float reciprocalMemory = (NEURON_LTM_DRIVE_BLEND * ltmNeuronPhase[n]) + (NEURON_STM_DRIVE_BLEND * stmNeuronPhase[n]);
        float memoryDrive = (reciprocalMemory - currentActivation) * NEURON_MEMORY_DRIVE_SCALE * phaseEnvelope;
        if (neurons[n] && std::abs(memoryDrive) > 1e-5f) {
            neurons[n]->addExternalInput(memoryDrive);
        }
    }

    // Track STM/LTM phase state for weighted connections and blend back to weights
    for (size_t c = 0; c < connections.size(); ++c) {
        Connection* conn = connections[c].get();
        if (!conn) continue;

        float magnitude = std::clamp(std::abs(conn->getWeight()), 0.0f, 1.0f);
        float stmUpdated = STM_CONNECTION_DECAY * stmConnectionPhase[c] + STM_CONNECTION_WEIGHT_BLEND * magnitude;
        stmUpdated = std::clamp(stmUpdated, 0.0f, 1.0f);
        float ltmUpdated = LTM_CONNECTION_DECAY * ltmConnectionPhase[c] + LTM_CONNECTION_STM_BLEND * stmUpdated;
        stmConnectionPhase[c] = stmUpdated;
        ltmConnectionPhase[c] = std::clamp(ltmUpdated, 0.0f, 1.0f);

        float sign = conn->getWeight() >= 0.0f ? 1.0f : -1.0f;
        float coupledMagnitude = CONNECTION_STM_WEIGHT_BLEND * stmConnectionPhase[c]
                               + CONNECTION_LTM_WEIGHT_BLEND * ltmConnectionPhase[c];
        float candidateWeight = sign * coupledMagnitude;
        float blendedWeight = CONNECTION_CURRENT_WEIGHT_BLEND * conn->getWeight()
                            + CONNECTION_CANDIDATE_WEIGHT_BLEND * candidateWeight;
        conn->setWeight(std::clamp(blendedWeight, -maxWeight, maxWeight));
    }
}

void NeuronNetwork::convergeSTMTopology() {
    if (!structuralConvergenceEnabled || neurons.size() <= 2 || connections.empty()) {
        return;
    }
    if (!beatTracker || !beatTracker->isEnabled()) {
        return;
    }

    ++convergenceFrameCounter;
    if (convergenceFrameCounter < convergenceIntervalFrames) {
        return;
    }
    convergenceFrameCounter = 0;

    const float phaseConfidence = beatTracker->getPhaseConfidence();
    const Pattern pattern = beatTracker->getStrongestPattern();
    const float rhythmStrength = std::max(pattern.strength, phaseConfidence);

    // Only prune when rhythm tracking is stable enough
    if (rhythmStrength < 0.20f || phaseConfidence < 0.10f) {
        return;
    }

    // Remove one least-useful connection at a time while rhythm remains stable
    if (connections.size() > 1) {
        float weakestScore = std::numeric_limits<float>::max();
        size_t weakestIndex = 0;
        for (size_t i = 0; i < connections.size(); ++i) {
            Connection* conn = connections[i].get();
            if (!conn) continue;
            float score = (CONNECTION_SCORE_WEIGHT * std::abs(conn->getWeight()))
                        + (CONNECTION_SCORE_STM * stmConnectionPhase[i])
                        + (CONNECTION_SCORE_LTM * ltmConnectionPhase[i]);
            if (score < weakestScore) {
                weakestScore = score;
                weakestIndex = i;
            }
        }
        if (weakestScore < (CONNECTION_PRUNE_BASE + CONNECTION_PRUNE_RHYTHM_SCALE * rhythmStrength)) {
            removeConnection(weakestIndex);
        }
    }

    // Prune isolated/weak neurons after connection pruning
    std::vector<size_t> degree(neurons.size(), 0);
    for (const auto& conn : connections) {
        if (!conn) continue;
        for (size_t n = 0; n < neurons.size(); ++n) {
            Neuron* neuron = neurons[n].get();
            if (!neuron) continue;
            if (conn->getSource() == neuron || conn->getTarget() == neuron) {
                degree[n]++;
            }
        }
    }

    float weakestNeuronScore = std::numeric_limits<float>::max();
    size_t weakestNeuronIndex = neurons.size();
    for (size_t n = 0; n < neurons.size(); ++n) {
        if (degree[n] != 0) continue; // only isolated neurons are eligible for pruning
        float score = NEURON_SCORE_STM * stmNeuronPhase[n] + NEURON_SCORE_LTM * ltmNeuronPhase[n];
        if (score < weakestNeuronScore) {
            weakestNeuronScore = score;
            weakestNeuronIndex = n;
        }
    }

    if (weakestNeuronIndex < neurons.size() && weakestNeuronScore < (NEURON_PRUNE_BASE + NEURON_PRUNE_RHYTHM_SCALE * rhythmStrength)) {
        removeNeuron(weakestNeuronIndex);
    }
}

size_t NeuronNetwork::assignedBandForNeuron(size_t neuronIndex) const {
    size_t bands = rhythmInterpreter ? rhythmInterpreter->getBandCount() : 1;
    if (bands == 0) bands = 1;
    if (neuronIndex < neuronBandMap.size()) {
        return std::min(neuronBandMap[neuronIndex], bands - 1);
    }
    return neuronIndex % bands; // fallback cyclic assignment
}

void NeuronNetwork::resetRhythmWeights(float value) {
    if (!rhythmInterpreter) return;
    ensureMatrixSize(rhythmInterpreter->getBandCount(), neurons.size());
    for (auto& row : rhythmConnectionMatrix) {
        std::fill(row.begin(), row.end(), value);
    }
}

void NeuronNetwork::learnFromRhythm() {
    if (!rhythmInterpreter || neurons.empty()) return;
    std::vector<float> filterOutputs = rhythmInterpreter->getFilterOutputs();
    ensureMatrixSize(filterOutputs.size(), neurons.size());

    // Normalize filter outputs to [0,1]
    float maxOut = 0.0f;
    for (float v : filterOutputs) maxOut = std::max(maxOut, v);
    float eps = 1e-6f;
    std::vector<float> normOut(filterOutputs.size(), 0.0f);
    for (size_t f = 0; f < filterOutputs.size(); ++f) {
        normOut[f] = filterOutputs[f] / std::max(eps, maxOut);
    }
    
    // Get recent onsets for temporal gating
    // Recent onsets boost learning rate when rhythmic events are detected
    std::vector<std::vector<RhythmInterpreter::OnsetEvent>> recentOnsets(filterOutputs.size());
    const float onsetWindow = 0.1f; // Consider onsets within last 100ms
    float currentTime = rhythmInterpreter->getAllOnsets().empty() ? 0.0f 
                       : rhythmInterpreter->getAllOnsets().back().timestamp;
    
    for (size_t f = 0; f < filterOutputs.size(); ++f) {
        std::vector<RhythmInterpreter::OnsetEvent> bandOnsets = rhythmInterpreter->getOnsetHistory(f);
        for (const auto& onset : bandOnsets) {
            if (currentTime - onset.timestamp < onsetWindow) {
                recentOnsets[f].push_back(onset);
            }
        }
    }

    // Learning target: band energy per neuron; prediction: neuron's current activation
    for (size_t n = 0; n < neurons.size(); ++n) {
        size_t fb = assignedBandForNeuron(n);
        float target = (fb < normOut.size()) ? normOut[fb] : 0.0f;
        float predicted = neurons[n]->getActivation(); // current processed activation
        float error = predicted - target; // d/dw ∝ error * feature

        // Compute rhythm drive for this neuron based on all mapped bands
        float rhythmDrive = 0.0f;
        float onsetBoost = 1.0f; // Multiplicative boost from detected onsets
        
        for (size_t f = 0; f < normOut.size(); ++f) {
            float map = (f < rhythmConnectionMatrix.size() && n < rhythmConnectionMatrix[f].size())
                            ? rhythmConnectionMatrix[f][n]
                            : 0.0f;
            if (map > 0.0f) {
                rhythmDrive += normOut[f] * map;
                
                // Boost learning when onsets are detected in mapped bands
                if (!recentOnsets[f].empty()) {
                    // Average onset strength from recent onsets
                    float avgOnsetStrength = 0.0f;
                    for (const auto& onset : recentOnsets[f]) {
                        avgOnsetStrength += onset.strength;
                    }
                    avgOnsetStrength /= recentOnsets[f].size();
                    
                    // Scale onset boost by mapping weight, onset strength, and onset bias
                    onsetBoost += map * avgOnsetStrength * 2.0f * onsetBias; // onset emphasis scaled by bias
                }
            }
        }
        
        // Apply onset bias: blend between pure filter output and onset-boosted learning
        // At onsetBias=0: pure filter output, onsetBias=1: full onset boost
        float effectiveOnsetBoost = 1.0f + (onsetBoost - 1.0f) * onsetBias;
        rhythmDrive *= mappingGain; // reflect applied scaling
        
        // Clamp effective onset boost to reasonable range
        effectiveOnsetBoost = std::min(5.0f, std::max(1.0f, effectiveOnsetBoost));
        
        // Apply beat phase-based learning gain if beat tracker is in Learning mode
        if (beatTracker && beatTracker->isEnabled() && 
            beatTracker->getBoostTarget() == BoostTarget::Learning) {
            float phaseGain = beatTracker->getPhaseBasedLearningGain();
            effectiveOnsetBoost *= phaseGain;
        }

        // Update connection weights targeting this neuron
        for (size_t c = 0; c < connections.size(); ++c) {
            Connection* conn = connections[c].get();
            if (!conn || conn->getTarget() != neurons[n].get()) continue;

            // Feature: source activation modulated by neuron's rhythm drive
            float srcAct = conn->getSource() ? conn->getSource()->getActivation() : 0.0f;
            float x = srcAct * rhythmDrive;

            if (x == 0.0f) continue; // nothing to learn this step

            float w = conn->getWeight();
            
            // Apply onset-modulated learning rate
            float adaptiveLR = learningRate * effectiveOnsetBoost;
            float dw = -adaptiveLR * error * x;      // gradient step with onset boost
            dw -= weightDecay * w;                   // L2-like regularization on connection weight
            w += dw;
            // Clamp for stability (allow negative weights)
            if (w >  maxWeight) w =  maxWeight;
            if (w < -maxWeight) w = -maxWeight;
            conn->setWeight(w);
        }
    }
}

void NeuronNetwork::setNeuronBandMapping(size_t neuronIndex, size_t bandIndex) {
    size_t bands = rhythmInterpreter ? rhythmInterpreter->getBandCount() : 1;
    if (neurons.empty()) return;
    ensureMatrixSize(bands, neurons.size());
    if (neuronIndex < neuronBandMap.size()) {
        neuronBandMap[neuronIndex] = std::min(bandIndex, bands - 1);
    }
}

size_t NeuronNetwork::getNeuronBandMapping(size_t neuronIndex) const {
    if (neuronIndex < neuronBandMap.size()) {
        return neuronBandMap[neuronIndex];
    }
    return assignedBandForNeuron(neuronIndex);
}