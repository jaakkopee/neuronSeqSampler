#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <cmath>

// Forward declarations
class NeuronNetwork;
class Neuron;
class Connection;

enum class ViewMode {
    Grid,
    Circular
};

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
    ViewMode currentViewMode;
    
    // Visual elements
    std::vector<sf::Vector2f> neuronPositions;
    
    void calculateNeuronPositions();
    void calculateGridPositions();
    void calculateCircularPositions();
    void drawNeuron(const Neuron* neuron, const sf::Vector2f& position);
    void drawConnection(const Connection* connection, 
                       const sf::Vector2f& sourcePos, 
                       const sf::Vector2f& targetPos);
    void drawCurvedConnection(const Connection* connection,
                             const sf::Vector2f& sourcePos,
                             const sf::Vector2f& targetPos,
                             bool isReverse = false);
    sf::Vector2f calculateBezierPoint(const sf::Vector2f& p0, const sf::Vector2f& p1, 
                                     const sf::Vector2f& p2, float t);

public:
    Visualizer(sf::RenderWindow* renderWindow, NeuronNetwork* neuronNetwork);
    
    void setCanvasArea(float x, float y, float width, float height);
    void render();
    void refreshLayout(); // Call this when the network structure changes
    
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
    void handleMouseDrag(int mouseX, int mouseY); // For panning and zooming
    void handleMouseScroll(int delta); // For zooming
    
    // View mode controls
    void setViewMode(ViewMode mode);
    ViewMode getViewMode() const { return currentViewMode; }
};