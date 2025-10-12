#pragma once
#include <vector>
#include <memory>
#include <deque>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <complex>
#include <cstring>
#include <limits>

/**
 * BeatRoot Implementation based on Dixon (2007)
 * "Evaluation of the Audio Beat Tracking System BeatRoot"
 * 
 * This implementation includes:
 * 1. Spectral Flux Onset Detection
 * 2. IOI (Inter-Onset Interval) Clustering for Tempo Induction
 * 3. Multi-Agent Beat Tracking with competing hypotheses
 */

// Forward declarations
class NeuronNetwork;

/**
 * Onset Detection using Spectral Flux
 * SF(n) = sum over frequency bins of max(0, |X(n,k)| - |X(n-1,k)|)
 */
class OnsetDetector {
private:
    static constexpr size_t FFT_SIZE = 1024;
    static constexpr size_t HOP_SIZE = 256;
    static constexpr float SAMPLE_RATE = 44100.0f;
    
    std::vector<float> audioBuffer;
    std::vector<float> previousEnergy;
    std::vector<float> currentEnergy;
    std::vector<float> onsetFunction;
    
    float onsetThreshold;
    float adaptiveThreshold;
    std::vector<float> onsetTimes;
    
    void computeEnergySpectrum(const std::vector<float>& input);
    void updateAdaptiveThreshold(float currentValue);
    
public:
    OnsetDetector();
    
    void processAudioFrame(const std::vector<float>& audioData);
    void detectOnsets();
    
    // Getters
    const std::vector<float>& getOnsetFunction() const { return onsetFunction; }
    const std::vector<float>& getOnsetTimes() const { return onsetTimes; }
    float getLatestOnsetStrength() const;
    bool hasRecentOnset(float timeWindow = 0.1f) const;
    
    // Configuration
    void setOnsetThreshold(float threshold) { onsetThreshold = threshold; }
    float getOnsetThreshold() const { return onsetThreshold; }
    void reset();
};

/**
 * IOI Clustering for Tempo Induction
 * Groups inter-onset intervals into tempo clusters
 */
class TempoInductor {
private:
    struct IOICluster {
        float meanInterval;     // Mean IOI in seconds
        float weight;          // Cluster weight/strength
        int count;             // Number of IOIs in cluster
        float tempo;           // Corresponding tempo in BPM
        
        IOICluster(float interval) : meanInterval(interval), weight(1.0f), count(1) {
            tempo = 60.0f / interval; // Convert to BPM
        }
    };
    
    std::vector<float> interOnsetIntervals;
    std::vector<IOICluster> clusters;
    float currentTempo;
    float tempoStrength;
    
    static constexpr float MIN_TEMPO = 30.0f;  // Minimum BPM
    static constexpr float MAX_TEMPO = 300.0f; // Maximum BPM
    static constexpr float CLUSTER_TOLERANCE = 0.025f; // 2.5% tolerance for clustering
    static constexpr size_t MAX_CLUSTERS = 10;
    static constexpr size_t MIN_IOIS_FOR_TEMPO = 3; // Minimum IOIs needed for tempo estimation
    
    void clusterIOIs();
    void updateClusters(float newIOI);
    float calculateClusterWeight(const IOICluster& cluster) const;
    
public:
    TempoInductor();
    
    void addOnsetTime(float time);
    void updateTempo();
    
    // Getters
    float getCurrentTempo() const { return currentTempo; }
    float getTempoStrength() const { return tempoStrength; }
    const std::vector<IOICluster>& getClusters() const { return clusters; }
    
    // Analysis
    bool hasStableTempo() const;
    std::vector<float> getTopTempos(size_t count = 3) const;
    
    void reset();
};

/**
 * Beat Tracking Agent
 * Each agent tracks a specific beat hypothesis
 */
class BeatAgent {
private:
    float beatInterval;    // Beat interval in seconds (60/BPM)
    float phase;          // Current phase (0.0 to beatInterval)
    float score;          // Agent score/confidence
    float tolerance;      // Tolerance window around beats
    float lastBeatTime;   // Time of last predicted beat
    
    std::vector<float> beatTimes; // History of beat predictions
    bool isActive;
    
    static constexpr float SCORE_DECAY = 0.9f;     // Score decay per frame
    static constexpr float MIN_SCORE = 0.1f;       // Minimum score to stay active
    static constexpr float ONSET_REWARD = 1.0f;    // Reward for onset near beat
    static constexpr float ONSET_PENALTY = -0.5f;  // Penalty for onset far from beat
    
public:
    BeatAgent(float tempo, float initialPhase = 0.0f);
    
    void update(float deltaTime, const std::vector<float>& onsetTimes, float currentTime);
    void scoreOnsets(const std::vector<float>& onsetTimes, float currentTime);
    
    bool isPredictingBeat(float currentTime, float tolerance) const;
    float getNextBeatTime(float currentTime) const;
    
    // Getters/Setters
    float getScore() const { return score; }
    float getBeatInterval() const { return beatInterval; }
    float getTempo() const { return 60.0f / beatInterval; }
    float getPhase() const { return phase; }
    bool getIsActive() const { return isActive; }
    const std::vector<float>& getBeatTimes() const { return beatTimes; }
    
    void setTempo(float bpm) { beatInterval = 60.0f / bpm; }
    void adjustPhase(float adjustment) { phase += adjustment; }
    void reset();
};

/**
 * Multi-Agent Beat Tracker
 * Manages multiple competing beat tracking agents
 */
class BeatTracker {
private:
    std::vector<std::unique_ptr<BeatAgent>> agents;
    std::unique_ptr<BeatAgent> winningAgent;
    
    float currentTime;
    float lastBeatTime;
    float currentTempo;
    float beatStrength;
    
    static constexpr size_t MAX_AGENTS = 8;
    static constexpr float AGENT_SPAWN_THRESHOLD = 0.7f; // Tempo strength needed to spawn agent
    static constexpr float BEAT_TOLERANCE = 0.05f;       // 50ms tolerance window
    
    void spawnAgent(float tempo, float phase = 0.0f);
    void cullWeakAgents();
    void updateWinningAgent();
    
public:
    BeatTracker();
    
    void update(float deltaTime, const std::vector<float>& onsetTimes, 
                const std::vector<float>& tempos, const std::vector<float>& tempoStrengths);
    
    void initializeFromTempo(float tempo, float firstBeatTime = 0.0f);
    
    // Beat prediction
    bool isBeatPredicted(float tolerance = BEAT_TOLERANCE) const;
    float getNextBeatTime() const;
    float getCurrentBeatStrength() const;
    
    // Getters
    float getCurrentTempo() const { return currentTempo; }
    float getBeatStrength() const { return beatStrength; }
    const BeatAgent* getWinningAgent() const { return winningAgent.get(); }
    size_t getNumActiveAgents() const;
    
    void reset();
};

/**
 * Main BeatRoot System
 * Coordinates onset detection, tempo induction, and beat tracking
 */
class BeatRoot {
private:
    std::unique_ptr<OnsetDetector> onsetDetector;
    std::unique_ptr<TempoInductor> tempoInductor;
    std::unique_ptr<BeatTracker> beatTracker;
    
    // Integration with existing system
    NeuronNetwork* neuronNetwork;
    
    // State
    float currentTime;
    bool isInitialized;
    bool beatDetectedThisFrame;
    float lastBeatTime;
    
    // Configuration
    bool enabled;
    float sensitivity;          // Overall system sensitivity
    bool autoInitialize;        // Auto-initialize from tempo detection
    float minInitializationTime; // Minimum time before auto-initialization
    
    // Output smoothing
    std::deque<float> tempoHistory;
    std::deque<float> beatStrengthHistory;
    static constexpr size_t HISTORY_SIZE = 10;
    
public:
    BeatRoot(NeuronNetwork* network = nullptr);
    ~BeatRoot() = default;
    
    // Main processing
    void processAudioFrame(const std::vector<float>& audioData, float deltaTime);
    void update(float deltaTime);
    
    // Manual initialization
    void initialize(float tempo, float firstBeatTime = 0.0f);
    void reset();
    
    // Beat information
    bool isBeatDetected() const { return beatDetectedThisFrame; }
    float getCurrentTempo() const;
    float getBeatStrength() const;
    float getNextBeatTime() const;
    
    // Onset information  
    float getOnsetStrength() const;
    bool hasRecentOnset(float timeWindow = 0.1f) const;
    
    // Configuration
    void setEnabled(bool enable) { enabled = enable; }
    bool isEnabled() const { return enabled; }
    
    void setSensitivity(float sens) { sensitivity = std::clamp(sens, 0.1f, 2.0f); }
    float getSensitivity() const { return sensitivity; }
    
    void setAutoInitialize(bool enable) { autoInitialize = enable; }
    bool getAutoInitialize() const { return autoInitialize; }
    
    // Component access (for debugging/advanced control)
    const OnsetDetector* getOnsetDetector() const { return onsetDetector.get(); }
    const TempoInductor* getTempoInductor() const { return tempoInductor.get(); }
    const BeatTracker* getBeatTracker() const { return beatTracker.get(); }
    
    // Analysis
    std::vector<float> getTopTempos(size_t count = 3) const;
    bool hasStableTempo() const;
    size_t getNumActiveAgents() const;
    
    // Smoothed outputs
    float getSmoothedTempo() const;
    float getSmoothedBeatStrength() const;
};