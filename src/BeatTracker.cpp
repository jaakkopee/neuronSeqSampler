#include "BeatTracker.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

BeatTracker::BeatTracker(size_t sampleRate, size_t frameSize)
    : sampleRate(sampleRate)
    , frameSize(frameSize)
    , enabled(false)  // Start disabled - user can enable via button
    , currentPhase(0.0f)
    , detectedTempo(120.0f)
    , phaseConfidence(0.0f)
    , beatPeriodSamples(0.0f)
    , beatBoost(5.0f)
    , phaseWindow(0.15f)
    , boostTarget(BoostTarget::Learning)
    , historyLength(sampleRate * 3)  // 3 seconds of history (optimization)
    , phaseVelocity(0.0f)
    , lastPhaseUpdate(0.0f)
    , frameCounter(0)
    , minTempo(40.0f)
    , maxTempo(320.0f)
    , tempoSmoothingFactor(0.95f)
    , maxAgents(5)
    , agentSpawnThreshold(0.3f)  // Lower threshold for easier agent spawning
    , agentRemovalThreshold(0.05f)  // Lower threshold to keep weak agents longer
{
    // Initialize beat period from default tempo
    beatPeriodSamples = (60.0f / detectedTempo) * sampleRate;
    phaseVelocity = 1.0f / beatPeriodSamples;
    
    // Initialize pattern finder
    patternFinder = std::make_unique<PatternFinder>(sampleRate);
    
    // Spawn initial agent with default hypothesis
    spawnAgent(detectedTempo, 0.0f);
}

void BeatTracker::update(const std::vector<float>& networkFirings, const std::vector<float>& inputOnsets) {
    if (!enabled) {
        return;
    }
    
    // Combine multi-dimensional signals into scalar activity measures
    float networkActivity = combineNetworkActivity(networkFirings);
    float inputActivity = combineInputActivity(inputOnsets);
    
    // Add to history buffers
    networkHistory.push_back(networkActivity);
    inputHistory.push_back(inputActivity);
    
    // Maintain history length
    while (networkHistory.size() > historyLength) {
        networkHistory.pop_front();
    }
    while (inputHistory.size() > historyLength) {
        inputHistory.pop_front();
    }
    
    // Update phase based on current tempo estimate
    updatePhase();
    
    // Update agent-based tracking system (every frame for real-time responsiveness)
    updateAgents();
    
    // Perform cross-correlation analysis periodically (every 200 frames ~= 2-4 sec)
    frameCounter++;
    if (frameCounter >= 200) {
        frameCounter = 0;
        if (networkHistory.size() >= sampleRate && inputHistory.size() >= sampleRate) {
            // Preprocess signals to emphasize both peaks (strong beats) and valleys (weak beats)
            preprocessSignalsForCorrelation();
            performCrossCorrelation();
        }
    }
}

void BeatTracker::updatePhase() {
    // Advance phase based on current tempo
    currentPhase += phaseVelocity * frameSize;
    
    // Wrap phase to [0, 1)
    while (currentPhase >= 1.0f) {
        currentPhase -= 1.0f;
    }
    while (currentPhase < 0.0f) {
        currentPhase += 1.0f;
    }
}

void BeatTracker::preprocessSignalsForCorrelation() {
    // Create bipolar signals that emphasize both peaks (strong beats) and valleys (weak beats)
    // This makes local minima count as negative influence in the correlation
    
    networkProcessed.clear();
    inputProcessed.clear();
    
    // Process network signal
    if (networkHistory.size() >= 5) {
        for (size_t i = 2; i < networkHistory.size() - 2; i++) {
            float current = networkHistory[i];
            float prev1 = networkHistory[i-1];
            float prev2 = networkHistory[i-2];
            float next1 = networkHistory[i+1];
            float next2 = networkHistory[i+2];
            
            float localMean = (prev2 + prev1 + current + next1 + next2) / 5.0f;
            
            // Create bipolar signal: deviation from local mean
            // Peaks become positive, valleys become negative
            float bipolar = current - localMean;
            
            // Detect local minima and accent them with negative values
            bool isLocalMin = (current < prev1 && current < next1 && 
                              current < prev2 && current < next2);
            
            if (isLocalMin) {
                // Emphasize valleys as negative influence
                bipolar -= 0.3f;
            }
            
            networkProcessed.push_back(bipolar);
        }
    }
    
    // Process input signal
    if (inputHistory.size() >= 5) {
        for (size_t i = 2; i < inputHistory.size() - 2; i++) {
            float current = inputHistory[i];
            float prev1 = inputHistory[i-1];
            float prev2 = inputHistory[i-2];
            float next1 = inputHistory[i+1];
            float next2 = inputHistory[i+2];
            
            float localMean = (prev2 + prev1 + current + next1 + next2) / 5.0f;
            
            // Create bipolar signal: deviation from local mean
            float bipolar = current - localMean;
            
            // Detect local minima and accent them with negative values
            bool isLocalMin = (current < prev1 && current < next1 && 
                              current < prev2 && current < next2);
            
            if (isLocalMin) {
                // Emphasize valleys as negative influence
                bipolar -= 0.3f;
            }
            
            inputProcessed.push_back(bipolar);
        }
    }
}

void BeatTracker::performCrossCorrelation() {
    // Calculate correlation for different lags corresponding to tempo range
    int minLagSamples = static_cast<int>((60.0f / maxTempo) * sampleRate);
    int maxLagSamples = static_cast<int>((60.0f / minTempo) * sampleRate);
    
    float bestCorrelation = -1.0f;
    int bestLag = 0;
    
    // Use preprocessed signals if available
    const auto& netSignal = networkProcessed.empty() ? networkHistory : networkProcessed;
    
    // Search for best lag in tempo range (coarse step for speed)
    for (int lag = minLagSamples; lag <= maxLagSamples && lag < static_cast<int>(netSignal.size()); lag += 50) {
        float correlation = calculateCorrelation(lag);
        if (correlation > bestCorrelation) {
            bestCorrelation = correlation;
            bestLag = lag;
        }
    }
    
    // Refine around best lag (smaller window for speed)
    if (bestLag > 0) {
        for (int lag = std::max(minLagSamples, bestLag - 10); 
             lag <= std::min(maxLagSamples, bestLag + 10) && lag < static_cast<int>(netSignal.size()); 
             lag += 2) {
            float correlation = calculateCorrelation(lag);
            if (correlation > bestCorrelation) {
                bestCorrelation = correlation;
                bestLag = lag;
            }
        }
    }
    
    // Update tempo if we found a good correlation
    if (bestCorrelation > 0.3f && bestLag > 0) {
        float newTempo = (60.0f * sampleRate) / static_cast<float>(bestLag);
        
        // Smooth tempo changes
        detectedTempo = tempoSmoothingFactor * detectedTempo + 
                       (1.0f - tempoSmoothingFactor) * newTempo;
        
        // Clamp to valid range
        detectedTempo = std::clamp(detectedTempo, minTempo, maxTempo);
        
        // Update beat period and phase velocity
        beatPeriodSamples = (60.0f / detectedTempo) * sampleRate;
        phaseVelocity = 1.0f / beatPeriodSamples;
        
        // Update confidence based on correlation strength
        phaseConfidence = std::min(1.0f, bestCorrelation);
        
        // Phase correction: find where in current cycle we are
        // Look for peak in recent input activity as phase reference
        findPhaseAlignment(bestLag);
        
        // Spawn new agent if correlation is strong enough (potential new hypothesis)
        // Phase is now refined by findPhaseAlignment above
        if (bestCorrelation > agentSpawnThreshold) {
            spawnAgent(newTempo, currentPhase);
        }
    } else {
        // Decay confidence when correlation is weak
        phaseConfidence *= 0.95f;
    }
}

void BeatTracker::findPhaseAlignment(int beatPeriod) {
    // Look at recent history to find phase offset (reduced window for speed)
    size_t searchWindow = std::min(static_cast<size_t>(beatPeriod), inputHistory.size());
    
    if (searchWindow < 2) return;
    
    // Find strongest onset in recent history
    float maxOnset = 0.0f;
    size_t maxPosition = 0;
    
    for (size_t i = inputHistory.size() - searchWindow; i < inputHistory.size(); i++) {
        if (inputHistory[i] > maxOnset) {
            maxOnset = inputHistory[i];
            maxPosition = i;
        }
    }
    
    // Calculate phase based on position of strongest onset
    if (maxOnset > 0.1f) {
        size_t samplesSinceOnset = inputHistory.size() - maxPosition;
        float phaseEstimate = static_cast<float>(samplesSinceOnset % beatPeriod) / beatPeriod;
        
        // Smooth phase correction to avoid jumps
        float phaseDiff = phaseEstimate - currentPhase;
        
        // Handle wraparound
        if (phaseDiff > 0.5f) phaseDiff -= 1.0f;
        if (phaseDiff < -0.5f) phaseDiff += 1.0f;
        
        // Apply gentle correction (10% per update)
        currentPhase += phaseDiff * 0.1f;
        
        // Wrap to [0, 1)
        while (currentPhase >= 1.0f) currentPhase -= 1.0f;
        while (currentPhase < 0.0f) currentPhase += 1.0f;
    }
}

float BeatTracker::calculateCorrelation(int lag) const {
    // Use preprocessed signals if available, otherwise fall back to raw signals
    const auto& netSignal = networkProcessed.empty() ? networkHistory : networkProcessed;
    const auto& inSignal = inputProcessed.empty() ? inputHistory : inputProcessed;
    
    if (lag >= static_cast<int>(netSignal.size()) || 
        lag >= static_cast<int>(inSignal.size())) {
        return 0.0f;
    }
    
    // Calculate normalized cross-correlation at this lag
    // Use only recent portion of history for speed (max 2 seconds)
    size_t maxSamples = std::min(static_cast<size_t>(sampleRate * 2), netSignal.size() - lag);
    size_t n = std::min(maxSamples, inSignal.size());
    
    if (n < 100) return 0.0f;  // Not enough data
    
    // Use only every 4th sample for correlation (downsampling for speed)
    float sum = 0.0f;
    float sumNetSq = 0.0f;
    float sumInSq = 0.0f;
    size_t count = 0;
    
    for (size_t i = 0; i < n; i += 4) {
        float netVal = netSignal[i + lag];
        float inVal = inSignal[i];
        
        sum += netVal * inVal;
        sumNetSq += netVal * netVal;
        sumInSq += inVal * inVal;
        count++;
    }
    
    if (count < 25) return 0.0f;  // Not enough samples
    
    // Normalized correlation coefficient
    float denom = std::sqrt(sumNetSq * sumInSq);
    if (denom < 1e-6f) return 0.0f;
    
    return sum / denom;
}

float BeatTracker::combineNetworkActivity(const std::vector<float>& networkFirings) const {
    if (networkFirings.empty()) return 0.0f;
    
    // Sum all neuron firings
    float activity = 0.0f;
    for (float firing : networkFirings) {
        activity += firing;
    }
    
    // Normalize by number of neurons
    return activity / static_cast<float>(networkFirings.size());
}

float BeatTracker::combineInputActivity(const std::vector<float>& inputOnsets) const {
    if (inputOnsets.empty()) return 0.0f;
    
    // Weight lower frequency bands more heavily (they contain downbeat info)
    float activity = 0.0f;
    float totalWeight = 0.0f;
    
    for (size_t i = 0; i < inputOnsets.size(); i++) {
        // Exponential decay weighting: lower bands get higher weight
        float weight = std::exp(-0.3f * i);
        activity += inputOnsets[i] * weight;
        totalWeight += weight;
    }
    
    return totalWeight > 0.0f ? activity / totalWeight : 0.0f;
}

float BeatTracker::getPhaseBasedLearningGain() const {
    if (!enabled || phaseConfidence < 0.1f) {
        return 1.0f;  // No modulation if disabled or low confidence
    }
    
    float gain = calculatePhaseGain(currentPhase);
    
    // Scale by confidence
    float modulatedGain = 1.0f + (gain - 1.0f) * phaseConfidence;
    
    return modulatedGain;
}

float BeatTracker::calculatePhaseGain(float phase) const {
    // Gaussian-like peak at phase = 0 (downbeat)
    // gain = 1 + beatBoost * exp(-(phase / phaseWindow)^2)
    
    // Handle wraparound: phase near 1.0 should also be near 0.0
    float distFromDownbeat = std::min(phase, 1.0f - phase);
    
    // Gaussian envelope
    float exponent = -(distFromDownbeat * distFromDownbeat) / (phaseWindow * phaseWindow);
    float boost = beatBoost * std::exp(exponent);
    
    return 1.0f + boost;
}

void BeatTracker::reset() {
    networkHistory.clear();
    inputHistory.clear();
    currentPhase = 0.0f;
    phaseConfidence = 0.0f;
    frameCounter = 0;
    detectedTempo = 120.0f;
    beatPeriodSamples = (60.0f / detectedTempo) * sampleRate;
    phaseVelocity = 1.0f / beatPeriodSamples;
    agents.clear();
    if (patternFinder) {
        patternFinder->reset();
    }
    // Spawn initial agent with default hypothesis for consistency with constructor
    spawnAgent(detectedTempo, 0.0f);
}

// ============================================================================
// Agent Implementation
// ============================================================================

Agent::Agent(float tempo, float phase, size_t sampleRate)
    : tempo(tempo)
    , phase(phase)
    , confidence(0.5f)  // Start with moderate confidence
    , sampleRate(sampleRate)
{
    beatPeriod = (60.0f / tempo) * sampleRate;
}

float Agent::scoreHypothesis(const std::deque<float>& onsets, const std::vector<Pattern>& patterns) {
    // Calculate onset alignment score
    float onsetScore = calculateOnsetAlignment(onsets);
    
    // Calculate pattern match score
    float patternScore = calculatePatternMatch(patterns);
    
    // Combine scores (60% onset alignment, 40% pattern matching)
    confidence = 0.6f * onsetScore + 0.4f * patternScore;
    
    // Clamp to valid range
    confidence = std::max(0.0f, std::min(1.0f, confidence));
    
    return confidence;
}

float Agent::calculateOnsetAlignment(const std::deque<float>& onsets) const {
    if (onsets.empty() || beatPeriod <= 0) {
        return 0.0f;
    }
    
    // Check how well onsets align with predicted beat positions
    float score = 0.0f;
    int sampleCount = 0;
    
    // Look at recent history (last 4 beats)
    size_t lookback = std::min(static_cast<size_t>(beatPeriod * 4), onsets.size());
    
    for (size_t i = onsets.size() - lookback; i < onsets.size(); i++) {
        // Calculate expected phase at this point in history
        float samplesSinceNow = static_cast<float>(onsets.size() - i);
        float expectedPhase = phase - (samplesSinceNow / beatPeriod);
        
        // Normalize to [0, 1)
        expectedPhase = expectedPhase - std::floor(expectedPhase);
        
        // Calculate distance to nearest beat (0.0 or 1.0)
        float distToDownbeat = std::min(expectedPhase, 1.0f - expectedPhase);
        
        // Weight onset strength by how close it is to a predicted beat
        // Strong onsets near predicted beats increase score
        float alignmentWeight = std::exp(-20.0f * distToDownbeat * distToDownbeat);
        score += onsets[i] * alignmentWeight;
        sampleCount++;
    }
    
    return sampleCount > 0 ? std::min(1.0f, score / sampleCount) : 0.0f;
}

float Agent::calculatePatternMatch(const std::vector<Pattern>& patterns) const {
    if (patterns.empty()) {
        return 0.5f;  // Neutral score if no patterns
    }
    
    float bestMatch = 0.0f;
    
    for (const auto& pattern : patterns) {
        // Check if pattern period matches this agent's tempo
        float periodRatio = pattern.period / beatPeriod;
        
        // Pattern should be close to 1x, 2x, 0.5x the beat period (harmonic relationships)
        float harmonicScore = 0.0f;
        for (float harmonic : {0.5f, 1.0f, 2.0f, 4.0f}) {
            float diff = std::abs(periodRatio - harmonic);
            if (diff < 0.1f) {  // Within 10% of harmonic
                harmonicScore = std::max(harmonicScore, 1.0f - diff * 10.0f);
            }
        }
        
        // Weight by pattern strength
        float matchScore = harmonicScore * pattern.strength;
        bestMatch = std::max(bestMatch, matchScore);
    }
    
    return bestMatch;
}

void Agent::adapt(float targetPhase, float targetTempo, float adaptationRate) {
    // Gradually adapt phase
    float phaseDiff = targetPhase - phase;
    
    // Handle wraparound
    if (phaseDiff > 0.5f) phaseDiff -= 1.0f;
    if (phaseDiff < -0.5f) phaseDiff += 1.0f;
    
    phase += phaseDiff * adaptationRate;
    
    // Wrap to [0, 1)
    while (phase >= 1.0f) phase -= 1.0f;
    while (phase < 0.0f) phase += 1.0f;
    
    // Gradually adapt tempo
    tempo += (targetTempo - tempo) * adaptationRate;
    beatPeriod = (60.0f / tempo) * sampleRate;
}

void Agent::advancePhase(size_t frameSamples) {
    float phaseIncrement = static_cast<float>(frameSamples) / beatPeriod;
    phase += phaseIncrement;
    
    // Wrap to [0, 1)
    while (phase >= 1.0f) phase -= 1.0f;
    while (phase < 0.0f) phase += 1.0f;
}

// ============================================================================
// PatternFinder Implementation
// ============================================================================

PatternFinder::PatternFinder(size_t sampleRate)
    : sampleRate(sampleRate)
{
}

std::vector<Pattern> PatternFinder::findPatterns(const std::deque<float>& onsets, 
                                                  float downbeatPhase, 
                                                  float beatPeriod) {
    recentPatterns.clear();
    
    if (onsets.empty() || beatPeriod <= 0) {
        return recentPatterns;
    }
    
    // Find peaks in onset data with adaptive threshold
    float threshold = 0.05f;  // Lowered threshold for better sensitivity
    auto peaks = findOnsetPeaks(onsets, threshold);
    
    if (peaks.size() < 2) {
        return recentPatterns;
    }
    
    // Limit peaks to reasonable number (focus on recent strong onsets)
    size_t maxPeaks = 16; // Reduced to 16 for cleaner patterns
    if (peaks.size() > maxPeaks) {
        // Keep the strongest peaks by re-sorting and filtering
        std::vector<std::pair<float, size_t>> strengthPeaks;
        for (size_t idx : peaks) {
            if (idx < onsets.size()) {
                strengthPeaks.push_back({onsets[idx], idx});
            }
        }
        std::sort(strengthPeaks.begin(), strengthPeaks.end(), 
                  [](const auto& a, const auto& b) { return a.first > b.first; });
        
        peaks.clear();
        for (size_t i = 0; i < maxPeaks && i < strengthPeaks.size(); ++i) {
            peaks.push_back(strengthPeaks[i].second);
        }
        std::sort(peaks.begin(), peaks.end()); // Re-sort by time
    }
    
    // Extract pattern relative to downbeat
    Pattern pattern = extractPattern(peaks, downbeatPhase, beatPeriod);
    
    if (pattern.occurrences >= 1) {
        recentPatterns.push_back(pattern);
    }
    
    return recentPatterns;
}

std::vector<size_t> PatternFinder::findOnsetPeaks(const std::deque<float>& onsets, float threshold) const {
    std::vector<size_t> peaks;
    
    if (onsets.size() < 3) {
        return peaks;
    }
    
    // Find local maxima above threshold
    for (size_t i = 1; i < onsets.size() - 1; i++) {
        if (onsets[i] > threshold &&
            onsets[i] > onsets[i-1] &&
            onsets[i] >= onsets[i+1]) {
            peaks.push_back(i);
        }
    }
    
    return peaks;
}

float PatternFinder::calculateRecurrence(const std::vector<size_t>& peakPositions, float period) const {
    if (peakPositions.size() < 2 || period <= 0) {
        return 0.0f;
    }
    
    // Count how many peaks align with periodic grid
    size_t alignedPeaks = 0;
    
    for (size_t peak : peakPositions) {
        // Calculate phase relative to period
        float phaseInPeriod = std::fmod(static_cast<float>(peak), period);
        float normalizedPhase = phaseInPeriod / period;
        
        // Check if peak aligns with any grid point (0, 0.25, 0.5, 0.75)
        for (float gridPoint : {0.0f, 0.25f, 0.5f, 0.75f}) {
            float dist = std::abs(normalizedPhase - gridPoint);
            dist = std::min(dist, 1.0f - dist);  // Wraparound
            
            if (dist < 0.1f) {  // Within 10% of grid point
                alignedPeaks++;
                break;
            }
        }
    }
    
    return static_cast<float>(alignedPeaks) / peakPositions.size();
}

Pattern PatternFinder::extractPattern(const std::vector<size_t>& peakPositions, 
                                       float downbeatPhase, 
                                       float beatPeriod) const {
    Pattern pattern;
    pattern.period = beatPeriod;
    pattern.occurrences = 0;
    pattern.strength = 0.0f;
    
    if (peakPositions.empty() || beatPeriod <= 0) {
        return pattern;
    }
    
    // Convert peak positions to phase values relative to downbeat
    for (size_t peakPos : peakPositions) {
        float phase = std::fmod(static_cast<float>(peakPos) + downbeatPhase * beatPeriod, beatPeriod) / beatPeriod;
        pattern.onsetPositions.push_back(phase);
    }
    
    // Calculate recurrence score
    pattern.strength = calculateRecurrence(peakPositions, beatPeriod);
    
    // Estimate occurrences (number of periods with peaks)
    if (!peakPositions.empty()) {
        float span = static_cast<float>(peakPositions.back() - peakPositions.front());
        pattern.occurrences = static_cast<int>(span / beatPeriod) + 1;
    }
    
    return pattern;
}

Pattern PatternFinder::getStrongestPattern() const {
    if (recentPatterns.empty()) {
        return Pattern();
    }
    
    auto strongest = std::max_element(recentPatterns.begin(), recentPatterns.end(),
        [](const Pattern& a, const Pattern& b) {
            return a.strength < b.strength;
        });
    
    return *strongest;
}

void PatternFinder::reset() {
    recentPatterns.clear();
}

// ============================================================================
// BeatTracker Agent Management
// ============================================================================

void BeatTracker::updateAgents() {
    // Update all agents with current onset data
    std::vector<Pattern> patterns;
    if (patternFinder) {
        patterns = patternFinder->findPatterns(inputHistory, currentPhase, beatPeriodSamples);
    }
    
    // Advance phase and score all agents
    for (auto& agent : agents) {
        agent->advancePhase(frameSize);
        agent->scoreHypothesis(inputHistory, patterns);
    }
    
    // Ensure we always have at least one agent
    if (agents.empty()) {
        spawnAgent(detectedTempo, currentPhase);
    }
    
    // Periodically spawn exploration agents (every ~5 seconds at 44.1kHz / 512 frames)
    static int explorationCounter = 0;
    explorationCounter++;
    if (explorationCounter >= 430 && agents.size() < maxAgents) {  // ~5 sec at 44.1kHz/512
        explorationCounter = 0;
        // Spawn agent with slightly different tempo for exploration
        float exploreTempo = detectedTempo * (0.9f + (rand() % 40) / 100.0f); // ±20%
        spawnAgent(exploreTempo, currentPhase);
    }
    
    // Remove weak agents
    pruneWeakAgents();
    
    // Update global state from best agent
    updateFromBestAgent();
}

void BeatTracker::spawnAgent(float tempo, float phase) {
    // Don't spawn if we're at max capacity
    if (agents.size() >= maxAgents) {
        return;
    }
    
    // Check if we already have a similar agent
    for (const auto& agent : agents) {
        float tempoDiff = std::abs(agent->getTempo() - tempo);
        float phaseDiff = std::abs(agent->getPhase() - phase);
        phaseDiff = std::min(phaseDiff, 1.0f - phaseDiff);  // Wraparound
        
        // If very similar agent exists, don't spawn
        if (tempoDiff < 5.0f && phaseDiff < 0.1f) {
            return;
        }
    }
    
    // Create new agent
    agents.push_back(std::make_unique<Agent>(tempo, phase, sampleRate));
}

void BeatTracker::pruneWeakAgents() {
    // Never remove all agents - keep at least one
    if (agents.size() <= 1) {
        return;
    }
    
    // Remove agents with confidence below threshold
    agents.erase(
        std::remove_if(agents.begin(), agents.end(),
            [this](const std::unique_ptr<Agent>& agent) {
                // Keep at least one agent even if weak
                if (agents.size() <= 1) return false;
                return agent->getConfidence() < agentRemovalThreshold;
            }),
        agents.end()
    );
}

Agent* BeatTracker::getBestAgent() {
    if (agents.empty()) {
        return nullptr;
    }
    
    auto best = std::max_element(agents.begin(), agents.end(),
        [](const std::unique_ptr<Agent>& a, const std::unique_ptr<Agent>& b) {
            return a->getConfidence() < b->getConfidence();
        });
    
    return best->get();
}

void BeatTracker::updateFromBestAgent() {
    Agent* best = getBestAgent();
    
    if (best && best->getConfidence() > 0.3f) {
        // Update global tempo and phase from best agent
        float targetTempo = best->getTempo();
        float targetPhase = best->getPhase();
        
        // Smooth transition
        detectedTempo = tempoSmoothingFactor * detectedTempo + 
                       (1.0f - tempoSmoothingFactor) * targetTempo;
        
        // Phase correction
        float phaseDiff = targetPhase - currentPhase;
        if (phaseDiff > 0.5f) phaseDiff -= 1.0f;
        if (phaseDiff < -0.5f) phaseDiff += 1.0f;
        currentPhase += phaseDiff * 0.1f;
        
        // Wrap phase
        while (currentPhase >= 1.0f) currentPhase -= 1.0f;
        while (currentPhase < 0.0f) currentPhase += 1.0f;
        
        // Update confidence
        phaseConfidence = best->getConfidence();
        
        // Update beat period
        beatPeriodSamples = (60.0f / detectedTempo) * sampleRate;
        phaseVelocity = 1.0f / beatPeriodSamples;
    }
}

void BeatTracker::setGlobalTempo(float tempo) {
    // Clamp to valid tempo range
    tempo = std::clamp(tempo, minTempo, maxTempo);
    
    // Update detected tempo with smoothing
    detectedTempo = detectedTempo * tempoSmoothingFactor + tempo * (1.0f - tempoSmoothingFactor);
    
    // Update beat period
    beatPeriodSamples = (60.0f / detectedTempo) * sampleRate;
    phaseVelocity = 1.0f / beatPeriodSamples;
}

void BeatTracker::setEnabled(bool newState) {
    enabled = newState;
}
