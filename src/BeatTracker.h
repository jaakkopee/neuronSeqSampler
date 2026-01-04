#pragma once
#include <vector>
#include <deque>
#include <cmath>
#include <algorithm>
#include <memory>

enum class BoostTarget {
    Learning,          // Apply phase boost to learning rate
    Activation,        // Apply phase boost to neuron activations
    ConnectionWeights  // Apply phase boost to connection weights
};

/**
 * @brief Represents a detected rhythmic pattern in onset data
 */
struct Pattern {
    std::vector<float> onsetPositions;  // Positions of strong onsets (in phase units 0.0-1.0)
    float period;                        // Pattern period in samples
    float strength;                      // Pattern strength (recurrence score)
    int occurrences;                     // Number of times pattern was detected
};

/**
 * @brief Agent representing a beat tracking hypothesis
 * 
 * Each agent maintains a tempo and phase hypothesis and competes
 * with other agents based on how well it aligns with input onset data.
 */
class Agent {
public:
    /**
     * @brief Construct an agent with initial hypothesis
     * @param tempo Initial tempo hypothesis in BPM
     * @param phase Initial phase hypothesis (0.0-1.0)
     * @param sampleRate Audio sample rate
     */
    Agent(float tempo, float phase, size_t sampleRate);
    
    /**
     * @brief Update agent's confidence score based on onset alignment
     * @param onsets Recent onset data
     * @param patterns Detected patterns to boost confidence
     * @return Updated confidence score
     */
    float scoreHypothesis(const std::deque<float>& onsets, const std::vector<Pattern>& patterns);
    
    /**
     * @brief Adapt phase and tempo based on new evidence
     * @param targetPhase Phase correction target
     * @param targetTempo Tempo correction target
     * @param adaptationRate Speed of adaptation (0.0-1.0)
     */
    void adapt(float targetPhase, float targetTempo, float adaptationRate = 0.1f);
    
    /**
     * @brief Advance phase based on current tempo
     * @param frameSamples Number of samples to advance
     */
    void advancePhase(size_t frameSamples);
    
    // Accessors
    float getTempo() const { return tempo; }
    float getPhase() const { return phase; }
    float getConfidence() const { return confidence; }
    void setConfidence(float conf) { confidence = std::max(0.0f, std::min(1.0f, conf)); }
    
private:
    float tempo;          // Tempo hypothesis in BPM
    float phase;          // Phase hypothesis (0.0-1.0, 0.0 = downbeat)
    float confidence;     // Confidence score (0.0-1.0)
    size_t sampleRate;    // Audio sample rate
    float beatPeriod;     // Beat period in samples (derived from tempo)
    
    /**
     * @brief Calculate how well this agent's hypothesis aligns with onsets
     */
    float calculateOnsetAlignment(const std::deque<float>& onsets) const;
    
    /**
     * @brief Calculate pattern match score for this agent
     */
    float calculatePatternMatch(const std::vector<Pattern>& patterns) const;
};

/**
 * @brief Pattern finder for detecting recurring rhythmic patterns in onset data
 */
class PatternFinder {
public:
    /**
     * @brief Construct a pattern finder
     * @param sampleRate Audio sample rate
     */
    PatternFinder(size_t sampleRate);
    
    /**
     * @brief Detect patterns in onset data starting from a downbeat hypothesis
     * @param onsets Onset strength data
     * @param downbeatPhase Current downbeat phase hypothesis (0.0-1.0)
     * @param beatPeriod Beat period in samples
     * @return Detected patterns
     */
    std::vector<Pattern> findPatterns(const std::deque<float>& onsets, 
                                      float downbeatPhase, 
                                      float beatPeriod);
    
    /**
     * @brief Get the strongest pattern detected
     */
    Pattern getStrongestPattern() const;
    
    /**
     * @brief Clear pattern history
     */
    void reset();
    
private:
    size_t sampleRate;
    std::vector<Pattern> recentPatterns;  // Recently detected patterns
    
    /**
     * @brief Find onset peaks in the data
     */
    std::vector<size_t> findOnsetPeaks(const std::deque<float>& onsets, float threshold) const;
    
    /**
     * @brief Calculate pattern recurrence score
     */
    float calculateRecurrence(const std::vector<size_t>& peakPositions, float period) const;
    
    /**
     * @brief Extract temporal pattern from peaks relative to downbeat
     */
    Pattern extractPattern(const std::vector<size_t>& peakPositions, 
                          float downbeatPhase, 
                          float beatPeriod) const;
};

/**
 * @brief Beat phase tracking via cross-correlation of network output and input audio
 * 
 * Compares network firing patterns with input audio onset patterns to:
 * 1. Detect beat period and tempo
 * 2. Identify downbeat phase (where beat 1 begins)
 * 3. Provide phase-dependent learning gain modulation
 */
class BeatTracker {
public:
    /**
     * @brief Construct a beat tracker
     * @param sampleRate Audio sample rate
     * @param frameSize Processing frame size
     */
    BeatTracker(size_t sampleRate, size_t frameSize);
    
    /**
     * @brief Update with new network firing and input onset data
     * @param networkFirings Vector of neuron firing indicators (1.0 if fired, 0.0 otherwise)
     * @param inputOnsets Vector of onset strengths per frequency band
     */
    void update(const std::vector<float>& networkFirings, const std::vector<float>& inputOnsets);
    
    /**
     * @brief Get current beat phase (0.0 = downbeat, 1.0 = next downbeat)
     */
    float getCurrentPhase() const { return currentPhase; }
    
    /**
     * @brief Get detected tempo in BPM
     */
    float getDetectedTempo() const { return detectedTempo; }
    
    /**
     * @brief Get learning gain multiplier based on current beat phase
     * @return Gain multiplier (1.0 = normal, higher at downbeat)
     */
    float getPhaseBasedLearningGain() const;
    
    /**
     * @brief Get confidence in current phase estimate (0.0-1.0)
     */
    float getPhaseConfidence() const { return phaseConfidence; }
    
    /**
     * @brief Enable/disable beat tracking
     */
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled; }
    
    /**
     * @brief Set global tempo reference (e.g., from auto-detect)
     * @param tempo Global tempo in BPM
     */
    void setGlobalTempo(float tempo);
    
    /**
     * @brief Set beat boost strength (multiplier at downbeat)
     */
    void setBeatBoost(float boost) { beatBoost = std::max(1.0f, boost); }
    float getBeatBoost() const { return beatBoost; }
    
    /**
     * @brief Set phase window width (how tight the temporal locking)
     * @param width Standard deviation in phase units (0.0-0.5)
     */
    void setPhaseWindow(float width) { phaseWindow = std::clamp(width, 0.01f, 0.5f); }
    float getPhaseWindow() const { return phaseWindow; }
    
    /**
     * @brief Set boost target (learning rate or neuron activation)
     */
    void setBoostTarget(BoostTarget target) { boostTarget = target; }
    BoostTarget getBoostTarget() const { return boostTarget; }
    
    /**
     * @brief Get current number of active agents
     */
    size_t getAgentCount() const { return agents.size(); }
    
    /**
     * @brief Get the strongest detected pattern
     */
    Pattern getStrongestPattern() const { 
        return patternFinder ? patternFinder->getStrongestPattern() : Pattern(); 
    }
    
    /**
     * @brief Reset beat tracking state
     */
    void reset();

private:
    size_t sampleRate;
    size_t frameSize;
    bool enabled;
    
    // Beat tracking parameters
    float currentPhase;           // Current position in beat cycle (0.0-1.0)
    float detectedTempo;          // Detected tempo in BPM
    float phaseConfidence;        // Confidence in phase estimate (0.0-1.0)
    float beatPeriodSamples;      // Beat period in samples
    
    // Learning modulation parameters
    float beatBoost;              // Learning gain multiplier at downbeat (1.0-20.0)
    float phaseWindow;            // Width of high-gain window around downbeat (0.01-0.5)
    BoostTarget boostTarget;      // Where to apply the boost (learning vs activation)
    
    // History buffers for cross-correlation
    std::deque<float> networkHistory;     // Recent network activity
    std::deque<float> inputHistory;       // Recent input onset activity
    std::deque<float> networkProcessed;   // Preprocessed network signal (bipolar: peaks and valleys)
    std::deque<float> inputProcessed;     // Preprocessed input signal (bipolar: peaks and valleys)
    size_t historyLength;                 // Samples to keep (e.g., 8 seconds)
    
    // Phase tracking state
    float phaseVelocity;          // Rate of phase change (based on tempo)
    float lastPhaseUpdate;        // Time of last phase correction
    size_t frameCounter;          // Frame counter for periodic analysis
    
    // Tempo detection parameters
    float minTempo;               // Minimum tempo to detect (40 BPM)
    float maxTempo;               // Maximum tempo to detect (320 BPM)
    float tempoSmoothingFactor;   // Smoothing for tempo changes (0.95)
    
    // Agent-based tracking system
    std::vector<std::unique_ptr<Agent>> agents;  // Active beat tracking agents
    std::unique_ptr<PatternFinder> patternFinder; // Pattern detection system
    size_t maxAgents;             // Maximum number of concurrent agents (default: 5)
    float agentSpawnThreshold;    // Confidence threshold to spawn new agent (0.6)
    float agentRemovalThreshold;  // Confidence threshold to remove agent (0.1)
    
    /**
     * @brief Perform cross-correlation analysis to detect beat period and phase
     */
    void performCrossCorrelation();
    
    /**
     * @brief Preprocess signals to emphasize both peaks (strong beats) and valleys (weak beats)
     */
    void preprocessSignalsForCorrelation();
    
    /**
     * @brief Update phase based on detected tempo
     */
    void updatePhase();
    
    /**
     * @brief Combine network firings into single activity signal
     */
    float combineNetworkActivity(const std::vector<float>& networkFirings) const;
    
    /**
     * @brief Combine input onsets into single activity signal
     */
    float combineInputActivity(const std::vector<float>& inputOnsets) const;
    
    /**
     * @brief Calculate cross-correlation at specific lag
     */
    float calculateCorrelation(int lag) const;
    
    /**
     * @brief Find peaks in correlation function
     */
    std::vector<std::pair<int, float>> findCorrelationPeaks() const;
    
    /**
     * @brief Find phase alignment based on beat period
     */
    void findPhaseAlignment(int beatPeriod);
    
    /**
     * @brief Calculate phase-based gain using Gaussian-like envelope
     */
    float calculatePhaseGain(float phase) const;
    
    /**
     * @brief Manage agent lifecycle (spawn, score, remove)
     */
    void updateAgents();
    
    /**
     * @brief Spawn new agent with given tempo and phase hypothesis
     */
    void spawnAgent(float tempo, float phase);
    
    /**
     * @brief Remove weak agents below removal threshold
     */
    void pruneWeakAgents();
    
    /**
     * @brief Get the agent with highest confidence
     */
    Agent* getBestAgent();
    
    /**
     * @brief Update global tempo and phase from best agent
     */
    void updateFromBestAgent();
};
