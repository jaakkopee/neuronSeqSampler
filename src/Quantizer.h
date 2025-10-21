#pragma once
#include <vector>
#include <cstddef>
#include <chrono>
#include <string>

/**
 * @brief Musical timing quantization system
 * 
 * The Quantizer class provides real-time musical quantization with swing support.
 * It can quantize events to musical grid subdivisions (half note to 64th note)
 * with adjustable quantization strength and swing timing.
 */
class Quantizer {
public:
    /**
     * @brief Available quantization grid subdivisions
     */
    enum class GridResolution {
        HALF_NOTE = 0,    // 1/2 note subdivisions
        QUARTER_NOTE = 1, // 1/4 note subdivisions (beat)
        EIGHTH_NOTE = 2,  // 1/8 note subdivisions
        SIXTEENTH_NOTE = 3, // 1/16 note subdivisions
        THIRTY_SECOND_NOTE = 4, // 1/32 note subdivisions
        SIXTY_FOURTH_NOTE = 5   // 1/64 note subdivisions
    };

    /**
     * @brief Event structure for quantization
     */
    struct Event {
        double timestamp;    // Event time in seconds
        int noteIndex;      // Note/sample index
        float velocity;     // Event velocity (0.0-1.0)
        bool isQuantized;   // Whether this event has been quantized
        
        Event(double time, int note, float vel = 1.0f) 
            : timestamp(time), noteIndex(note), velocity(vel), isQuantized(false) {}
    };

    /**
     * @brief Construct a new Quantizer
     * @param bpm Beats per minute for timing calculations
     * @param sampleRate Audio sample rate for precise timing
     */
    Quantizer(float bpm = 120.0f, int sampleRate = 44100);

    // ========================= CORE QUANTIZATION =========================
    
    /**
     * @brief Quantize a single event to the current grid
     * @param event Event to quantize (modified in place)
     * @return True if event was quantized, false if no change needed
     */
    bool quantizeEvent(Event& event);
    
    /**
     * @brief Quantize multiple events
     * @param events Vector of events to quantize
     * @return Number of events that were quantized
     */
    size_t quantizeEvents(std::vector<Event>& events);
    
    /**
     * @brief Get the next quantization grid point for a given time
     * @param timestamp Current time in seconds
     * @return Next grid point time in seconds
     */
    double getNextGridPoint(double timestamp) const;
    
    /**
     * @brief Get the previous quantization grid point for a given time
     * @param timestamp Current time in seconds
     * @return Previous grid point time in seconds
     */
    double getPreviousGridPoint(double timestamp) const;
    
    /**
     * @brief Get the nearest quantization grid point for a given time
     * @param timestamp Current time in seconds
     * @return Nearest grid point time in seconds (considering swing)
     */
    double getNearestGridPoint(double timestamp) const;

    // ========================= PARAMETER CONTROLS =========================
    
    /**
     * @brief Set the quantization grid resolution
     * @param resolution Grid subdivision to use
     */
    void setGridResolution(GridResolution resolution);
    
    /**
     * @brief Get current grid resolution
     * @return Current grid subdivision
     */
    GridResolution getGridResolution() const { return gridResolution; }
    
    /**
     * @brief Set quantization amount/strength
     * @param amount Quantization strength (0.0 = no quantization, 1.0 = full quantization)
     */
    void setQuantizationAmount(float amount);
    
    /**
     * @brief Get current quantization amount
     * @return Quantization strength (0.0-1.0)
     */
    float getQuantizationAmount() const { return quantizationAmount; }
    
    /**
     * @brief Set swing factor for groove timing
     * @param swing Swing amount (-1.0 to 1.0, 0.0 = straight timing)
     */
    void setSwingFactor(float swing);
    
    /**
     * @brief Get current swing factor
     * @return Swing amount (-1.0 to 1.0)
     */
    float getSwingFactor() const { return swingFactor; }
    
    /**
     * @brief Set tempo for timing calculations
     * @param bpm Beats per minute
     */
    void setBPM(float bpm);
    
    /**
     * @brief Get current tempo
     * @return Beats per minute
     */
    float getBPM() const { return currentBPM; }

    // ========================= UTILITY METHODS =========================
    
    /**
     * @brief Enable or disable quantization
     * @param enabled Whether quantization should be active
     */
    void setEnabled(bool enabled) { isEnabled = enabled; }
    
    /**
     * @brief Check if quantization is enabled
     * @return True if quantization is active
     */
    bool isQuantizationEnabled() const { return isEnabled; }
    
    /**
     * @brief Get grid interval in seconds for current resolution
     * @return Time between grid points in seconds
     */
    double getGridInterval() const;
    
    /**
     * @brief Get readable name for current grid resolution
     * @return Human-readable grid name (e.g., "1/4 Note")
     */
    std::string getGridResolutionName() const;
    
    /**
     * @brief Reset quantization timing (e.g., after tempo change)
     */
    void reset();

private:
    // ========================= TIMING PARAMETERS =========================
    float currentBPM;           // Current tempo in beats per minute
    int sampleRate;             // Audio sample rate for timing precision
    GridResolution gridResolution; // Current quantization grid
    float quantizationAmount;   // Quantization strength (0.0-1.0)
    float swingFactor;          // Swing timing (-1.0 to 1.0)
    bool isEnabled;             // Whether quantization is active
    
    // ========================= TIMING CALCULATIONS =========================
    double quarterNoteInterval; // Time for one quarter note in seconds
    
    // ========================= INTERNAL METHODS =========================
    
    /**
     * @brief Update internal timing calculations after parameter changes
     */
    void updateTimingCalculations();
    
    /**
     * @brief Apply swing to a grid position
     * @param gridPosition Normalized position within a beat (0.0-1.0)
     * @return Swing-adjusted position
     */
    double applySwing(double gridPosition) const;
    
    /**
     * @brief Apply swing timing to a specific grid point
     * @param gridPoint Time of the grid point in seconds
     * @param gridInterval Interval between grid points in seconds
     * @return Swing-adjusted grid point time
     */
    double applySwingToGridPoint(double gridPoint, double gridInterval) const;
    
    /**
     * @brief Get the subdivision factor for current grid resolution
     * @return Number of subdivisions per quarter note
     */
    float getSubdivisionFactor() const;
    
    /**
     * @brief Interpolate between original and quantized time
     * @param originalTime Original event time
     * @param quantizedTime Fully quantized time
     * @return Partially quantized time based on quantizationAmount
     */
    double interpolateQuantization(double originalTime, double quantizedTime) const;
};