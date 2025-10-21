#include "SimpleSpectralDisplay.h"
#include "RhythmInterpreter.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <cstdio>

SimpleSpectralDisplay::SimpleSpectralDisplay(RhythmInterpreter* rhythmInterp)
    : rhythmInterpreter(rhythmInterp)
    , position(0, 0)
    , size(400, 300)
    , fontLoaded(false)
    , needsTextureUpdate(false)
    , lastKnownBPM(120.0f) // Default BPM
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

void SimpleSpectralDisplay::updateFrequencyBands() {
    if (!rhythmInterpreter) return;
    
    // Use the stored BPM (set by setManualBPM or defaults to 120.0f)
    float currentBPM = lastKnownBPM;
    
    // Calculate tempo scaling factor (same as RhythmInterpreter)
    float tempoScale = currentBPM / 120.0f;
    
    // Base frequencies at 120 BPM (Todd 1994)
    const std::vector<float> baseFrequencies = {
        0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f
    };
    
    const std::vector<std::string> baseNames = {
        "Phrase", "Whole", "Half", "Quarter", "Eighth", "16th", "32nd", "Onset"
    };
    
    // Update frequency band names with current tempo-scaled frequencies
    frequencyBandNames.clear();
    for (size_t i = 0; i < baseFrequencies.size(); ++i) {
        float scaledFreq = baseFrequencies[i] * tempoScale;
        
        // Format frequency with appropriate precision
        std::string freqStr;
        if (scaledFreq < 1.0f) {
            // For frequencies < 1Hz, show 3 decimal places
            char buffer[16];
            snprintf(buffer, sizeof(buffer), "%.3fHz", scaledFreq);
            freqStr = buffer;
        } else if (scaledFreq < 10.0f) {
            // For 1-10Hz, show 1 decimal place
            char buffer[16]; 
            snprintf(buffer, sizeof(buffer), "%.1fHz", scaledFreq);
            freqStr = buffer;
        } else {
            // For >= 10Hz, show as integer
            char buffer[16];
            snprintf(buffer, sizeof(buffer), "%.0fHz", scaledFreq);
            freqStr = buffer;
        }
        
        frequencyBandNames.push_back(baseNames[i] + " (" + freqStr + ")");
    }
}

void SimpleSpectralDisplay::initializeColorMap() {
    // Amplitude-based color map: smooth spectrum from black -> blue -> cyan -> green -> yellow -> orange -> red -> white
    // This will be used as a lookup table for amplitude-to-color conversion
    bandColors = {
        sf::Color(255, 255, 255),  // White - maximum amplitude
        sf::Color(255, 100, 100),  // Bright red - very high amplitude  
        sf::Color(255, 200, 0),    // Orange-yellow - high amplitude
        sf::Color(200, 255, 0),    // Yellow-green - medium-high amplitude
        sf::Color(0, 255, 100),    // Cyan-green - medium amplitude
        sf::Color(0, 200, 255),    // Light blue - medium-low amplitude
        sf::Color(0, 100, 255),    // Blue - low amplitude
        sf::Color(50, 50, 100)     // Dark blue - very low amplitude
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
    
    // If setting to nullptr (network reset), clear any pending data
    if (!rhythmInterpreter) {
        std::cout << "🔄 SimpleSpectralDisplay: Rhythm interpreter cleared, resetting display" << std::endl;
        clear(); // Clear the display data
    }
}

void SimpleSpectralDisplay::setOpacity(float opacity) {
    config.opacity = std::clamp(opacity, 0.0f, 100.0f);
}

void SimpleSpectralDisplay::setContrast(float contrast) {
    config.contrast = std::clamp(contrast, 0.1f, 10.0f);
}

void SimpleSpectralDisplay::setManualBPM(float bpm) {
    // Update BPM and force frequency band update
    if (std::abs(bpm - lastKnownBPM) >= 0.01f) {
        lastKnownBPM = bpm;
        updateFrequencyBands();
    }
}

float SimpleSpectralDisplay::getOpacity() const {
    return config.opacity;
}

void SimpleSpectralDisplay::update() {
    if (!rhythmInterpreter) return;
    
    // Update frequency band labels if BPM changed
    updateFrequencyBands();
    
    // Check if it's time for an update
    float timeSinceLastUpdate = updateClock.getElapsedTime().asSeconds();
    float updateInterval = 1.0f / config.updateRate;
    
    if (timeSinceLastUpdate >= updateInterval) {
        // Safely get current filter outputs and add to history
        try {
            auto filterOutputs = rhythmInterpreter->getFilterOutputs();
            if (!filterOutputs.empty()) {
                addDataPoint(filterOutputs);
            }
        } catch (const std::exception& e) {
            std::cerr << "⚠️  SimpleSpectralDisplay: Error getting filter outputs: " << e.what() << std::endl;
            // Reset rhythm interpreter reference to prevent further crashes
            rhythmInterpreter = nullptr;
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
    // Apply contrast adjustment first to preserve dynamic range
    float contrastedAmplitude = amplitude * config.contrast;
    
    // Adaptive scaling: stronger boost for very small values, gentler for larger ones
    float scaledAmplitude;
    if (contrastedAmplitude < 0.01f) {
        scaledAmplitude = contrastedAmplitude * 15.0f; // Strong boost for quiet signals
    } else if (contrastedAmplitude < 0.1f) {
        scaledAmplitude = 0.15f + (contrastedAmplitude - 0.01f) * 5.0f; // Medium boost 
    } else {
        scaledAmplitude = 0.6f + (contrastedAmplitude - 0.1f) * 2.0f; // Gentle boost for loud signals
    }
    float normalizedAmplitude = std::clamp(scaledAmplitude, 0.0f, 1.0f);
    
    // Apply gamma correction for better perceptual response with normalized levels
    float gamma = 0.5f; // Moderate gamma for balanced visibility across the range
    float correctedAmplitude = std::pow(normalizedAmplitude, gamma);
    
    // Create smooth amplitude-based color interpolation through spectrum
    sf::Color resultColor;
    
    if (correctedAmplitude < 0.001f) {
        // Near-silent: black
        resultColor = sf::Color(0, 0, 0);
    } else if (correctedAmplitude < 0.125f) {
        // Very low: dark blue to blue
        float t = correctedAmplitude / 0.125f;
        resultColor = sf::Color(
            static_cast<std::uint8_t>(0 + t * 50),     // R: 0 -> 50
            static_cast<std::uint8_t>(0 + t * 50),     // G: 0 -> 50  
            static_cast<std::uint8_t>(50 + t * 150)    // B: 50 -> 200
        );
    } else if (correctedAmplitude < 0.25f) {
        // Low: blue to cyan
        float t = (correctedAmplitude - 0.125f) / 0.125f;
        resultColor = sf::Color(
            static_cast<std::uint8_t>(50),                      // R: constant
            static_cast<std::uint8_t>(50 + t * 150),           // G: 50 -> 200
            static_cast<std::uint8_t>(200)                     // B: constant
        );
    } else if (correctedAmplitude < 0.5f) {
        // Medium: cyan to green
        float t = (correctedAmplitude - 0.25f) / 0.25f;
        resultColor = sf::Color(
            static_cast<std::uint8_t>(50),                     // R: constant
            static_cast<std::uint8_t>(200 + t * 55),          // G: 200 -> 255
            static_cast<std::uint8_t>(200 - t * 100)          // B: 200 -> 100
        );
    } else if (correctedAmplitude < 0.75f) {
        // Medium-high: green to yellow
        float t = (correctedAmplitude - 0.5f) / 0.25f;
        resultColor = sf::Color(
            static_cast<std::uint8_t>(50 + t * 205),          // R: 50 -> 255
            static_cast<std::uint8_t>(255),                   // G: constant
            static_cast<std::uint8_t>(100 - t * 100)         // B: 100 -> 0
        );
    } else if (correctedAmplitude < 0.9f) {
        // High: yellow to red
        float t = (correctedAmplitude - 0.75f) / 0.15f;
        resultColor = sf::Color(
            static_cast<std::uint8_t>(255),                   // R: constant
            static_cast<std::uint8_t>(255 - t * 155),        // G: 255 -> 100
            static_cast<std::uint8_t>(0)                     // B: constant
        );
    } else {
        // Very high: red to white (maximum range)
        float t = (correctedAmplitude - 0.9f) / 0.1f;
        resultColor = sf::Color(
            static_cast<std::uint8_t>(255),                   // R: constant
            static_cast<std::uint8_t>(100 + t * 155),        // G: 100 -> 255
            static_cast<std::uint8_t>(0 + t * 255)           // B: 0 -> 255
        );
    }
    
    // Apply configurable opacity
    std::uint8_t alpha = static_cast<std::uint8_t>(config.opacity * 2.55f);
    resultColor.a = alpha;
    
    return resultColor;
}