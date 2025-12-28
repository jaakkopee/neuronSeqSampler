#pragma once
#include <vector>
#include <deque>
#include <cmath>
#include <algorithm>

enum class BoostTarget {
    Learning,      // Apply phase boost to learning rate
    Activation     // Apply phase boost to neuron activations
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
    void setEnabled(bool enabled) { this->enabled = enabled; }
    bool isEnabled() const { return enabled; }
    
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
    size_t historyLength;                 // Samples to keep (e.g., 8 seconds)
    
    // Phase tracking state
    float phaseVelocity;          // Rate of phase change (based on tempo)
    float lastPhaseUpdate;        // Time of last phase correction
    size_t frameCounter;          // Frame counter for periodic analysis
    
    // Tempo detection parameters
    float minTempo;               // Minimum tempo to detect (40 BPM)
    float maxTempo;               // Maximum tempo to detect (200 BPM)
    float tempoSmoothingFactor;   // Smoothing for tempo changes (0.95)
    
    /**
     * @brief Perform cross-correlation analysis to detect beat period and phase
     */
    void performCrossCorrelation();
    
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
};
