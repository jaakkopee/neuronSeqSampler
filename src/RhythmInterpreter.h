#pragma once
#include <vector>
#include <cstddef>

/**
 * @brief Advanced rhythm detection and analysis engine
 * 
 * The RhythmInterpreter processes audio data through 8 frequency bands designed
 * for hierarchical rhythm detection following Todd (1994) rhythmogram theory.
 * Features adaptive sensitivity, contrast enhancement, and real-time parameter control.
 */
class RhythmInterpreter {
public:
    // ========================= CORE INTERFACE =========================
    
    /**
     * @brief Construct a new Rhythm Interpreter
     * @param sampleRate Audio sample rate in Hz
     * @param bufferSize Audio buffer size in samples  
     */
    RhythmInterpreter(size_t sampleRate, size_t bufferSize);
    
    /**
     * @brief Process a frame of audio data for rhythm detection
     * @param audioData Input audio samples
     */
    void processAudioFrame(const std::vector<float>& audioData);
    
    /**
     * @brief Get current filter output levels for all bands
     * @return Vector of 8 filter output values (0.0-1.0 range)
     */
    std::vector<float> getFilterOutputs() const;
    
    // ========================= SENSITIVITY CONTROLS =========================
    
    /**
     * @brief Set sensitivity for a specific frequency band
     * @param bandIndex Band index (0-7)
     * @param gain Sensitivity multiplier (-3.0 to 10.0)
     */
    void setSensitivity(size_t bandIndex, float gain);
    
    /**
     * @brief Get effective sensitivity (user × adaptive)
     * @param bandIndex Band index (0-7) 
     * @return Combined sensitivity value
     */
    float getSensitivity(size_t bandIndex) const;
    
    /**
     * @brief Set output filter gain for a specific band
     * @param bandIndex Band index (0-7)
     * @param gain Output gain multiplier (0.0-5.0)
     */
    void setFilterGain(size_t bandIndex, float gain);
    
    /**
     * @brief Get current filter gain for a specific band
     * @param bandIndex Band index (0-7)
     * @return Current gain value
     */
    float getFilterGain(size_t bandIndex) const;
    
    // ========================= BAND PARAMETER CONTROLS =========================
    
    // Frequency control (center frequency of each band)
    void setBandFrequency(size_t bandIndex, float frequency);
    float getBandFrequency(size_t bandIndex) const;
    
    // Bandwidth control (frequency range width)
    void setBandBandwidth(size_t bandIndex, float bandwidth);
    float getBandBandwidth(size_t bandIndex) const;
    
    // Scaling control (pre-amplification)
    void setBandScaling(size_t bandIndex, float scaling);
    float getBandScaling(size_t bandIndex) const;
    
    // Limit control (maximum output level)
    void setBandLimit(size_t bandIndex, float limit);
    float getBandLimit(size_t bandIndex) const;
    
    // Q-factor control (filter resonance/sharpness)
    void setQValue(size_t bandIndex, float q);
    float getQValue(size_t bandIndex) const;
    
    // ========================= ADAPTIVE SYSTEM MONITORING =========================
    
    /**
     * @brief Get current adaptive sensitivity multiplier
     * @param bandIndex Band index (0-7)
     * @return Auto-calculated sensitivity (read-only)
     */
    float getAdaptiveSensitivity(size_t bandIndex) const;
    
    /**
     * @brief Get energy baseline (average quiet level)
     * @param bandIndex Band index (0-7)
     * @return Running baseline energy
     */
    float getEnergyBaseline(size_t bandIndex) const;
    
    /**
     * @brief Get recent peak energy level
     * @param bandIndex Band index (0-7)
     * @return Recent maximum energy
     */
    float getEnergyPeak(size_t bandIndex) const;
    
    /**
     * @brief Get estimated noise floor
     * @param bandIndex Band index (0-7)
     * @return Current noise floor estimate
     */
    float getNoiseFloor(size_t bandIndex) const;
    
    /**
     * @brief Get current dynamic range (peak - baseline)
     * @param bandIndex Band index (0-7)
     * @return Calculated dynamic range
     */
    float getDynamicRange(size_t bandIndex) const;

private:
    // ========================= CONFIGURATION CONSTANTS =========================
    static constexpr size_t DEFAULT_BAND_COUNT = 8;
    static constexpr float ADAPTATION_RATE = 0.001f;
    static constexpr float PEAK_DECAY = 0.999f;
    static constexpr float BASELINE_RATE = 0.0005f;
    static constexpr int ADAPTATION_INTERVAL = 100;  // frames
    
    // ========================= CORE SYSTEM STATE =========================
    size_t sampleRate;
    size_t bufferSize;
    size_t bandCount;
    int frameCounter;
    
    // ========================= BAND CONFIGURATION PARAMETERS =========================
    std::vector<float> bandFrequencies;      // Center frequencies (Hz)
    std::vector<float> bandBandwidths;       // Frequency band widths (Hz)  
    std::vector<float> bandScalings;         // Pre-amplification factors
    std::vector<float> bandLimits;           // Maximum output levels
    std::vector<float> qValues;              // Filter Q-factors (resonance)
    
    // ========================= USER CONTROLS =========================
    std::vector<float> bandGains;            // User sensitivity settings
    std::vector<float> filterGains;          // User output gain settings
    
    // ========================= RUNTIME STATE =========================  
    std::vector<float> filterOutputs;        // Current output levels
    std::vector<int> stuckCounters;          // Anti-stuck mechanism counters
    
    // ========================= ADAPTIVE SENSITIVITY SYSTEM =========================
    std::vector<float> adaptiveSensitivities; // Auto-calculated multipliers
    std::vector<float> energyBaselines;       // Running quiet-period averages
    std::vector<float> energyPeaks;          // Recent maximum levels
    std::vector<float> noiseFloors;          // Estimated noise floors
    std::vector<float> dynamicRanges;        // Calculated dynamic ranges  
    std::vector<int> adaptationCounters;     // Adaptation timing counters
    
    // ========================= CORE PROCESSING METHODS =========================
    
    /**
     * @brief Initialize all band parameters to default values
     */
    void initializeBands();
    
    /**
     * @brief Update adaptive sensitivity for a specific band
     * @param bandIndex Band to update
     * @param rawEnergy Current raw energy level
     */
    void updateAdaptiveSensitivity(size_t bandIndex, float rawEnergy);
    
    /**
     * @brief Apply contrast enhancement algorithms
     * @param bandIndex Band to enhance
     * @param energy Input energy level
     * @return Enhanced energy with improved contrast
     */
    float applyContrastEnhancement(size_t bandIndex, float energy);
    
    // ========================= SIGNAL PROCESSING METHODS =========================
    
    /**
     * @brief Apply bandpass filtering to audio data
     * @param data Input audio samples
     * @param freq Center frequency
     * @param bw Bandwidth
     * @return Filtered audio samples
     */
    std::vector<float> bandpassFilter(const std::vector<float>& data, float freq, float bw);
    
    /**
     * @brief Zero-crossing based onset detection
     * @param data Input audio samples  
     * @param bandIndex Band index for parameter lookup
     * @return Onset detection output
     */
    std::vector<float> zeroCrossingOnsetDetection(const std::vector<float>& data, int bandIndex);
    
    /**
     * @brief Envelope detection and smoothing
     * @param data Input audio samples
     * @param bandIndex Band index for parameter lookup  
     * @return Smoothed envelope
     */
    std::vector<float> envelopeDetection(const std::vector<float>& data, int bandIndex);
    
    // ========================= UTILITY METHODS =========================
    
    /**
     * @brief Validate band index is within valid range
     * @param bandIndex Index to validate
     * @return True if valid
     */
    inline bool isValidBandIndex(size_t bandIndex) const {
        return bandIndex < bandCount;
    }
};
