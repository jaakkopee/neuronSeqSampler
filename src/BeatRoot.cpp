#include <complex>
#include <cstring>
#include "BeatRoot.h"
#include "NeuronNetwork.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>

// ============================================================================
// OnsetDetector Implementation
// ============================================================================

OnsetDetector::OnsetDetector() 
    : onsetThreshold(0.3f), adaptiveThreshold(0.0f) {
    
    audioBuffer.resize(FFT_SIZE, 0.0f);
    previousEnergy.resize(FFT_SIZE / 8, 0.0f); // Frequency band energies
    currentEnergy.resize(FFT_SIZE / 8, 0.0f);
    onsetFunction.reserve(1000); // Reserve space for onset function history
    onsetTimes.reserve(100);     // Reserve space for onset times
}

void OnsetDetector::processAudioFrame(const std::vector<float>& audioData) {
    // Shift buffer and add new data
    size_t inputSize = std::min(audioData.size(), static_cast<size_t>(HOP_SIZE));
    
    // Shift existing data
    memmove(audioBuffer.data(), audioBuffer.data() + inputSize, 
            (FFT_SIZE - inputSize) * sizeof(float));
    
    // Add new data
    std::copy(audioData.begin(), audioData.begin() + inputSize, 
              audioBuffer.end() - inputSize);
    
    // Compute simplified energy spectrum
    computeEnergySpectrum(audioBuffer);
    
    // Calculate spectral flux: SF(n) = sum of positive energy differences
    float spectralFlux = 0.0f;
    for (size_t k = 0; k < currentEnergy.size(); ++k) {
        float diff = currentEnergy[k] - previousEnergy[k];
        if (diff > 0.0f) {
            spectralFlux += diff;
        }
        previousEnergy[k] = currentEnergy[k]; // Update for next frame
    }
    
    // Normalize and store
    spectralFlux /= currentEnergy.size();
    onsetFunction.push_back(spectralFlux);
    
    // Limit history size
    if (onsetFunction.size() > 1000) {
        onsetFunction.erase(onsetFunction.begin(), onsetFunction.begin() + 100);
    }
    
    // Update adaptive threshold
    updateAdaptiveThreshold(spectralFlux);
}

void OnsetDetector::computeEnergySpectrum(const std::vector<float>& input) {
    // Simplified energy spectrum using frequency band analysis
    size_t bandSize = input.size() / currentEnergy.size();
    
    for (size_t band = 0; band < currentEnergy.size(); ++band) {
        float energy = 0.0f;
        size_t start = band * bandSize;
        size_t end = std::min(start + bandSize, input.size());
        
        for (size_t i = start; i < end; ++i) {
            energy += input[i] * input[i];
        }
        
        currentEnergy[band] = energy / bandSize;
    }
}

void OnsetDetector::updateAdaptiveThreshold(float currentValue) {
    // Simple adaptive threshold using exponential moving average
    const float alpha = 0.01f; // Adaptation rate
    adaptiveThreshold = (1.0f - alpha) * adaptiveThreshold + alpha * currentValue;
}

void OnsetDetector::detectOnsets() {
    if (onsetFunction.size() < 3) return;
    
    float currentTime = (onsetFunction.size() * HOP_SIZE) / SAMPLE_RATE;
    float threshold = std::max(onsetThreshold, adaptiveThreshold * 2.0f);
    
    // Peak picking with local maxima detection
    size_t currentIdx = onsetFunction.size() - 2; // Check previous frame
    float currentValue = onsetFunction[currentIdx];
    
    if (currentValue > threshold &&
        currentValue > onsetFunction[currentIdx - 1] &&
        currentValue > onsetFunction[currentIdx + 1]) {
        
        // Check minimum time between onsets (50ms)
        float onsetTime = (currentIdx * HOP_SIZE) / SAMPLE_RATE;
        if (onsetTimes.empty() || (onsetTime - onsetTimes.back()) > 0.05f) {
            onsetTimes.push_back(onsetTime);
            
            // Limit onset history
            if (onsetTimes.size() > 100) {
                onsetTimes.erase(onsetTimes.begin(), onsetTimes.begin() + 20);
            }
        }
    }
}

float OnsetDetector::getLatestOnsetStrength() const {
    return onsetFunction.empty() ? 0.0f : onsetFunction.back();
}

bool OnsetDetector::hasRecentOnset(float timeWindow) const {
    if (onsetTimes.empty()) return false;
    
    float currentTime = (onsetFunction.size() * HOP_SIZE) / SAMPLE_RATE;
    return (currentTime - onsetTimes.back()) <= timeWindow;
}

void OnsetDetector::reset() {
    onsetFunction.clear();
    onsetTimes.clear();
    std::fill(previousEnergy.begin(), previousEnergy.end(), 0.0f);
    std::fill(currentEnergy.begin(), currentEnergy.end(), 0.0f);
    std::fill(audioBuffer.begin(), audioBuffer.end(), 0.0f);
    adaptiveThreshold = 0.0f;
}

// ============================================================================
// TempoInductor Implementation  
// ============================================================================

TempoInductor::TempoInductor() 
    : currentTempo(120.0f), tempoStrength(0.0f) {
    interOnsetIntervals.reserve(100);
}

void TempoInductor::addOnsetTime(float time) {
    if (!interOnsetIntervals.empty()) {
        float lastOnsetTime = interOnsetIntervals.empty() ? 0.0f : 
                             time - interOnsetIntervals.back();
        
        float ioi = time - lastOnsetTime;
        
        // Only consider reasonable IOIs (corresponding to 30-300 BPM)
        float minIOI = 60.0f / MAX_TEMPO;
        float maxIOI = 60.0f / MIN_TEMPO;
        
        if (ioi >= minIOI && ioi <= maxIOI) {
            interOnsetIntervals.push_back(ioi);
            updateClusters(ioi);
        }
    }
    
    // Limit IOI history
    if (interOnsetIntervals.size() > 50) {
        interOnsetIntervals.erase(interOnsetIntervals.begin(), 
                                 interOnsetIntervals.begin() + 10);
    }
}

void TempoInductor::updateClusters(float newIOI) {
    bool foundCluster = false;
    
    // Try to add to existing cluster
    for (auto& cluster : clusters) {
        float tolerance = cluster.meanInterval * CLUSTER_TOLERANCE;
        if (std::abs(newIOI - cluster.meanInterval) <= tolerance) {
            // Update cluster mean and weight
            float oldMean = cluster.meanInterval;
            cluster.count++;
            cluster.meanInterval = ((cluster.meanInterval * (cluster.count - 1)) + newIOI) / cluster.count;
            cluster.weight = calculateClusterWeight(cluster);
            cluster.tempo = 60.0f / cluster.meanInterval;
            foundCluster = true;
            break;
        }
    }
    
    // Create new cluster if none found
    if (!foundCluster) {
        if (clusters.size() < MAX_CLUSTERS) {
            clusters.emplace_back(newIOI);
        } else {
            // Replace weakest cluster
            auto weakest = std::min_element(clusters.begin(), clusters.end(),
                [](const IOICluster& a, const IOICluster& b) {
                    return a.weight < b.weight;
                });
            *weakest = IOICluster(newIOI);
        }
    }
    
    // Sort clusters by weight
    std::sort(clusters.begin(), clusters.end(),
        [](const IOICluster& a, const IOICluster& b) {
            return a.weight > b.weight;
        });
}

float TempoInductor::calculateClusterWeight(const IOICluster& cluster) const {
    // Weight based on cluster size and recency
    float sizeWeight = static_cast<float>(cluster.count);
    float recencyWeight = 1.0f; // Could be improved with time-based weighting
    return sizeWeight * recencyWeight;
}

void TempoInductor::updateTempo() {
    if (clusters.empty() || interOnsetIntervals.size() < MIN_IOIS_FOR_TEMPO) {
        tempoStrength = 0.0f;
        return;
    }
    
    // Use strongest cluster for tempo
    const IOICluster& strongest = clusters[0];
    currentTempo = strongest.tempo;
    
    // Calculate tempo strength based on cluster consistency
    float totalWeight = std::accumulate(clusters.begin(), clusters.end(), 0.0f,
        [](float sum, const IOICluster& cluster) {
            return sum + cluster.weight;
        });
    
    tempoStrength = strongest.weight / std::max(totalWeight, 1.0f);
}

bool TempoInductor::hasStableTempo() const {
    return tempoStrength > 0.6f && clusters.size() >= 2;
}

std::vector<float> TempoInductor::getTopTempos(size_t count) const {
    std::vector<float> tempos;
    size_t numTempos = std::min(count, clusters.size());
    
    for (size_t i = 0; i < numTempos; ++i) {
        tempos.push_back(clusters[i].tempo);
    }
    
    return tempos;
}

void TempoInductor::reset() {
    interOnsetIntervals.clear();
    clusters.clear();
    currentTempo = 120.0f;
    tempoStrength = 0.0f;
}

// ============================================================================
// BeatAgent Implementation
// ============================================================================

BeatAgent::BeatAgent(float tempo, float initialPhase) 
    : beatInterval(60.0f / tempo), phase(initialPhase), score(1.0f), 
      tolerance(0.05f), lastBeatTime(0.0f), isActive(true) {
    beatTimes.reserve(100);
}

void BeatAgent::update(float deltaTime, const std::vector<float>& onsetTimes, float currentTime) {
    if (!isActive) return;
    
    // Update phase
    phase += deltaTime;
    if (phase >= beatInterval) {
        // Beat occurred
        float beatTime = currentTime - (phase - beatInterval);
        beatTimes.push_back(beatTime);
        lastBeatTime = beatTime;
        phase = phase - beatInterval;
        
        // Limit beat history
        if (beatTimes.size() > 50) {
            beatTimes.erase(beatTimes.begin(), beatTimes.begin() + 10);
        }
    }
    
    // Score this agent based on onsets
    scoreOnsets(onsetTimes, currentTime);
    
    // Apply score decay
    score *= SCORE_DECAY;
    
    // Deactivate if score too low
    if (score < MIN_SCORE) {
        isActive = false;
    }
}

void BeatAgent::scoreOnsets(const std::vector<float>& onsetTimes, float currentTime) {
    // Score based on how well onsets align with predicted beats
    for (float onsetTime : onsetTimes) {
        if (onsetTime < currentTime - 2.0f) continue; // Too old
        
        // Find closest beat prediction
        float closestBeatDistance = std::numeric_limits<float>::max();
        
        for (float beatTime : beatTimes) {
            float distance = std::abs(onsetTime - beatTime);
            closestBeatDistance = std::min(closestBeatDistance, distance);
        }
        
        // Also check predicted next beat
        float nextBeatTime = getNextBeatTime(currentTime);
        float nextBeatDistance = std::abs(onsetTime - nextBeatTime);
        closestBeatDistance = std::min(closestBeatDistance, nextBeatDistance);
        
        // Score based on distance
        if (closestBeatDistance <= tolerance) {
            float reward = ONSET_REWARD * (1.0f - closestBeatDistance / tolerance);
            score += reward;
        } else if (closestBeatDistance <= tolerance * 2.0f) {
            score += ONSET_PENALTY;
        }
    }
    
    // Clamp score
    score = std::max(0.0f, std::min(score, 10.0f));
}

bool BeatAgent::isPredictingBeat(float currentTime, float tolerance) const {
    if (!isActive) return false;
    
    float nextBeatTime = getNextBeatTime(currentTime);
    return std::abs(currentTime - nextBeatTime) <= tolerance;
}

float BeatAgent::getNextBeatTime(float currentTime) const {
    return lastBeatTime + beatInterval * std::ceil((currentTime - lastBeatTime) / beatInterval);
}

void BeatAgent::reset() {
    phase = 0.0f;
    score = 1.0f;
    lastBeatTime = 0.0f;
    isActive = true;
    beatTimes.clear();
}

// ============================================================================
// BeatTracker Implementation
// ============================================================================

BeatTracker::BeatTracker() 
    : currentTime(0.0f), lastBeatTime(0.0f), currentTempo(120.0f), beatStrength(0.0f) {
    agents.reserve(MAX_AGENTS);
}

void BeatTracker::update(float deltaTime, const std::vector<float>& onsetTimes,
                        const std::vector<float>& tempos, const std::vector<float>& tempoStrengths) {
    currentTime += deltaTime;
    
    // Spawn agents for strong tempo hypotheses
    for (size_t i = 0; i < tempos.size() && i < tempoStrengths.size(); ++i) {
        if (tempoStrengths[i] > AGENT_SPAWN_THRESHOLD && agents.size() < MAX_AGENTS) {
            // Check if we already have an agent for this tempo
            bool hasAgent = false;
            for (const auto& agent : agents) {
                if (std::abs(agent->getTempo() - tempos[i]) < 5.0f) {
                    hasAgent = true;
                    break;
                }
            }
            
            if (!hasAgent) {
                spawnAgent(tempos[i]);
            }
        }
    }
    
    // Update all agents
    for (auto& agent : agents) {
        agent->update(deltaTime, onsetTimes, currentTime);
    }
    
    // Remove inactive agents
    cullWeakAgents();
    
    // Update winning agent
    updateWinningAgent();
}

void BeatTracker::spawnAgent(float tempo, float phase) {
    if (agents.size() >= MAX_AGENTS) return;
    
    agents.push_back(std::make_unique<BeatAgent>(tempo, phase));
}

void BeatTracker::cullWeakAgents() {
    agents.erase(
        std::remove_if(agents.begin(), agents.end(),
            [](const std::unique_ptr<BeatAgent>& agent) {
                return !agent->getIsActive();
            }),
        agents.end()
    );
}

void BeatTracker::updateWinningAgent() {
    if (agents.empty()) {
        winningAgent = nullptr;
        beatStrength = 0.0f;
        return;
    }
    
    // Find agent with highest score
    auto bestAgent = std::max_element(agents.begin(), agents.end(),
        [](const std::unique_ptr<BeatAgent>& a, const std::unique_ptr<BeatAgent>& b) {
            return a->getScore() < b->getScore();
        });
    
    if (bestAgent != agents.end()) {
        winningAgent = std::make_unique<BeatAgent>(**bestAgent);
        currentTempo = winningAgent->getTempo();
        beatStrength = winningAgent->getScore() / 10.0f; // Normalize to 0-1
        
        // Check if beat is predicted now
        if (winningAgent->isPredictingBeat(currentTime, BEAT_TOLERANCE)) {
            lastBeatTime = currentTime;
        }
    }
}

void BeatTracker::initializeFromTempo(float tempo, float firstBeatTime) {
    reset();
    spawnAgent(tempo, 0.0f);
    currentTempo = tempo;
    lastBeatTime = firstBeatTime;
}

bool BeatTracker::isBeatPredicted(float tolerance) const {
    return winningAgent && winningAgent->isPredictingBeat(currentTime, tolerance);
}

float BeatTracker::getNextBeatTime() const {
    return winningAgent ? winningAgent->getNextBeatTime(currentTime) : currentTime + 1.0f;
}

float BeatTracker::getCurrentBeatStrength() const {
    return beatStrength;
}

size_t BeatTracker::getNumActiveAgents() const {
    return std::count_if(agents.begin(), agents.end(),
        [](const std::unique_ptr<BeatAgent>& agent) {
            return agent->getIsActive();
        });
}

void BeatTracker::reset() {
    agents.clear();
    winningAgent = nullptr;
    currentTime = 0.0f;
    lastBeatTime = 0.0f;
    currentTempo = 120.0f;
    beatStrength = 0.0f;
}

// ============================================================================
// BeatRoot Main System Implementation  
// ============================================================================

BeatRoot::BeatRoot(NeuronNetwork* network)
    : neuronNetwork(network), currentTime(0.0f), isInitialized(false),
      beatDetectedThisFrame(false), lastBeatTime(0.0f), enabled(true),
      sensitivity(1.0f), autoInitialize(true), minInitializationTime(2.0f) {
    
    onsetDetector = std::make_unique<OnsetDetector>();
    tempoInductor = std::make_unique<TempoInductor>();
    beatTracker = std::make_unique<BeatTracker>();
    
    tempoHistory.resize(HISTORY_SIZE, 120.0f);
    beatStrengthHistory.resize(HISTORY_SIZE, 0.0f);
}

void BeatRoot::processAudioFrame(const std::vector<float>& audioData, float deltaTime) {
    if (!enabled) return;
    
    currentTime += deltaTime;
    
    // Process onset detection
    onsetDetector->processAudioFrame(audioData);
    onsetDetector->detectOnsets();
    
    // Update tempo induction with new onsets
    const auto& onsetTimes = onsetDetector->getOnsetTimes();
    for (float onsetTime : onsetTimes) {
        if (onsetTime >= currentTime - deltaTime) { // Only new onsets
            tempoInductor->addOnsetTime(onsetTime);
        }
    }
    tempoInductor->updateTempo();
    
    // Auto-initialize if conditions are met
    if (!isInitialized && autoInitialize && currentTime > minInitializationTime) {
        if (tempoInductor->hasStableTempo()) {
            initialize(tempoInductor->getCurrentTempo());
        }
    }
    
    // Update beat tracking if initialized
    if (isInitialized) {
        auto topTempos = tempoInductor->getTopTempos(3);
        std::vector<float> tempoStrengths;
        for (size_t i = 0; i < topTempos.size(); ++i) {
            tempoStrengths.push_back(tempoInductor->getTempoStrength() * (1.0f - 0.1f * i));
        }
        
        beatTracker->update(deltaTime, onsetTimes, topTempos, tempoStrengths);
        
        // Check for beat this frame
        beatDetectedThisFrame = beatTracker->isBeatPredicted();
        if (beatDetectedThisFrame) {
            lastBeatTime = currentTime;
        }
    } else {
        beatDetectedThisFrame = false;
    }
    
    // Update smoothing history
    tempoHistory.pop_front();
    tempoHistory.push_back(getCurrentTempo());
    
    beatStrengthHistory.pop_front();
    beatStrengthHistory.push_back(getBeatStrength());
}

void BeatRoot::update(float deltaTime) {
    // Additional per-frame updates can go here
    // Main processing happens in processAudioFrame
}

void BeatRoot::initialize(float tempo, float firstBeatTime) {
    beatTracker->initializeFromTempo(tempo, firstBeatTime);
    isInitialized = true;
    
    std::cout << "BeatRoot initialized with tempo: " << tempo << " BPM" << std::endl;
}

float BeatRoot::getCurrentTempo() const {
    return isInitialized ? beatTracker->getCurrentTempo() : tempoInductor->getCurrentTempo();
}

float BeatRoot::getBeatStrength() const {
    return isInitialized ? beatTracker->getBeatStrength() : tempoInductor->getTempoStrength();
}

float BeatRoot::getNextBeatTime() const {
    return isInitialized ? beatTracker->getNextBeatTime() : currentTime + 1.0f;
}

float BeatRoot::getOnsetStrength() const {
    return onsetDetector->getLatestOnsetStrength() * sensitivity;
}

bool BeatRoot::hasRecentOnset(float timeWindow) const {
    return onsetDetector->hasRecentOnset(timeWindow);
}

std::vector<float> BeatRoot::getTopTempos(size_t count) const {
    return tempoInductor->getTopTempos(count);
}

bool BeatRoot::hasStableTempo() const {
    return tempoInductor->hasStableTempo();
}

size_t BeatRoot::getNumActiveAgents() const {
    return isInitialized ? beatTracker->getNumActiveAgents() : 0;
}

float BeatRoot::getSmoothedTempo() const {
    return std::accumulate(tempoHistory.begin(), tempoHistory.end(), 0.0f) / tempoHistory.size();
}

float BeatRoot::getSmoothedBeatStrength() const {
    return std::accumulate(beatStrengthHistory.begin(), beatStrengthHistory.end(), 0.0f) / beatStrengthHistory.size();
}

void BeatRoot::reset() {
    onsetDetector->reset();
    tempoInductor->reset();
    beatTracker->reset();
    
    currentTime = 0.0f;
    isInitialized = false;
    beatDetectedThisFrame = false;
    lastBeatTime = 0.0f;
    
    std::fill(tempoHistory.begin(), tempoHistory.end(), 120.0f);
    std::fill(beatStrengthHistory.begin(), beatStrengthHistory.end(), 0.0f);
    
    std::cout << "BeatRoot system reset" << std::endl;
}