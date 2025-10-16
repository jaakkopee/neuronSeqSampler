#include "SpectralRhythmogramDisplay.h"
#include "RhythmInterpreter.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <cstdint>

SpectralRhythmogramDisplay::SpectralRhythmogramDisplay(RhythmInterpreter* rhythmInterp, sf::RenderWindow* renderWindow)
    : rhythmInterpreter(rhythmInterp)
    , window(renderWindow)
    , position(0, 0)
    , size(400, 300)
    , fontLoaded(false)
    , needsRedraw(true)
    , currentTimeIndex(0)
{
    initializeFrequencyBands();
    initializeColorMap();
    loadFont();
    
    // Initialize spectrogram image
    spectrogramImage = sf::Image(sf::Vector2u(config.timeWindowSamples, 8), sf::Color::Black);
    if (!spectrogramTexture.loadFromImage(spectrogramImage)) {
        std::cerr << "Failed to load spectrogram texture from image" << std::endl;
    }
    spectrogramSprite = sf::Sprite(spectrogramTexture);
}

void SpectralRhythmogramDisplay::initializeFrequencyBands() {
    // Todd (1994) 8-band frequency structure
    frequencyBandNames = {
        "Phrase (0.125Hz)",   // Long-term musical structure
        "Whole (0.25Hz)",     // 4-beat units
        "Half (0.5Hz)",       // 2-beat units  
        "Quarter (1Hz)",      // Basic beat
        "Eighth (2Hz)",       // Sub-beat subdivisions
        "16th (4Hz)",         // Fast subdivisions
        "32nd (8Hz)",         // Very fast subdivisions
        "Onset (16Hz)"        // Attack transients
    };
    
    frequencyBandCenters = {
        0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f
    };
}

void SpectralRhythmogramDisplay::initializeColorMap() {
    // Create distinct colors for each frequency band (lower frequencies warmer colors)
    bandColors = {
        sf::Color(180, 50, 50),    // Deep red - Phrase
        sf::Color(220, 80, 40),    // Red-orange - Whole  
        sf::Color(240, 120, 30),   // Orange - Half
        sf::Color(255, 180, 20),   // Yellow-orange - Quarter
        sf::Color(200, 220, 40),   // Yellow-green - Eighth
        sf::Color(100, 200, 80),   // Green - 16th
        sf::Color(60, 150, 200),   // Blue - 32nd
        sf::Color(120, 80, 220)    // Purple - Onset
    };
}

void SpectralRhythmogramDisplay::loadFont() {
    // Try to load system font
    if (font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") ||
        font.openFromFile("/System/Library/Fonts/Arial.ttf") ||
        font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
        fontLoaded = true;
    } else {
        std::cerr << "Warning: Could not load font for SpectralRhythmogramDisplay" << std::endl;
        fontLoaded = false;
    }
}

void SpectralRhythmogramDisplay::setPosition(float x, float y) {
    position = sf::Vector2f(x, y);
    spectrogramSprite.setPosition(position);
}

void SpectralRhythmogramDisplay::setSize(float width, float height) {
    size = sf::Vector2f(width, height);
    
    // Scale sprite to fit the designated area
    float scaleX = (size.x - 80) / config.timeWindowSamples; // Leave space for labels
    float scaleY = (size.y - 40) / 8; // 8 frequency bands, leave space for time axis
    spectrogramSprite.setScale(sf::Vector2f(scaleX, scaleY));
    spectrogramSprite.setPosition(sf::Vector2f(position.x + 80, position.y + 20));
}

void SpectralRhythmogramDisplay::setDisplayConfig(const DisplayConfig& newConfig) {
    bool needsResize = (config.timeWindowSamples != newConfig.timeWindowSamples);
    config = newConfig;
    
    if (needsResize) {
        // Recreate image with new time window size
        spectrogramImage = sf::Image(sf::Vector2u(config.timeWindowSamples, 8), sf::Color::Black);
        if (!spectrogramTexture.loadFromImage(spectrogramImage)) {
            std::cerr << "Failed to reload spectrogram texture from image" << std::endl;
        }
        spectrogramSprite.setTexture(spectrogramTexture);
        
        // Clear history to match new size
        amplitudeHistory.clear();
        currentTimeIndex = 0;
    }
    
    needsRedraw = true;
}

void SpectralRhythmogramDisplay::update() {
    if (!rhythmInterpreter) return;
    
    // Check if it's time for an update
    float timeSinceLastUpdate = updateClock.getElapsedTime().asSeconds();
    float updateInterval = 1.0f / config.updateRate;
    
    if (timeSinceLastUpdate >= updateInterval) {
        // Get current filter outputs from rhythm interpreter
        auto filterOutputs = rhythmInterpreter->getFilterOutputs();
        
        if (!filterOutputs.empty()) {
            addDataPoint(filterOutputs);
            needsRedraw = true;
        }
        
        updateClock.restart();
    }
    
    if (needsRedraw) {
        updateSpectrogramTexture();
        needsRedraw = false;
    }
}

void SpectralRhythmogramDisplay::addDataPoint(const std::vector<float>& filterOutputs) {
    // Ensure we have data for all 8 frequency bands
    std::vector<float> amplitudes(8, 0.0f);
    for (size_t i = 0; i < std::min(filterOutputs.size(), amplitudes.size()); ++i) {
        amplitudes[i] = std::abs(filterOutputs[i]); // Ensure positive amplitude
    }
    
    // Add to history
    amplitudeHistory.push_back(amplitudes);
    
    // Maintain time window size
    if (amplitudeHistory.size() > config.timeWindowSamples) {
        amplitudeHistory.pop_front();
    }
    
    currentTimeIndex++;
}

void SpectralRhythmogramDisplay::updateSpectrogramTexture() {
    if (amplitudeHistory.empty()) return;
    
    // Update the image pixel by pixel
    for (size_t timeIdx = 0; timeIdx < amplitudeHistory.size(); ++timeIdx) {
        const auto& amplitudes = amplitudeHistory[timeIdx];
        
        for (size_t freqIdx = 0; freqIdx < 8; ++freqIdx) {
            // Note: frequency axis is inverted (lower frequencies at bottom)
            size_t yPixel = 7 - freqIdx; // Invert Y axis
            float amplitude = amplitudes[freqIdx];
            
            sf::Color pixelColor = amplitudeToColor(amplitude, freqIdx);
            spectrogramImage.setPixel(sf::Vector2u(timeIdx, yPixel), pixelColor);
        }
    }
    
    // Update texture
    spectrogramTexture.update(spectrogramImage);
}

sf::Color SpectralRhythmogramDisplay::amplitudeToColor(float amplitude, size_t bandIndex) const {
    // Clamp amplitude to configured range
    float normalizedAmplitude = std::clamp(
        (amplitude - config.minAmplitude) / (config.maxAmplitude - config.minAmplitude),
        0.0f, 1.0f
    );
    
    // Apply amplitude scaling
    normalizedAmplitude = std::pow(normalizedAmplitude, 1.0f / config.amplitudeScale);
    
    // Get base color for this frequency band
    sf::Color baseColor = bandColors[bandIndex % bandColors.size()];
    
    // Scale intensity based on amplitude
    std::uint8_t intensity = static_cast<std::uint8_t>(normalizedAmplitude * 255);
    
    return sf::Color(
        (baseColor.r * intensity) / 255,
        (baseColor.g * intensity) / 255,
        (baseColor.b * intensity) / 255,
        255
    );
}

std::uint8_t SpectralRhythmogramDisplay::amplitudeToIntensity(float amplitude) const {
    float normalizedAmplitude = std::clamp(
        (amplitude - config.minAmplitude) / (config.maxAmplitude - config.minAmplitude),
        0.0f, 1.0f
    );
    
    return static_cast<std::uint8_t>(normalizedAmplitude * 255);
}

void SpectralRhythmogramDisplay::render() {
    if (!window) return;
    
    // Draw background
    sf::RectangleShape background(size);
    background.setPosition(position);
    background.setFillColor(config.backgroundColor);
    window->draw(background);
    
    // Draw spectrogram
    window->draw(spectrogramSprite);
    
    // Draw overlays
    if (config.showGrid) {
        drawGrid(*window);
    }
    
    if (config.showLabels) {
        drawLabels(*window);
    }
    
    if (config.showTimeMarkers) {
        drawTimeMarkers(*window);
    }
}

void SpectralRhythmogramDisplay::drawGrid(sf::RenderTarget& target) const {
    sf::Vector2f spectrogramPos = spectrogramSprite.getPosition();
    sf::Vector2f spectrogramSize = sf::Vector2f(
        config.timeWindowSamples * spectrogramSprite.getScale().x,
        8 * spectrogramSprite.getScale().y
    );
    
    // Horizontal grid lines (frequency bands)
    for (int i = 0; i <= 8; ++i) {
        sf::Vertex line[] = {
            sf::Vertex({sf::Vector2f(spectrogramPos.x, spectrogramPos.y + i * spectrogramSize.y / 8), config.gridColor}),
            sf::Vertex({sf::Vector2f(spectrogramPos.x + spectrogramSize.x, spectrogramPos.y + i * spectrogramSize.y / 8), config.gridColor})
        };
        target.draw(line, 2, sf::PrimitiveType::Lines);
    }
    
    // Vertical grid lines (time markers every 50 pixels)
    for (float x = 0; x <= spectrogramSize.x; x += 50) {
        sf::Vertex line[] = {
            sf::Vertex({sf::Vector2f(spectrogramPos.x + x, spectrogramPos.y), config.gridColor}),
            sf::Vertex({sf::Vector2f(spectrogramPos.x + x, spectrogramPos.y + spectrogramSize.y), config.gridColor})
        };
        target.draw(line, 2, sf::PrimitiveType::Lines);
    }
}

void SpectralRhythmogramDisplay::drawLabels(sf::RenderTarget& target) const {
    if (!fontLoaded) return;
    
    sf::Vector2f spectrogramPos = spectrogramSprite.getPosition();
    sf::Vector2f spectrogramSize = sf::Vector2f(
        config.timeWindowSamples * spectrogramSprite.getScale().x,
        8 * spectrogramSprite.getScale().y
    );
    
    // Draw frequency band labels (on the left)
    for (size_t i = 0; i < frequencyBandNames.size(); ++i) {
        sf::Text label(font, frequencyBandNames[i], 12);
        label.setFillColor(config.textColor);
        
        // Position label at center of frequency band (remember Y is inverted)
        float yPos = spectrogramPos.y + (7 - i) * spectrogramSize.y / 8 + spectrogramSize.y / 16;
        label.setPosition(sf::Vector2f(position.x + 5, yPos - 6)); // Offset for text centering
        
        target.draw(label);
    }
    
    // Draw title
    sf::Text title(font, "Spectral Rhythmogram", 16);
    title.setFillColor(config.textColor);
    title.setStyle(sf::Text::Bold);
    title.setPosition(sf::Vector2f(position.x + size.x / 2 - 80, position.y));
    target.draw(title);
}

void SpectralRhythmogramDisplay::drawTimeMarkers(sf::RenderTarget& target) const {
    if (!fontLoaded || !rhythmInterpreter) return;
    
    sf::Vector2f spectrogramPos = spectrogramSprite.getPosition();
    sf::Vector2f spectrogramSize = sf::Vector2f(
        config.timeWindowSamples * spectrogramSprite.getScale().x,
        8 * spectrogramSprite.getScale().y
    );
    
    // Calculate time scale based on update rate
    float timePerPixel = 1.0f / (config.updateRate * spectrogramSprite.getScale().x);
    float totalTimeSpan = timePerPixel * spectrogramSize.x;
    
    // Draw time axis labels
    int numMarkers = 4;
    for (int i = 0; i <= numMarkers; ++i) {
        float timeValue = (totalTimeSpan * i) / numMarkers;
        float xPos = spectrogramPos.x + (spectrogramSize.x * i) / numMarkers;
        
        // Time marker line
        sf::Vertex line[] = {
            sf::Vertex({sf::Vector2f(xPos, spectrogramPos.y + spectrogramSize.y), config.gridColor}),
            sf::Vertex({sf::Vector2f(xPos, spectrogramPos.y + spectrogramSize.y + 5), config.gridColor})
        };
        target.draw(line, 2, sf::PrimitiveType::Lines);
        
        // Time label
        char timeStr[16];
        snprintf(timeStr, sizeof(timeStr), "%.1fs", totalTimeSpan - timeValue); // Show time from now
        sf::Text timeLabel(font, timeStr, 10);
        timeLabel.setFillColor(config.textColor);
        timeLabel.setPosition(sf::Vector2f(xPos - 15, spectrogramPos.y + spectrogramSize.y + 8));
        target.draw(timeLabel);
    }
}

void SpectralRhythmogramDisplay::clear() {
    amplitudeHistory.clear();
    currentTimeIndex = 0;
    spectrogramImage = sf::Image(sf::Vector2u(config.timeWindowSamples, 8), sf::Color::Black);
    spectrogramTexture.update(spectrogramImage);
    needsRedraw = true;
}

void SpectralRhythmogramDisplay::reset() {
    clear();
    updateClock.restart();
}

void SpectralRhythmogramDisplay::handleMouseClick(float x, float y) {
    // Convert click position to spectrogram coordinates
    sf::Vector2f spectrogramPos = spectrogramSprite.getPosition();
    sf::Vector2f spectrogramSize = sf::Vector2f(
        config.timeWindowSamples * spectrogramSprite.getScale().x,
        8 * spectrogramSprite.getScale().y
    );
    
    if (x >= spectrogramPos.x && x <= spectrogramPos.x + spectrogramSize.x &&
        y >= spectrogramPos.y && y <= spectrogramPos.y + spectrogramSize.y) {
        
        // Calculate which frequency band was clicked
        float relativeY = (y - spectrogramPos.y) / spectrogramSize.y;
        size_t bandIndex = static_cast<size_t>((1.0f - relativeY) * 8); // Invert Y
        bandIndex = std::clamp(bandIndex, size_t(0), size_t(7));
        
        // Calculate time index
        float relativeX = (x - spectrogramPos.x) / spectrogramSize.x;
        size_t timeIndex = static_cast<size_t>(relativeX * config.timeWindowSamples);
        
        std::cout << "Clicked band " << bandIndex << " (" << frequencyBandNames[bandIndex] 
                  << ") at time index " << timeIndex << std::endl;
    }
}

void SpectralRhythmogramDisplay::handleMouseScroll(float delta) {
    // Zoom amplitude scale with mouse scroll
    config.amplitudeScale *= (1.0f + delta * 0.1f);
    config.amplitudeScale = std::clamp(config.amplitudeScale, 0.1f, 5.0f);
    needsRedraw = true;
    
    std::cout << "Amplitude scale: " << config.amplitudeScale << std::endl;
}

void SpectralRhythmogramDisplay::handleKeyPress(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Key::R:
            reset();
            std::cout << "Spectral rhythmogram display reset" << std::endl;
            break;
        case sf::Keyboard::Key::G:
            config.showGrid = !config.showGrid;
            std::cout << "Grid " << (config.showGrid ? "enabled" : "disabled") << std::endl;
            break;
        case sf::Keyboard::Key::T:
            config.showTimeMarkers = !config.showTimeMarkers;
            std::cout << "Time markers " << (config.showTimeMarkers ? "enabled" : "disabled") << std::endl;
            break;
        case sf::Keyboard::Key::Equal: // Plus key
            config.updateRate = std::min(config.updateRate * 1.2f, 60.0f);
            std::cout << "Update rate: " << config.updateRate << " Hz" << std::endl;
            break;
        case sf::Keyboard::Key::Hyphen: // Minus key
            config.updateRate = std::max(config.updateRate / 1.2f, 1.0f);
            std::cout << "Update rate: " << config.updateRate << " Hz" << std::endl;
            break;
        default:
            break;
    }
}

void SpectralRhythmogramDisplay::exportToImage(const std::string& filename) const {
    if (!spectrogramImage.saveToFile(filename)) {
        std::cerr << "Failed to export spectral rhythmogram to: " << filename << std::endl;
    } else {
        std::cout << "Exported spectral rhythmogram to: " << filename << std::endl;
    }
}

float SpectralRhythmogramDisplay::getMaxAmplitudeInBand(size_t bandIndex, size_t timeRange) const {
    if (bandIndex >= 8 || amplitudeHistory.empty()) return 0.0f;
    
    size_t startIdx = timeRange == 0 ? 0 : 
                     (amplitudeHistory.size() > timeRange ? amplitudeHistory.size() - timeRange : 0);
    
    float maxAmp = 0.0f;
    for (size_t i = startIdx; i < amplitudeHistory.size(); ++i) {
        maxAmp = std::max(maxAmp, amplitudeHistory[i][bandIndex]);
    }
    
    return maxAmp;
}

std::vector<float> SpectralRhythmogramDisplay::getAverageSpectrum(size_t timeRange) const {
    std::vector<float> averages(8, 0.0f);
    if (amplitudeHistory.empty()) return averages;
    
    size_t startIdx = timeRange == 0 ? 0 : 
                     (amplitudeHistory.size() > timeRange ? amplitudeHistory.size() - timeRange : 0);
    size_t sampleCount = amplitudeHistory.size() - startIdx;
    
    if (sampleCount == 0) return averages;
    
    for (size_t i = startIdx; i < amplitudeHistory.size(); ++i) {
        for (size_t band = 0; band < 8; ++band) {
            averages[band] += amplitudeHistory[i][band];
        }
    }
    
    for (float& avg : averages) {
        avg /= sampleCount;
    }
    
    return averages;
}