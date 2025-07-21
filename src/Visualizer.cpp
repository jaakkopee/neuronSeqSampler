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
{
    calculateNeuronPositions();
}

void Visualizer::setCanvasArea(float x, float y, float width, float height) {
    canvasOffset = sf::Vector2f(x, y);
    canvasSize = sf::Vector2f(width, height);
    calculateNeuronPositions();
}

void Visualizer::calculateNeuronPositions() {
    if (!network) return;
    
    neuronPositions.clear();
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

void Visualizer::render() {
    if (!network) return;
    
    // Draw connections first (so they appear behind neurons)
    const auto& connections = network->getConnections();
    for (size_t i = 0; i < connections.size(); ++i) {
        const Connection* conn = connections[i].get();
        
        // Find source and target neuron positions
        const auto& neurons = network->getNeurons();
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
            drawConnection(conn, neuronPositions[sourceIndex], neuronPositions[targetIndex]);
        }
    }
    
    // Draw neurons
    const auto& neurons = network->getNeurons();
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
    sf::Vertex line[] = {
        sf::Vertex(sourcePos),
        sf::Vertex(targetPos)
    };
    
    // Color based on connection weight
    float weight = connection->getWeight();
    sf::Color color = connectionColor;
    
    if (std::abs(weight) > 0.1f) {
        // Make the line more visible for significant weights
        float intensity = std::min(1.0f, std::abs(weight));
        color.a = static_cast<sf::Uint8>(100 + 155 * intensity);
        
        // Use different colors for positive/negative weights
        if (weight > 0) {
            color = sf::Color(255, 255, 255, color.a); // White for positive
        } else {
            color = sf::Color(255, 100, 100, color.a); // Red for negative
        }
    } else {
        color.a = 50; // Very faint for near-zero weights
    }
    
    line[0].color = color;
    line[1].color = color;
    
    window->draw(line, 2, sf::Lines);
}