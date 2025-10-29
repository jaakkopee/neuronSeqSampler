#include "Visualizer.h"
#include "NeuronNetwork.h"
#include "Neuron.h"
#include "Connection.h"
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>

Visualizer::Visualizer(sf::RenderWindow* renderWindow, NeuronNetwork* neuronNetwork)
    : window(renderWindow)
    , network(neuronNetwork)
    , neuronRadius(15.0f)
    , normalNeuronColor(sf::Color::Blue)
    , firedNeuronColor(sf::Color::Red)
    , connectionColor(sf::Color::White)
    , highlightConnectionColor(sf::Color::Yellow)
    , fontLoaded(false)
    , canvasOffset(50.0f, 50.0f)
    , canvasSize(500.0f, 400.0f)
    , currentViewMode(ViewMode::Grid)
    , hoveredNeuronIndex(-1)
    , mousePosition(0.0f, 0.0f)
    , showTooltip(false)
{
    // Try to load a default system font
    loadFont("");
    calculateNeuronPositions();
}

void Visualizer::setCanvasArea(float x, float y, float width, float height) {
    canvasOffset = sf::Vector2f(x, y);
    canvasSize = sf::Vector2f(width, height);
    calculateNeuronPositions();
}

void Visualizer::refreshLayout() {
    calculateNeuronPositions();
}

void Visualizer::calculateNeuronPositions() {
    if (!network) return;
    
    neuronPositions.clear();
    size_t neuronCount = network->getNeuronCount();
    
    if (neuronCount == 0) return;
    
    switch (currentViewMode) {
        case ViewMode::Grid:
            calculateGridPositions();
            break;
        case ViewMode::Circular:
            calculateCircularPositions();
            break;
    }
}

void Visualizer::calculateGridPositions() {
    if (!network) return;
    
    size_t neuronCount = network->getNeuronCount();
    if (neuronCount == 0) return;
    
    // Arrange neurons in a grid pattern
    int cols = static_cast<int>(std::ceil(std::sqrt(neuronCount)));
    int rows = static_cast<int>(std::ceil(static_cast<float>(neuronCount) / cols));
    
    float spacingX = canvasSize.x / (cols + 1);
    float spacingY = canvasSize.y / (rows + 1);
    
    for (size_t i = 0; i < neuronCount; ++i) {
        int row = i / cols;
        int col = i % cols;
        
        float x = canvasOffset.x + spacingX * (col + 1);
        float y = canvasOffset.y + spacingY * (row + 1);
        
        neuronPositions.push_back(sf::Vector2f(x, y));
    }
}

void Visualizer::calculateCircularPositions() {
    if (!network) return;
    
    size_t neuronCount = network->getNeuronCount();
    if (neuronCount == 0) return;
    
    // Calculate center and radius for circular layout
    float centerX = canvasOffset.x + canvasSize.x / 2.0f;
    float centerY = canvasOffset.y + canvasSize.y / 2.0f;
    float radius = std::min(canvasSize.x, canvasSize.y) * 0.35f; // Leave some margin
    
    for (size_t i = 0; i < neuronCount; ++i) {
        // Distribute neurons evenly around the circle
        float angle = (2.0f * static_cast<float>(M_PI) * static_cast<float>(i)) / static_cast<float>(neuronCount);
        
        // Convert to Cartesian coordinates
        float x = centerX + radius * std::cos(angle);
        float y = centerY + radius * std::sin(angle);
        
        neuronPositions.push_back(sf::Vector2f(x, y));
    }
}

void Visualizer::render() {
    if (!network) return;
    
    // Draw connections first (so they appear behind neurons)
    const auto& connections = network->getConnections();
    const auto& neurons = network->getNeurons();
    std::vector<bool> connectionDrawn(connections.size(), false);
    
    for (size_t i = 0; i < connections.size(); ++i) {
        if (connectionDrawn[i]) continue;
        
        const Connection* conn = connections[i].get();
        
        // Find source and target neuron positions
        int sourceIndex = -1, targetIndex = -1;
        
        for (size_t j = 0; j < neurons.size(); ++j) {
            if (neurons[j].get() == conn->getSource()) {
                sourceIndex = j;
            }
            if (neurons[j].get() == conn->getTarget()) {
                targetIndex = j;
            }
        }
        
        if (sourceIndex >= 0 && targetIndex >= 0 && 
            sourceIndex < neuronPositions.size() && 
            targetIndex < neuronPositions.size()) {
            
            // Check for bi-directional connection
            bool hasBidirectional = false;
            size_t reverseConnIndex = 0;
            
            for (size_t k = i + 1; k < connections.size(); ++k) {
                const Connection* otherConn = connections[k].get();
                if (otherConn->getSource() == conn->getTarget() && 
                    otherConn->getTarget() == conn->getSource()) {
                    hasBidirectional = true;
                    reverseConnIndex = k;
                    connectionDrawn[k] = true; // Mark reverse connection as drawn
                    break;
                }
            }
            
            if (hasBidirectional) {
                // Draw curved connections for bi-directional links
                drawCurvedConnection(conn, neuronPositions[sourceIndex], neuronPositions[targetIndex], false);
                const Connection* reverseConn = connections[reverseConnIndex].get();
                drawCurvedConnection(reverseConn, neuronPositions[targetIndex], neuronPositions[sourceIndex], true);
            } else {
                // Draw straight connection
                drawConnection(conn, neuronPositions[sourceIndex], neuronPositions[targetIndex]);
            }
            
            connectionDrawn[i] = true;
        }
    }
    
    // Ensure firing animations vector matches neuron count
    if (firingAnimations.size() != neurons.size()) {
        firingAnimations.resize(neurons.size());
    }
    
    // Check for newly fired neurons and update animations
    for (size_t i = 0; i < neurons.size(); ++i) {
        bool currentlyFired = neurons[i]->getHasFired();
        float activation = neurons[i]->getActivation(); // Use processed activation for visualization
        float threshold = neurons[i]->getThreshold();
        
        // Detect firing: either hasFired flag is set, or activation exceeds threshold
        bool isCurrentlyFiring = currentlyFired || (activation > threshold);
        bool justFired = isCurrentlyFiring && !firingAnimations[i].previouslyFired;
        
        // Update previous state
        firingAnimations[i].previouslyFired = isCurrentlyFiring;
        
        // Trigger animation if neuron just fired
        if (justFired) {
            triggerFiringAnimation(i);
        }
    }
    
    // Update firing animations
    updateFiringAnimations();
    
    // Draw neurons (reuse neurons variable from above)
    for (size_t i = 0; i < neurons.size() && i < neuronPositions.size(); ++i) {
        drawNeuron(neurons[i].get(), neuronPositions[i], i);
    }
    
    // Draw tooltip if showing
    if (showTooltip && hoveredNeuronIndex >= 0 && 
        hoveredNeuronIndex < static_cast<int>(neurons.size()) && fontLoaded) {
        const Neuron* hoveredNeuron = neurons[hoveredNeuronIndex].get();
        std::string samplePath = hoveredNeuron->getSampleFilePath();
        
        // Debug: Print what we got from getSampleFilePath()
        std::cout << "Tooltip Debug - Neuron " << hoveredNeuronIndex << " sample path: '" << samplePath << "'" << std::endl;
        
        // Extract just the filename from the path for cleaner display
        std::string filename = samplePath;
        size_t lastSlash = samplePath.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            filename = samplePath.substr(lastSlash + 1);
        }
        
        if (!filename.empty()) {
            drawTooltip(filename, mousePosition);
        } else {
            // Debug: Show what we're working with
            drawTooltip("No sample file (empty path)", mousePosition);
        }
    }
}

void Visualizer::drawNeuron(const Neuron* neuron, const sf::Vector2f& position, size_t neuronIndex) {
    // Apply firing animation scaling
    float scaleFactor = (neuronIndex < firingAnimations.size()) ? firingAnimations[neuronIndex].scaleFactor : 1.0f;
    float scaledRadius = neuronRadius * scaleFactor;
    
    sf::CircleShape circle(scaledRadius);
        circle.setOrigin(sf::Vector2f(scaledRadius, scaledRadius));
    circle.setPosition(position);
    
    // Use rainbow color based on activation level and firing state
    sf::Color color = getRainbowColor(neuron->getActivation(), neuron->getHasFired()); // Use processed activation for color
    
    circle.setFillColor(color);
    circle.setOutlineThickness(2.0f);
    circle.setOutlineColor(sf::Color::White);
    
    window->draw(circle);
    
    // Draw text if font is loaded
    if (fontLoaded) {
        // Create neuron index text (N1, N2, etc.)
        sf::Text neuronText(font, "N" + std::to_string(neuronIndex + 1), 12);
        neuronText.setFillColor(sf::Color::White);
        // Center the text on the neuron
        sf::FloatRect textBounds = neuronText.getLocalBounds();
        neuronText.setOrigin(sf::Vector2f(textBounds.position.x + textBounds.size.x / 2.0f, 
                                          textBounds.position.y + textBounds.size.y / 2.0f));
        neuronText.setPosition(position); // Centered on the neuron
        window->draw(neuronText);
    }
}

void Visualizer::drawConnection(const Connection* connection, 
                               const sf::Vector2f& sourcePos, 
                               const sf::Vector2f& targetPos) {
    // Calculate vibration based on source and target neuron activations
    const Neuron* sourceNeuron = connection->getSource();
    const Neuron* targetNeuron = connection->getTarget();
    float sourceActivation = sourceNeuron ? std::abs(sourceNeuron->getActivation()) : 0.0f;
    float targetActivation = targetNeuron ? std::abs(targetNeuron->getActivation()) : 0.0f;
    float totalActivation = sourceActivation + targetActivation;
    
    // Color based on connection weight
    float weight = connection->getWeight();
    sf::Color baseColor = connectionColor;
    
    if (std::abs(weight) > 0.1f) {
        float intensity = std::min(1.0f, std::abs(weight));
        baseColor.a = static_cast<std::uint8_t>(100 + 155 * intensity);
        if (weight > 0) {
            baseColor = sf::Color(255, 255, 255, baseColor.a); // White for positive
        } else {
            baseColor = sf::Color(255, 100, 100, baseColor.a); // Red for negative
        }
    } else {
        baseColor.a = 50; // Very faint for near-zero weights
    }
    // Enhance color based on neuronal activity
    float activityBoost = std::min(1.0f, totalActivation);
    sf::Color activityColor = baseColor;
    if (activityBoost > 0.1f) {
        activityColor.r = static_cast<std::uint8_t>(std::min(255, static_cast<int>(baseColor.r + activityBoost * 100)));
        activityColor.g = static_cast<std::uint8_t>(std::min(255, static_cast<int>(baseColor.g + activityBoost * 50)));
        activityColor.b = static_cast<std::uint8_t>(std::min(255, static_cast<int>(baseColor.b + activityBoost * 150)));
        activityColor.a = static_cast<std::uint8_t>(std::min(255, static_cast<int>(baseColor.a + activityBoost * 100)));
    }
    
    // Draw single thick line with vibration effect
    sf::Vector2f direction = targetPos - sourcePos;
    sf::Vector2f perpendicular(-direction.y, direction.x);
    float lineLength = std::sqrt(perpendicular.x * perpendicular.x + perpendicular.y * perpendicular.y);
    if (lineLength > 0) {
        perpendicular.x /= lineLength;
        perpendicular.y /= lineLength;
    }
    
    // Only apply vibration if connection has non-zero weight
    sf::Vector2f vibratingSourcePos = sourcePos;
    sf::Vector2f vibratingTargetPos = targetPos;
    
    if (std::abs(weight) > 0.001f) { // Small threshold to avoid floating point precision issues
        // Get current time for sine wave calculations
        float currentTime = animationClock.getElapsedTime().asSeconds();
        
        // Dance modulation frequencies: 4, 8, 16, 32, 64 Hz (higher frequencies for more dynamic movement)
        std::vector<float> frequencies = {4.0f, 8.0f, 16.0f, 32.0f, 64.0f};
        
        // Calculate composite sine wave modulation
        float modulation = 0.0f;
        for (float freq : frequencies) {
            float amplitude = totalActivation * (1.0f / freq) * 8.0f; // Dramatic amplitude for striking visual effect
            modulation += amplitude * std::sin(2.0f * M_PI * freq * currentTime);
        }
        
        // Apply modulation in perpendicular direction to connection line
        sf::Vector2f direction = targetPos - sourcePos;
        sf::Vector2f perpDir(-direction.y, direction.x);
        float dirLength = std::sqrt(perpDir.x * perpDir.x + perpDir.y * perpDir.y);
        if (dirLength > 0) {
            perpDir.x /= dirLength;
            perpDir.y /= dirLength;
        }
        
        sf::Vector2f vibration = perpDir * modulation;
        
        vibratingSourcePos = sourcePos + vibration;
        vibratingTargetPos = targetPos + vibration;
    }
    
    // Create thick line using rectangle shape
    sf::RectangleShape line;
    sf::Vector2f lineVector = vibratingTargetPos - vibratingSourcePos;
    float length = std::sqrt(lineVector.x * lineVector.x + lineVector.y * lineVector.y);
    float angle = std::atan2(lineVector.y, lineVector.x) * 180.0f / M_PI;
    
    line.setSize(sf::Vector2f(length, 3.0f));
    line.setPosition(vibratingSourcePos);
    line.setOrigin(sf::Vector2f(0, 1.5f));
    line.setRotation(sf::degrees(angle));
    line.setFillColor(activityColor);
    
    window->draw(line);
}
void Visualizer::handleMouseDrag(int mouseX, int mouseY) {
    // Pan the canvas based on mouse drag
    // For simplicity, let's just move the canvas offset by a fixed amount
    // In a real application, you'd track mouse movement delta
    int deltaX = mouseX - (canvasOffset.x + canvasSize.x / 2);
    int deltaY = mouseY - (canvasOffset.y + canvasSize.y / 2);
    canvasOffset.x += deltaX * 0.1f; // Adjust the factor for sensitivity
    canvasOffset.y += deltaY * 0.1f;
    calculateNeuronPositions();
}

void Visualizer::handleMouseScroll(int delta) {
    // Zoom in/out with mouse scroll
    if (delta > 0) {
        // Zoom in
        canvasSize.x *= 0.9f;
        canvasSize.y *= 0.9f;
    } else if (delta < 0) {
        // Zoom out
        canvasSize.x *= 1.1f;
        canvasSize.y *= 1.1f;
    }
    calculateNeuronPositions();
}

void Visualizer::setViewMode(ViewMode mode) {
    if (currentViewMode != mode) {
        currentViewMode = mode;
        calculateNeuronPositions();
    }
}

void Visualizer::drawCurvedConnection(const Connection* connection,
                                     const sf::Vector2f& sourcePos,
                                     const sf::Vector2f& targetPos,
                                     bool isReverse) {
    // Calculate control point for Bezier curve
    sf::Vector2f midPoint = (sourcePos + targetPos) / 2.0f;
    
    // Always use the same reference direction to ensure curves are on opposite sides
    // Use the direction from the lower-indexed position to higher-indexed position
    sf::Vector2f referenceDirection;
    if (sourcePos.x < targetPos.x || (sourcePos.x == targetPos.x && sourcePos.y < targetPos.y)) {
        referenceDirection = targetPos - sourcePos;
    } else {
        referenceDirection = sourcePos - targetPos;
    }
    
    sf::Vector2f perpendicular(-referenceDirection.y, referenceDirection.x); // Perpendicular vector
    
    // Normalize perpendicular vector
    float length = std::sqrt(perpendicular.x * perpendicular.x + perpendicular.y * perpendicular.y);
    if (length > 0) {
        perpendicular.x /= length;
        perpendicular.y /= length;
    }
    
    // Offset control point perpendicular to the connection line
    float curveOffset = 30.0f; // Adjust for curve intensity
    
    // Use opposite sides for forward and reverse connections
    if (isReverse) {
        perpendicular = -perpendicular; // Flip to opposite side
    }
    
    // Calculate vibration based on source and target neuron activations
    const Neuron* sourceNeuron = connection->getSource();
    const Neuron* targetNeuron = connection->getTarget();
    float sourceActivation = sourceNeuron ? std::abs(sourceNeuron->getActivation()) : 0.0f;
    float targetActivation = targetNeuron ? std::abs(targetNeuron->getActivation()) : 0.0f;
    float totalActivation = sourceActivation + targetActivation;
    
    // Get connection weight to determine if vibration should be applied
    float weight = connection->getWeight();
    
    // Base curve offset with optional vibration (only if weight is non-zero)
    float vibrationOffset = 0.0f;
    if (std::abs(weight) > 0.001f) { // Small threshold to avoid floating point precision issues
        // Get current time for sine wave calculations
        float currentTime = animationClock.getElapsedTime().asSeconds();
        
        // Dance modulation frequencies: 4, 8, 16, 32, 64 Hz (higher frequencies for more dynamic movement)
        std::vector<float> frequencies = {4.0f, 8.0f, 16.0f, 32.0f, 64.0f};
        
        // Calculate composite sine wave modulation for curve offset
        for (float freq : frequencies) {
            float amplitude = totalActivation * (1.0f / freq) * 6.0f; // Dramatic amplitude for striking curve movement
            vibrationOffset += amplitude * std::sin(2.0f * M_PI * freq * currentTime);
        }
    }
    sf::Vector2f controlPoint = midPoint + perpendicular * (curveOffset + vibrationOffset);
    
    // Generate thicker curved line using multiple parallel curves
    const int segments = 30; // More segments for smoother curves
    const int thickness = 3; // Number of parallel lines for thickness
    std::vector<std::vector<sf::Vertex>> thickCurveLines;
    
    for (int lineOffset = -thickness/2; lineOffset <= thickness/2; ++lineOffset) {
        std::vector<sf::Vertex> curveVertices;
        sf::Vector2f offsetVector = perpendicular * static_cast<float>(lineOffset) * 0.5f;
        
        for (int i = 0; i <= segments; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(segments);
            sf::Vector2f basePoint = calculateBezierPoint(sourcePos, controlPoint, targetPos, t);
            
            // Add subtle vibration to each point only if weight is non-zero
            sf::Vector2f vibrationVector(0.0f, 0.0f);
            if (std::abs(weight) > 0.001f) {
                // Get current time for sine wave calculations
                float currentTime = animationClock.getElapsedTime().asSeconds();
                
                // Dance modulation frequencies: 4, 8, 16, 32, 64 Hz (higher frequencies for more dynamic movement)
                std::vector<float> frequencies = {4.0f, 8.0f, 16.0f, 32.0f, 64.0f};
                
                float vibrationX = 0.0f, vibrationY = 0.0f;
                for (float freq : frequencies) {
                    float amplitude = totalActivation * (1.0f / freq) * 0.5f; // More dramatic per-vertex movement
                    vibrationX += amplitude * std::sin(2.0f * M_PI * freq * currentTime + t * 5.0f);
                    vibrationY += amplitude * std::cos(2.0f * M_PI * freq * currentTime + t * 3.0f);
                }
                vibrationVector = sf::Vector2f(vibrationX, vibrationY);
            }
            
            sf::Vector2f point = basePoint + offsetVector + vibrationVector;
            sf::Vertex v;
            v.position = point;
            curveVertices.push_back(v);
        }
        thickCurveLines.push_back(curveVertices);
    }
    
    // Color based on connection weight and neuronal activity (weight already retrieved above)
    sf::Color baseColor = connectionColor;
    
    if (std::abs(weight) > 0.1f) {
        float intensity = std::min(1.0f, std::abs(weight));
    baseColor.a = static_cast<std::uint8_t>(100 + 155 * intensity);
        
        if (weight > 0) {
            baseColor = sf::Color(255, 255, 255, baseColor.a); // White for positive
        } else {
            baseColor = sf::Color(255, 100, 100, baseColor.a); // Red for negative
        }
    } else {
        baseColor.a = 50; // Very faint for near-zero weights
    }
    
    // Enhance color based on neuronal activity
    float activityBoost = std::min(1.0f, totalActivation);
    sf::Color activityColor = baseColor;
    
    if (activityBoost > 0.1f) {
        // Add brightness and saturation based on activity
    activityColor.r = static_cast<std::uint8_t>(std::min(255, static_cast<int>(baseColor.r + activityBoost * 100)));
    activityColor.g = static_cast<std::uint8_t>(std::min(255, static_cast<int>(baseColor.g + activityBoost * 50)));
    activityColor.b = static_cast<std::uint8_t>(std::min(255, static_cast<int>(baseColor.b + activityBoost * 150)));
    activityColor.a = static_cast<std::uint8_t>(std::min(255, static_cast<int>(baseColor.a + activityBoost * 100)));
    }
    
    // Draw all thick curve lines
    for (auto& curveVertices : thickCurveLines) {
        if (curveVertices.size() > 1) {
            // Apply color to all vertices in this line
            for (auto& vertex : curveVertices) {
                vertex.color = activityColor;
            }
            window->draw(&curveVertices[0], curveVertices.size(), sf::PrimitiveType::LineStrip);
        }
    }
}

sf::Vector2f Visualizer::calculateBezierPoint(const sf::Vector2f& p0, const sf::Vector2f& p1, 
                                              const sf::Vector2f& p2, float t) {
    // Quadratic Bezier curve: B(t) = (1-t)²P0 + 2(1-t)tP1 + t²P2
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    
    sf::Vector2f result = uu * p0 + 2.0f * u * t * p1 + tt * p2;
    return result;
}

bool Visualizer::loadFont(const std::string& fontPath) {
    fontLoaded = false;
    
    // Try to load the specified font first
    if (!fontPath.empty()) {
        if (font.openFromFile(fontPath)) {
            fontLoaded = true;
            std::cout << "Loaded font: " << fontPath << std::endl;
            return true;
        }
    }
    
    // Try to load system default font (SFML's built-in font is not available)
    // Try common system font paths
    std::vector<std::string> systemFonts = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf", 
        "/System/Library/Fonts/Helvetica.ttc",
        "/Windows/Fonts/arial.ttf",
        "assets/fonts/DejaVuSans.ttf"
    };
    
    for (const auto& path : systemFonts) {
        if (font.openFromFile(path)) {
            fontLoaded = true;
            std::cout << "Loaded system font: " << path << std::endl;
            return true;
        }
    }
    
    std::cout << "Warning: Could not load any font. Text rendering will be disabled." << std::endl;
    return false;
}

void Visualizer::updateFiringAnimations() {
    const float animationDuration = 0.3f; // 300ms animation
    const float maxScale = 1.1f; // Scale up to 110% of original size
    
    for (auto& anim : firingAnimations) {
        if (anim.isActive) {
            float elapsed = anim.timer.getElapsedTime().asSeconds();
            
            if (elapsed >= animationDuration) {
                // Animation complete
                anim.isActive = false;
                anim.scaleFactor = 1.0f;
            } else {
                // Calculate scale factor using a smooth ease-out curve
                float progress = elapsed / animationDuration;
                // Use sine wave for smooth scaling: grow fast, then shrink smoothly
                float sineProgress = std::sin(progress * 3.14159f); // 0 to π for half sine wave
                anim.scaleFactor = 1.0f + (maxScale - 1.0f) * sineProgress;
            }
        }
    }
}

void Visualizer::triggerFiringAnimation(size_t neuronIndex) {
    if (neuronIndex < firingAnimations.size()) {
        firingAnimations[neuronIndex].isActive = true;
        firingAnimations[neuronIndex].timer.restart();
        firingAnimations[neuronIndex].scaleFactor = 1.0f;
    }
}

sf::Color Visualizer::getRainbowColor(float activation, bool hasFired) const {
    // Map activation to rainbow spectrum: 0 = violet/blue, 1+ = red
    float normalizedActivation = std::min(2.0f, std::abs(activation)); // Cap at 2.0 for color mapping
    float hue = (1.0f - normalizedActivation / 2.0f) * 240.0f; // 240° (blue) to 0° (red)
    
    // Convert HSV to RGB
    float saturation = hasFired ? 1.0f : 0.7f; // More saturated when fired
    float value = 0.8f + (normalizedActivation / 2.0f) * 0.2f; // Brighter with higher activation
    
    // HSV to RGB conversion
    float c = value * saturation;
    float x = c * (1.0f - std::abs(std::fmod(hue / 60.0f, 2.0f) - 1.0f));
    float m = value - c;
    
    float r, g, b;
    
    if (hue >= 0 && hue < 60) {
        r = c; g = x; b = 0;
    } else if (hue >= 60 && hue < 120) {
        r = x; g = c; b = 0;
    } else if (hue >= 120 && hue < 180) {
        r = 0; g = c; b = x;
    } else if (hue >= 180 && hue < 240) {
        r = 0; g = x; b = c;
    } else if (hue >= 240 && hue < 300) {
        r = x; g = 0; b = c;
    } else {
        r = c; g = 0; b = x;
    }
    
    return sf::Color(
    static_cast<std::uint8_t>((r + m) * 255),
    static_cast<std::uint8_t>((g + m) * 255),
    static_cast<std::uint8_t>((b + m) * 255)
    );
}

void Visualizer::handleMouseMove(int mouseX, int mouseY) {
    mousePosition = sf::Vector2f(static_cast<float>(mouseX), static_cast<float>(mouseY));
    
    // Check if mouse is over a neuron
    int newHoveredIndex = getNeuronAtPosition(mousePosition);
    
    if (newHoveredIndex != hoveredNeuronIndex) {
        // Reset tooltip when switching neurons or leaving
        hoveredNeuronIndex = newHoveredIndex;
        showTooltip = false;
        tooltipTimer.restart();
        
        // Debug output
        if (newHoveredIndex >= 0) {
            std::cout << "🎯 Hovering over neuron " << newHoveredIndex << std::endl;
        }
    }
    
    // Start tooltip timer if hovering over a neuron
    if (hoveredNeuronIndex >= 0 && tooltipTimer.getElapsedTime().asSeconds() > TOOLTIP_DELAY) {
        if (!showTooltip) {
            std::cout << "📝 Showing tooltip for neuron " << hoveredNeuronIndex << std::endl;
        }
        showTooltip = true;
    }
}

int Visualizer::getNeuronAtPosition(const sf::Vector2f& position) const {
    if (!network) return -1;
    
    const auto& neurons = network->getNeurons();
    
    for (size_t i = 0; i < neurons.size() && i < neuronPositions.size(); ++i) {
        sf::Vector2f neuronPos = neuronPositions[i];
        
        // Calculate distance from mouse to neuron center
        float dx = position.x - neuronPos.x;
        float dy = position.y - neuronPos.y;
        float distanceSquared = dx * dx + dy * dy;
        
        // Check if within neuron radius (with some extra margin for easier hovering)
        float checkRadius = neuronRadius * 1.2f; // 20% larger hit area
        if (distanceSquared <= checkRadius * checkRadius) {
            return static_cast<int>(i);
        }
    }
    
    return -1; // No neuron found at this position
}

void Visualizer::drawTooltip(const std::string& text, const sf::Vector2f& position) {
    if (!fontLoaded || text.empty()) return;
    
    // Create tooltip text
    sf::Text tooltipText(font, text, 14);
    tooltipText.setFillColor(sf::Color::White);
    
    // Calculate tooltip background size
    sf::FloatRect textBounds = tooltipText.getLocalBounds();
    float padding = 8.0f;
    float tooltipWidth = textBounds.size.x + 2 * padding;
    float tooltipHeight = textBounds.size.y + 2 * padding;
    
    // Position tooltip slightly offset from mouse to avoid cursor overlap
    sf::Vector2f tooltipPos = position + sf::Vector2f(15.0f, -tooltipHeight - 10.0f);
    
    // Keep tooltip within window bounds
    sf::Vector2u windowSize = window->getSize();
    if (tooltipPos.x + tooltipWidth > windowSize.x) {
        tooltipPos.x = position.x - tooltipWidth - 15.0f;
    }
    if (tooltipPos.y < 0) {
        tooltipPos.y = position.y + 20.0f;
    }
    
    // Draw tooltip background
    sf::RectangleShape tooltipBg(sf::Vector2f(tooltipWidth, tooltipHeight));
    tooltipBg.setPosition(tooltipPos);
    tooltipBg.setFillColor(sf::Color(50, 50, 50, 230)); // Semi-transparent dark background
    tooltipBg.setOutlineThickness(1.0f);
    tooltipBg.setOutlineColor(sf::Color(150, 150, 150, 200));
    window->draw(tooltipBg);
    
    // Draw tooltip text
    tooltipText.setPosition(tooltipPos + sf::Vector2f(padding, padding));
    window->draw(tooltipText);
}