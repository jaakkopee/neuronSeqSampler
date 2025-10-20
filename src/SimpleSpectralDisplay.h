#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <cstdint>
#include <deque>
#include <optional>

// Forward declarations
class RhythmInterpreter;

/**
 * SpectralRhythmogramDisplay - Basic spectral rhythmogram visualization
 * Real-time display showing frequency band outputs over time
 */

struct SimpleDisplayConfig {
    size_t timeWindowSamples = 200;  // Number of time samples to display
    float updateRate = 30.0f;        // Updates per second
    float minAmplitude = 0.0f;
    float maxAmplitude = 5e-8f;      // Adjusted for extremely small filter outputs (5e-8 = 0.00000005)
    sf::Color backgroundColor = sf::Color::Black;
    sf::Color textColor = sf::Color::White;
    bool showLabels = true;
    bool showGrid = true;
    float opacity = 100.0f;          // Opacity in percentage (0-100)
    float contrast = 1.0f;           // Contrast multiplier (0.1-3.0, default 1.0)
};

class SimpleSpectralDisplay {
public:
    SimpleSpectralDisplay(RhythmInterpreter* rhythmInterp);
    
    void setPosition(float x, float y);
    void setSize(float width, float height);
    void setRhythmInterpreter(RhythmInterpreter* rhythmInterp);
    void setOpacity(float opacity);  // Set opacity in range 0-100%
    void setContrast(float contrast); // Set contrast multiplier in range 0.1-3.0
    void setManualBPM(float bpm);    // Set manual BPM for frequency label updates
    float getOpacity() const;        // Get current opacity in range 0-100%
    void update();
    void render(sf::RenderWindow& window);
    void clear();
    
    void handleKeyPress(sf::Keyboard::Key key);
    
private:
    RhythmInterpreter* rhythmInterpreter;
    SimpleDisplayConfig config;
    
    sf::Vector2f position;
    sf::Vector2f size;
    sf::Font font;
    bool fontLoaded;
    
    std::vector<std::string> frequencyBandNames;
    std::vector<sf::Color> bandColors;
    
    sf::Clock updateClock;
    
    // Scrolling spectrogram data
    sf::Image spectrogramImage;
    sf::Texture spectrogramTexture;
    std::optional<sf::Sprite> spectrogramSprite;
    bool needsTextureUpdate;
    
    // Time history of filter outputs
    std::deque<std::vector<float>> amplitudeHistory;
    
    // Track BPM for dynamic frequency labels
    float lastKnownBPM;
    
    void initializeFrequencyBands();
    void updateFrequencyBands(); // Update labels based on current BPM
    void initializeColorMap();
    void loadFont();
    void initializeSpectrogram();
    void addDataPoint(const std::vector<float>& filterOutputs);
    void updateSpectrogramImage();
    sf::Color amplitudeToColor(float amplitude, size_t bandIndex);
    void drawBackground(sf::RenderWindow& window);
    void drawSpectrogram(sf::RenderWindow& window);
    void drawLabels(sf::RenderWindow& window);
    void drawGrid(sf::RenderWindow& window);
};