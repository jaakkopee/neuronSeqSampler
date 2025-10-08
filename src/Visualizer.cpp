#include "Visualizer.h"
#include "NeuronNetwork.h"
#include "Neuron.h"
#include "Connection.h"
#include <cmath>

Visualizer::Visualizer(sf::RenderWindow* renderWindow, NeuronNetwork* neuronNetwork)
    : window(renderWindow)
    , network(neuronNetwork)
    , neuronRadius(15.0f)
    , normalNeuronColor(sf::Color::Blue)
    , firedNeuronColor(sf::Color::Red)
    , connectionColor(sf::Color::White)
    , highlightConnectionColor(sf::Color::Yellow)
    , canvasOffset(50.0f, 50.0f)
    , canvasSize(500.0f, 400.0f)
    , currentViewMode(ViewMode::Grid)
{
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
    
    // Draw neurons (reuse neurons variable from above)
    for (size_t i = 0; i < neurons.size() && i < neuronPositions.size(); ++i) {
        drawNeuron(neurons[i].get(), neuronPositions[i]);
    }
}

void Visualizer::drawNeuron(const Neuron* neuron, const sf::Vector2f& position) {
    sf::CircleShape circle(neuronRadius);
    circle.setOrigin(neuronRadius, neuronRadius);
    circle.setPosition(position);
    
    // Choose color based on whether the neuron has fired
    sf::Color color = neuron->getHasFired() ? firedNeuronColor : normalNeuronColor;
    
    // Add intensity based on activation level
    float activation = std::abs(neuron->getActivation());
    float intensity = std::min(1.0f, activation / 2.0f); // Scale activation to 0-1
    
    color.r = static_cast<sf::Uint8>(color.r * intensity + (255 - color.r) * (1.0f - intensity));
    color.g = static_cast<sf::Uint8>(color.g * intensity + (255 - color.g) * (1.0f - intensity));
    color.b = static_cast<sf::Uint8>(color.b * intensity + (255 - color.b) * (1.0f - intensity));
    
    circle.setFillColor(color);
    circle.setOutlineThickness(2.0f);
    circle.setOutlineColor(sf::Color::White);
    
    window->draw(circle);
    
    // Note: Text rendering would require loading a font
    // For now, neuron identification is by position only
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
        baseColor.a = static_cast<sf::Uint8>(100 + 155 * intensity);
        
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
        activityColor.r = static_cast<sf::Uint8>(std::min(255, static_cast<int>(baseColor.r + activityBoost * 100)));
        activityColor.g = static_cast<sf::Uint8>(std::min(255, static_cast<int>(baseColor.g + activityBoost * 50)));
        activityColor.b = static_cast<sf::Uint8>(std::min(255, static_cast<int>(baseColor.b + activityBoost * 150)));
        activityColor.a = static_cast<sf::Uint8>(std::min(255, static_cast<int>(baseColor.a + activityBoost * 100)));
    }
    
    // Draw single thick line with vibration effect
    sf::Vector2f direction = targetPos - sourcePos;
    sf::Vector2f perpendicular(-direction.y, direction.x);
    float lineLength = std::sqrt(perpendicular.x * perpendicular.x + perpendicular.y * perpendicular.y);
    if (lineLength > 0) {
        perpendicular.x /= lineLength;
        perpendicular.y /= lineLength;
    }
    
    float vibrationIntensity = totalActivation * 4.5f; // Amplitude for straight lines
    
    // Apply vibration to endpoints
    float vibrationX = std::sin(totalActivation * 12.0f) * vibrationIntensity;
    float vibrationY = std::cos(totalActivation * 10.0f) * vibrationIntensity;
    sf::Vector2f vibration(vibrationX, vibrationY);
    
    sf::Vector2f vibratingSourcePos = sourcePos + vibration;
    sf::Vector2f vibratingTargetPos = targetPos + vibration;
    
    // Create thick line using rectangle shape
    sf::RectangleShape line;
    sf::Vector2f lineVector = vibratingTargetPos - vibratingSourcePos;
    float length = std::sqrt(lineVector.x * lineVector.x + lineVector.y * lineVector.y);
    float angle = std::atan2(lineVector.y, lineVector.x) * 180.0f / M_PI;
    
    line.setSize(sf::Vector2f(length, 3.0f));
    line.setPosition(vibratingSourcePos);
    line.setOrigin(0, 1.5f);
    line.setRotation(angle);
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
    
    // Base curve offset with smaller amplitude vibration
    float vibrationIntensity = totalActivation * 2.0f; // Reduced from 10.0f for smaller amplitude
    float vibrationOffset = std::sin(totalActivation * 12.0f) * vibrationIntensity; // Synchronized rhythm
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
            
            // Add subtle vibration to each point - smaller amplitude, synchronized rhythm
            float vibrationX = std::sin(t * 25.0f + totalActivation * 12.0f) * vibrationIntensity * 0.15f; // Reduced from 0.3f
            float vibrationY = std::cos(t * 20.0f + totalActivation * 10.0f) * vibrationIntensity * 0.15f; // Reduced from 0.3f
            
            sf::Vector2f point = basePoint + offsetVector + sf::Vector2f(vibrationX, vibrationY);
            curveVertices.push_back(sf::Vertex(point));
        }
        thickCurveLines.push_back(curveVertices);
    }
    
    // Color based on connection weight and neuronal activity
    float weight = connection->getWeight();
    sf::Color baseColor = connectionColor;
    
    if (std::abs(weight) > 0.1f) {
        float intensity = std::min(1.0f, std::abs(weight));
        baseColor.a = static_cast<sf::Uint8>(100 + 155 * intensity);
        
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
        activityColor.r = static_cast<sf::Uint8>(std::min(255, static_cast<int>(baseColor.r + activityBoost * 100)));
        activityColor.g = static_cast<sf::Uint8>(std::min(255, static_cast<int>(baseColor.g + activityBoost * 50)));
        activityColor.b = static_cast<sf::Uint8>(std::min(255, static_cast<int>(baseColor.b + activityBoost * 150)));
        activityColor.a = static_cast<sf::Uint8>(std::min(255, static_cast<int>(baseColor.a + activityBoost * 100)));
    }
    
    // Draw all thick curve lines
    for (auto& curveVertices : thickCurveLines) {
        if (curveVertices.size() > 1) {
            // Apply color to all vertices in this line
            for (auto& vertex : curveVertices) {
                vertex.color = activityColor;
            }
            window->draw(&curveVertices[0], curveVertices.size(), sf::LineStrip);
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