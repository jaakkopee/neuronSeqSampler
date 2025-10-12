#pragma once
#include <vector>
#include <memory>
#include <complex>
#include <SFML/Audio.hpp>

// Forward declarations
class NeuronNetwork;
class AudioManager;

/**
 * Adaptive filterbank for rhythm analysis
 * Each filter represents a different frequency band that adapts to detected rhythms
 */
class AdaptiveFilter {
private:
    float centerFrequency;
    float bandwidth;
    float adaptationRate;
    float resonance; // Q factor for filter resonance (higher = more resonant)
    std::vector<float> coefficients;
    std::vector<float> delayLine;
    float currentEnergy;
    float adaptiveGain;
    
    // Rhythmogram processing state for low-frequency detection
    float sampleCount;          // For phase tracking in rhythmogram analysis
    float smoothedOutput;       // Smoothed correlation output for rhythmogram
    
    void updateFilterCoefficients(); // Update coefficients when parameters change

public:
    AdaptiveFilter(float freq, float bw, float adaptRate = 0.1f, float res = 1.0f);
    
    float process(float input);
    void adaptToRhythm(float rhythmStrength);
    
    // Getters
    float getCenterFrequency() const { return centerFrequency; }
    float getBandwidth() const { return bandwidth; }
    float getResonance() const { return resonance; }
    float getCurrentEnergy() const { return currentEnergy; }
    float getAdaptiveGain() const { return adaptiveGain; }
    
    // Setters for real-time control
    void setCenterFrequency(float freq);
    void setBandwidth(float bw);
    void setResonance(float res);
    void setAdaptationRate(float rate) { adaptationRate = rate; }
};

/**
 * Multi-band rhythm detection and analysis
 * Detects tempo, beat strength, and rhythmic patterns
 */
class RhythmDetector {
private:
    static constexpr size_t BUFFER_SIZE = 1024;
    static constexpr size_t HISTORY_LENGTH = 100;
    
    std::vector<float> audioBuffer;
    std::vector<float> onsetBuffer;
    std::vector<float> tempoHistory;
    
    float currentTempo;
    float beatStrength;
    float rhythmicComplexity;
    
    // Analysis parameters
    float onsetThreshold;
    float tempoSmoothingFactor;
    
    void detectOnsets();
    void analyzeTempo();
    void calculateComplexity();

public:
    RhythmDetector();
    
    void processAudioChunk(const std::vector<float>& audioData);
    void reset();
    
    // Getters
    float getCurrentTempo() const { return currentTempo; }
    float getBeatStrength() const { return beatStrength; }
    float getRhythmicComplexity() const { return rhythmicComplexity; }
    
    // Analysis results
    bool isBeatDetected() const;
    float getGrooveStrength() const;
    std::vector<float> getRhythmPattern() const;
};

/**
 * Connection matrix between filterbank outputs and neural network inputs
 * Allows flexible routing of rhythm analysis to specific neurons
 */
class ConnectionMatrix {
private:
    std::vector<std::vector<float>> weights; // [filterIndex][neuronIndex]
    size_t numFilters;
    size_t numNeurons;
    
    // Learning parameters
    float learningRate;
    bool adaptiveMode;
    
public:
    ConnectionMatrix(size_t filters, size_t neurons, float lr = 0.01f);
    
    // Weight management
    void setWeight(size_t filterIndex, size_t neuronIndex, float weight);
    float getWeight(size_t filterIndex, size_t neuronIndex) const;
    
    // Matrix operations
    std::vector<float> transform(const std::vector<float>& filterOutputs, float rhythmogramScale = 500.0f) const;
    void adaptWeights(const std::vector<float>& filterOutputs, 
                      const std::vector<float>& neuronFeedback);
    
    // Configuration
    void setLearningRate(float rate) { learningRate = rate; }
    void setAdaptiveMode(bool enabled) { adaptiveMode = enabled; }
    void randomizeWeights(float minWeight = -0.5f, float maxWeight = 0.5f);
    void clearWeights();
    void resizeMatrix(size_t newNumNeurons);
    
    // Getters
    size_t getNumFilters() const { return numFilters; }
    size_t getNumNeurons() const { return numNeurons; }
    bool isAdaptiveMode() const { return adaptiveMode; }
};

/**
 * Main rhythm interpreter class
 * Coordinates audio analysis, filtering, and network feedback
 */
class RhythmInterpreter {
private:
    // Core components
    std::vector<std::unique_ptr<AdaptiveFilter>> filterBank;
    std::unique_ptr<RhythmDetector> rhythmDetector;
    std::unique_ptr<ConnectionMatrix> connectionMatrix;
    
    // Network integration
    NeuronNetwork* neuronNetwork;
    AudioManager* audioManager;
    
    // Audio processing
    std::vector<float> audioBuffer;
    std::vector<float> filterOutputs;
    std::vector<float> neuronInputs;
    std::vector<float> processedAudioBuffer; // Buffer for processed audio output
    
    // Configuration
    size_t sampleRate;
    size_t bufferSize;
    bool enabled;
    float globalGain; // Should be 0 for analysis-only mode, >0 for filtered audio output
    std::vector<float> filterGains; // User-controlled gain for each filter band
    std::vector<bool> filterSoloEnabled; // Solo/listen state for each filter band
    bool anyFilterSoloed; // True if any filter is currently soloed
    bool audioOutputEnabled; // Enable filtered audio output
    float rhythmogramScale; // Scaling factor for rhythmogram to neural activation (0.0-20.0, default 5.0)
    float bpm; // Beats per minute for tempo-relative frequency scaling (30.0-260.0, default 120.0)
    
    // Frequency bands for filterbank (in Hz)
    static const std::vector<float> DEFAULT_FREQUENCIES;
    static const std::vector<float> DEFAULT_BANDWIDTHS;
    static const std::vector<float> DEFAULT_RESONANCES;
    
    void initializeFilterBank();
    void processAudioBuffer();
    void updateNeuronInputs();
    void updateFilterBankForBPM(); // Update filter frequencies based on current BPM

public:
    RhythmInterpreter(NeuronNetwork* network, AudioManager* audioMgr, 
                     size_t sampleRate = 44100, size_t bufferSize = 512);
    ~RhythmInterpreter() = default;
    
    // Main processing
    void processAudioFrame(const std::vector<float>& audioData);
    void update(); // Called each network tick
    
    // Configuration
    void setEnabled(bool enable) { enabled = enable; }
    void setGlobalGain(float gain) { globalGain = gain; }
    void setSampleRate(size_t rate) { sampleRate = rate; }
    
    // Filter bank control
    void setFilterFrequency(size_t filterIndex, float frequency);
    void setFilterBandwidth(size_t filterIndex, float bandwidth);
    void setFilterAdaptation(size_t filterIndex, float rate);
    
    // Connection matrix control
    void setConnectionWeight(size_t filterIndex, size_t neuronIndex, float weight);
    void enableAdaptiveConnections(bool enable);
    void randomizeConnections();
    void updateNetworkSize(); // Update connection matrix size to match network
    
    // Analysis access
    const RhythmDetector* getRhythmDetector() const { return rhythmDetector.get(); }
    const ConnectionMatrix* getConnectionMatrix() const { return connectionMatrix.get(); }
    ConnectionMatrix* getConnectionMatrix() { return connectionMatrix.get(); }
    
    // Filter gain control
    void setFilterGain(size_t filterIndex, float gain);
    float getFilterGain(size_t filterIndex) const;
    
    // Filter solo/listen control
    void setFilterSolo(size_t filterIndex, bool solo);
    bool getFilterSolo(size_t filterIndex) const;
    void clearAllSolos(); // Turn off all solo states
    std::vector<float> getSoloedFilterOutput() const; // Get mixed output of only soloed filters
    std::vector<float> getProcessedAudioOutput() const; // Get the processed audio suitable for playback
    void setAudioOutputEnabled(bool enabled) { audioOutputEnabled = enabled; }
    bool isAudioOutputEnabled() const { return audioOutputEnabled; }
    
    // Filter resonance control
    void setFilterResonance(size_t filterIndex, float resonance);
    float getFilterResonance(size_t filterIndex) const;
    
    // Rhythmogram scale control
    void setRhythmogramScale(float scale);
    float getRhythmogramScale() const { return rhythmogramScale; }
    
    // BPM control (tempo-relative frequency scaling)
    void setBPM(float beatsPerMinute);
    float getBPM() const { return bpm; }
    
    // Getters
    bool isEnabled() const { return enabled; }
    float getGlobalGain() const { return globalGain; }
    size_t getNumFilters() const { return filterBank.size(); }
    std::vector<float> getFilterOutputs() const { return filterOutputs; }
    std::vector<float> getCurrentNeuronInputs() const { return neuronInputs; }
    
    // Real-time status
    bool isRhythmDetected() const;
    float getCurrentTempo() const;
    float getOverallRhythmStrength() const;
};