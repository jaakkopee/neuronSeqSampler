#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

// Forward declarations
class NeuronNetwork;
class Neuron;
class Connection;

class Visualizer {
private:
    sf::RenderWindow* window;
    NeuronNetwork* network;
    
    // Visual settings
    float neuronRadius;
    sf::Color normalNeuronColor;
    sf::Color firedNeuronColor;
    sf::Color connectionColor;
    sf::Color highlightConnectionColor;
    
    // Layout settings
    sf::Vector2f canvasOffset;
    sf::Vector2f canvasSize;
    
    // Visual elements
    std::vector<sf::Vector2f> neuronPositions;
    
    void calculateNeuronPositions();
    void drawNeuron(const Neuron* neuron, const sf::Vector2f& position);
    void drawConnection(const Connection* connection, 
                       const sf::Vector2f& sourcePos, 
                       const sf::Vector2f& targetPos);

public:
    Visualizer(sf::RenderWindow* renderWindow, NeuronNetwork* neuronNetwork);
    
    void setCanvasArea(float x, float y, float width, float height);
    void render();
    
    // Settings
    void setNeuronRadius(float radius) { neuronRadius = radius; }
    void setNeuronColors(const sf::Color& normal, const sf::Color& fired) {
        normalNeuronColor = normal;
        firedNeuronColor = fired;
    }
    void setConnectionColors(const sf::Color& normal, const sf::Color& highlight) {
        connectionColor = normal;
        highlightConnectionColor = highlight;
    }
};