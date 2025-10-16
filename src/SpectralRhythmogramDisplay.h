#pragma once
#include <SFML/Graphics.hpp>
#include <deque>
#include <vector>
#include <cstdint>

// Forward declarations
class RhythmInterpreter;

/**
 * SpectralRhythmogramDisplay - Real-time spectral rhythmogram visualization
 * 
 * Displays a 2D spectrogram-like visualization of rhythmic patterns:
 * - Y-axis: Frequency bands (8 Todd frequency bands, lower frequencies at bottom)
 * - X-axis: Time (scrolling from right to left)
 * - Color intensity: Filter output amplitude (darker = higher amplitude)
 * 
 * Features:
 * - Real-time scrolling display with configurable time window
 * - Color-coded amplitude visualization with customizable color maps
 * - Frequency band labels with musical context (Phrase, Whole, Half, Quarter, etc.)
 * - Tempo-aware scaling and time markers
 * - Interactive zooming and panning
 * - Export capability for analysis
 */
class SpectralRhythmogramDisplay {
public:
    struct DisplayConfig {
        size_t timeWindowSamples = 200;     // Number of time samples to display
        float updateRate = 30.0f;           // Updates per second
        sf::Color backgroundColor = sf::Color(20, 20, 30);
        sf::Color gridColor = sf::Color(80, 80, 100, 128);
        sf::Color textColor = sf::Color::White;
        bool showGrid = true;
        bool showLabels = true;
        bool showTimeMarkers = true;
        float amplitudeScale = 1.0f;        // Scaling factor for amplitude display
        float minAmplitude = 0.0001f;       // Minimum amplitude for color mapping
        float maxAmplitude = 0.01f;         // Maximum amplitude for color mapping
    };

private:
    // Core data
    RhythmInterpreter* rhythmInterpreter;
    sf::RenderWindow* window;
    
    // Display configuration
    DisplayConfig config;
    sf::Vector2f position;
    sf::Vector2f size;
    
    // Time series data storage
    std::deque<std::vector<float>> amplitudeHistory; // [time][frequency_band]
    sf::Clock updateClock;
    
    // Rendering resources
    sf::Font font;
    bool fontLoaded;
    sf::Image spectrogramImage;
    sf::Texture spectrogramTexture;
    sf::Sprite spectrogramSprite;
    
    // Filter band information
    std::vector<std::string> frequencyBandNames;
    std::vector<float> frequencyBandCenters;
    std::vector<sf::Color> bandColors;
    
    // Rendering state
    bool needsRedraw;
    size_t currentTimeIndex;
    
    // Color mapping
    sf::Color amplitudeToColor(float amplitude, size_t bandIndex) const;
    std::uint8_t amplitudeToIntensity(float amplitude) const;
    
    // Rendering methods
    void updateSpectrogramTexture();
    void drawGrid(sf::RenderTarget& target) const;
    void drawLabels(sf::RenderTarget& target) const;
    void drawTimeMarkers(sf::RenderTarget& target) const;
    
    // Initialization
    void initializeFrequencyBands();
    void initializeColorMap();
    void loadFont();

public:
    SpectralRhythmogramDisplay(RhythmInterpreter* rhythmInterp, sf::RenderWindow* renderWindow);
    
    // Configuration
    void setPosition(float x, float y);
    void setSize(float width, float height);
    void setDisplayConfig(const DisplayConfig& newConfig);
    DisplayConfig& getConfig() { return config; }
    
    // Update and rendering
    void update();
    void render();
    
    // Data management
    void addDataPoint(const std::vector<float>& filterOutputs);
    void clear();
    void reset();
    
    // Interaction
    void handleMouseClick(float x, float y);
    void handleMouseScroll(float delta);
    void handleKeyPress(sf::Keyboard::Key key);
    
    // Utility
    void exportToImage(const std::string& filename) const;
    std::vector<float> getFrequencyBandCenters() const { return frequencyBandCenters; }
    size_t getTimeWindowSize() const { return config.timeWindowSamples; }
    
    // Analysis helpers
    float getMaxAmplitudeInBand(size_t bandIndex, size_t timeRange = 0) const;
    std::vector<float> getAverageSpectrum(size_t timeRange = 0) const;
};