#include "SimpleSpectralDisplay.h"
#include "RhythmInterpreter.h"
#include <algorithm>
#include <cmath>
#include <iostream>

SimpleSpectralDisplay::SimpleSpectralDisplay(RhythmInterpreter* rhythmInterp)
    : rhythmInterpreter(rhythmInterp)
    , position(0, 0)
    , size(400, 300)
    , fontLoaded(false)
    , needsTextureUpdate(false)
{
    initializeFrequencyBands();
    initializeColorMap();
    loadFont();
    initializeSpectrogram();
}

void SimpleSpectralDisplay::initializeFrequencyBands() {
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
}

void SimpleSpectralDisplay::initializeColorMap() {
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

void SimpleSpectralDisplay::loadFont() {
    // Try to load system font
    if (font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") ||
        font.openFromFile("/System/Library/Fonts/Arial.ttf") ||
        font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
        fontLoaded = true;
    } else {
        std::cerr << "Warning: Could not load font for SimpleSpectralDisplay" << std::endl;
        fontLoaded = false;
    }
}

void SimpleSpectralDisplay::setPosition(float x, float y) {
    position = sf::Vector2f(x, y);
}

void SimpleSpectralDisplay::setSize(float width, float height) {
    size = sf::Vector2f(width, height);
    // Reinitialize spectrogram with new size
    initializeSpectrogram();
}

void SimpleSpectralDisplay::setRhythmInterpreter(RhythmInterpreter* rhythmInterp) {
    rhythmInterpreter = rhythmInterp;
}

void SimpleSpectralDisplay::update() {
    if (!rhythmInterpreter) return;
    
    // Check if it's time for an update
    float timeSinceLastUpdate = updateClock.getElapsedTime().asSeconds();
    float updateInterval = 1.0f / config.updateRate;
    
    if (timeSinceLastUpdate >= updateInterval) {
        // Get current filter outputs and add to history
        auto filterOutputs = rhythmInterpreter->getFilterOutputs();
        if (!filterOutputs.empty()) {
            addDataPoint(filterOutputs);
        }
        updateClock.restart();
    }
    
    // Update spectrogram image if needed
    if (needsTextureUpdate) {
        updateSpectrogramImage();
    }
}

void SimpleSpectralDisplay::render(sf::RenderWindow& window) {
    drawBackground(window);
    drawSpectrogram(window);
    
    if (config.showGrid) {
        drawGrid(window);
    }
    
    if (config.showLabels) {
        drawLabels(window);
    }
}

void SimpleSpectralDisplay::drawBackground(sf::RenderWindow& window) {
    sf::RectangleShape background(size);
    background.setPosition(position);
    background.setFillColor(config.backgroundColor);
    window.draw(background);
}

void SimpleSpectralDisplay::drawSpectrogram(sf::RenderWindow& window) {
    if (amplitudeHistory.empty() || !spectrogramSprite) return;
    
    // Calculate display area (leave space for labels)
    float displayWidth = size.x - 80;  // Leave 80 pixels for labels
    float displayHeight = size.y - 40; // Leave 40 pixels for time axis
    float displayX = position.x + 80;
    float displayY = position.y + 20;
    
    // Position and scale the spectrogram sprite
    spectrogramSprite->setPosition(sf::Vector2f(displayX, displayY));
    
    // Scale to fit display area
    float scaleX = displayWidth / config.timeWindowSamples;
    float scaleY = displayHeight / 8; // 8 frequency bands
    spectrogramSprite->setScale(sf::Vector2f(scaleX, scaleY));
    
    // Draw the scrolling spectrogram
    window.draw(*spectrogramSprite);
}

void SimpleSpectralDisplay::drawGrid(sf::RenderWindow& window) {
    float displayWidth = size.x - 80;
    float displayHeight = size.y - 40;
    float displayX = position.x + 80;
    float displayY = position.y + 20;
    
    // Horizontal grid lines (frequency bands)
    for (int i = 0; i <= 8; ++i) {
        float yPos = displayY + i * displayHeight / 8;
        
        sf::RectangleShape line(sf::Vector2f(displayWidth, 1));
        line.setPosition(sf::Vector2f(displayX, yPos));
        line.setFillColor(sf::Color(100, 100, 100, 128));
        window.draw(line);
    }
    
    // Vertical grid lines
    for (int i = 0; i <= 4; ++i) {
        float xPos = displayX + i * displayWidth / 4;
        
        sf::RectangleShape line(sf::Vector2f(1, displayHeight));
        line.setPosition(sf::Vector2f(xPos, displayY));
        line.setFillColor(sf::Color(100, 100, 100, 128));
        window.draw(line);
    }
}

void SimpleSpectralDisplay::drawLabels(sf::RenderWindow& window) {
    if (!fontLoaded) return;
    
    float displayHeight = size.y - 40;
    float displayY = position.y + 20;
    float barHeight = displayHeight / 8;
    
    // Draw frequency band labels
    for (size_t i = 0; i < frequencyBandNames.size(); ++i) {
        sf::Text label(font, frequencyBandNames[i], 12);
        label.setFillColor(config.textColor);
        
        float yPos = displayY + (7 - i) * barHeight + barHeight / 2 - 6; // Center vertically
        label.setPosition(sf::Vector2f(position.x + 5, yPos));
        
        window.draw(label);
    }
    
    // Draw title
    sf::Text title(font, "Rhythm Spectrum", 16);
    title.setFillColor(config.textColor);
    title.setPosition(sf::Vector2f(position.x + size.x / 2 - 60, position.y));
    window.draw(title);
}

void SimpleSpectralDisplay::clear() {
    amplitudeHistory.clear();
    initializeSpectrogram();
}

void SimpleSpectralDisplay::handleKeyPress(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Key::G:
            config.showGrid = !config.showGrid;
            std::cout << "Grid " << (config.showGrid ? "enabled" : "disabled") << std::endl;
            break;
        case sf::Keyboard::Key::L:
            config.showLabels = !config.showLabels;
            std::cout << "Labels " << (config.showLabels ? "enabled" : "disabled") << std::endl;
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

void SimpleSpectralDisplay::initializeSpectrogram() {
    // Create image for spectrogram (width = time samples, height = 8 frequency bands)
    spectrogramImage = sf::Image(sf::Vector2u(config.timeWindowSamples, 8), sf::Color::Black);
    
    if (!spectrogramTexture.loadFromImage(spectrogramImage)) {
        std::cerr << "Failed to load spectrogram texture" << std::endl;
    }
    
    // Initialize sprite after texture is loaded
    spectrogramSprite = sf::Sprite(spectrogramTexture);
    needsTextureUpdate = true;
}

void SimpleSpectralDisplay::addDataPoint(const std::vector<float>& filterOutputs) {
    // Store the new data point
    std::vector<float> amplitudes(8, 0.0f);
    for (size_t i = 0; i < std::min(filterOutputs.size(), amplitudes.size()); ++i) {
        amplitudes[i] = std::abs(filterOutputs[i]);
    }
    
    amplitudeHistory.push_back(amplitudes);
    
    // Maintain window size
    if (amplitudeHistory.size() > config.timeWindowSamples) {
        amplitudeHistory.pop_front();
    }
    
    needsTextureUpdate = true;
}

void SimpleSpectralDisplay::updateSpectrogramImage() {
    if (amplitudeHistory.empty()) return;
    
    // Clear the image
    spectrogramImage = sf::Image(sf::Vector2u(config.timeWindowSamples, 8), sf::Color::Black);
    
    // Fill the image with amplitude history - new data on left, travels right
    for (size_t timeIdx = 0; timeIdx < amplitudeHistory.size(); ++timeIdx) {
        const auto& amplitudes = amplitudeHistory[timeIdx];
        
        // Reverse X mapping: newest data (highest timeIdx) appears at X=0 (left)
        // Older data appears further right
        size_t xPixel = amplitudeHistory.size() - 1 - timeIdx;
        
        for (size_t freqIdx = 0; freqIdx < 8; ++freqIdx) {
            // Invert Y axis (lower frequencies at bottom)
            size_t yPixel = 7 - freqIdx;
            float amplitude = amplitudes[freqIdx];
            
            sf::Color pixelColor = amplitudeToColor(amplitude, freqIdx);
            spectrogramImage.setPixel(sf::Vector2u(xPixel, yPixel), pixelColor);
        }
    }
    
    // Update texture
    spectrogramTexture.update(spectrogramImage);
    if (spectrogramSprite) {
        spectrogramSprite->setTexture(spectrogramTexture);
    }
    needsTextureUpdate = false;
}

sf::Color SimpleSpectralDisplay::amplitudeToColor(float amplitude, size_t bandIndex) {
    // Use different scaling for different filter types
    float normalizedAmplitude;
    
    if (bandIndex < 5) {
        // Lower frequency filters (0-4): Phrase, Whole, Half, Quarter, Eighth
        // These have larger amplitude ranges (typically 0.001 to 0.01)
        normalizedAmplitude = std::clamp(amplitude / 0.01f, 0.0f, 1.0f);
    } else {
        // Higher frequency filters (5-7): 16th, 32nd, Onset
        // These have smaller amplitude ranges (typically 1e-7 to 1e-6)
        normalizedAmplitude = std::clamp(amplitude / 1e-6f, 0.0f, 1.0f);
    }
    
    // Get base color for this frequency band
    sf::Color baseColor = bandColors[bandIndex % bandColors.size()];
    
    // Use a brighter color mapping with minimum visibility
    float brightness = 0.2f + (normalizedAmplitude * 0.8f); // Range from 20% to 100% brightness
    
    return sf::Color(
        static_cast<std::uint8_t>(baseColor.r * brightness),
        static_cast<std::uint8_t>(baseColor.g * brightness),
        static_cast<std::uint8_t>(baseColor.b * brightness),
        255  // Full opacity
    );
}