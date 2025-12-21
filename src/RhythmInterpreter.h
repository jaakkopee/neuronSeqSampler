#pragma once
#include <vector>
#include <cstddef>
#include <complex>
#include <fftw3.h>

/**
 * @brief FFT-based Gamma-tone filterbank for frequency analysis
 * 
 * Implements a high-performance bank of gamma-tone filters using FFTW3
 * for real-time audio frequency decomposition with proper gamma-tone characteristics
 */
class GammaToneFilterBank {
public:
    GammaToneFilterBank(size_t sampleRate, size_t bandCount);
    ~GammaToneFilterBank();
    
    std::vector<float> process(const std::vector<float>& input);
    std::vector<float> FFTProcess(const std::vector<float>& input);
    void setBandFrequency(size_t bandIndex, float frequency);
    void setBandBandwidth(size_t bandIndex, float bandwidth);
    
private:
    size_t sampleRate;
    size_t bandCount;
    size_t fftSize;
    
    // FFTW3 structures
    double* fftInput;
    fftw_complex* fftOutput;
    fftw_complex* ifftInput;
    double* ifftOutput;
    fftw_plan forwardPlan;
    fftw_plan inversePlan;
    
    // Gamma-tone filter characteristics
    struct GammaToneBand {
        float centerFrequency;     // Center frequency in Hz
        float bandwidth;          // Bandwidth in Hz  
        float gain;              // Filter gain
        int order;               // Gamma-tone filter order (typically 4)
        std::vector<std::complex<double>> frequencyResponse; // Pre-computed frequency domain response
        
        // Legacy state variables (for time-domain fallback)
        float prevInput1;
        float prevInput2;
        float prevOutput1;
        float prevOutput2;
    };
    std::vector<GammaToneBand> bands;
    
    // Buffer for overlap-add processing
    std::vector<float> overlapBuffer;
    
    // Internal methods
    void initializeFFTW();
    void generateGammaToneResponse(size_t bandIndex);
    void cleanupFFTW();
    std::complex<double> gammaToneFrequencyResponse(double frequency, const GammaToneBand& band);
};

/**
 * @brief Advanced rhythm detection and analysis engine
 * 
 * The RhythmInterpreter processes audio data through 8 frequency bands designed
 * for hierarchical rhythm detection following Todd (1994) rhythmogram theory.
 * Features direct user sensitivity control and real-time parameter control.
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

    // ========================= BAND COUNT CONTROL =========================
    /**
     * @brief Set number of rhythmogram bands with logarithmic frequency spacing
     * @param count Number of bands (e.g., 4-16)
     */
    void setBandCount(size_t count);
    
    // ========================= FILTER GAIN CONTROLS =========================
    
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
    std::vector<float> getBandFrequencies() const;
    
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
    
    // ========================= GLOBAL SCALE CONTROLS =========================
    
    // Global rhythmogram scale (applies to all bands)
    void setRhythmogramScale(float scale);
    float getRhythmogramScale() const;
    
    // ========================= Tempo and Beat Detection Controls =========================

    // Set the tempo detection sensitivity
    void setTempoSensitivity(float sensitivity);
    float getTempoSensitivity() const;

    // Get the current detected tempo
    float getDetectedTempo() const;

    // Get the current beat positions
    std::vector<float> getBeatPositions() const;

    // Auto-tempo following controls
    void setAutoTempoEnabled(bool enabled);
    bool isAutoTempoEnabled() const;
    void setBaseTempoFrequency(float frequency);
    float getBaseTempoFrequency() const;

    // ========================= UTILITY METHODS =========================
    
    /**
     * @brief Get number of frequency bands
     * @return Band count (always 8)
     */
    size_t getBandCount() const { return bandCount; }

private:
    // ========================= CONFIGURATION CONSTANTS =========================
    static constexpr size_t DEFAULT_BAND_COUNT = 8;
    
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

    // Tempo and beat detection parameters
    float tempoSensitivity;
    float detectedTempo;
    std::vector<float> beatPositions;
    
    // Auto-tempo following parameters
    bool autoTempoEnabled;
    float baseTempoFrequency;             // Base frequency for 120 BPM
    std::vector<float> defaultFrequencies; // Original band frequencies for scaling
    float tempoSmoothingFactor;           // Smoothing for tempo changes
    float lastStableTempo;                // Previously detected stable tempo

    // ========================= USER CONTROLS =========================
    std::vector<float> bandGains;            // User sensitivity settings
    std::vector<float> filterGains;          // User output gain settings
    float rhythmogramScale;                  // Global scaling multiplier for all bands
    
    // ========================= RUNTIME STATE =========================  
    std::vector<float> filterOutputs;        // Current output levels
    std::vector<int> stuckCounters;          // Anti-stuck mechanism counters

    //Gamma Tone Filter Array
    GammaToneFilterBank GTFilterBank;

    // ========================= CORE PROCESSING METHODS =========================
    
    /**
     * @brief Initialize all band parameters to default values
     */
    void initializeBands();
    void initializeBandsLogarithmic(float minFreq, float maxFreq, size_t count);
    
    // Removed updateAdaptiveSensitivity and applyContrastEnhancement methods
    
    // ========================= SIGNAL PROCESSING METHODS =========================
    
    /**
     * @brief Apply bandpass filtering to audio data
     * @param data Input audio samples
     * @param freq Center frequency
     * @param bw Bandwidth
     * @return Filtered audio samples
     */
    std::vector<float> bandpassFilter(const std::vector<float>& data, float freq, float bw, float q);
    
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