#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <cmath>
#include <string>
#include <vector>

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
    
    // Text rendering
    sf::Font font;
    bool fontLoaded;
    
    // Animation timing
    sf::Clock animationClock;
    
    // Firing animation state
    struct FiringAnimation {
        bool isActive = false;
        sf::Clock timer;
        float scaleFactor = 1.0f;
        bool previouslyFired = false;
    };
    std::vector<FiringAnimation> firingAnimations;
    
    // Layout settings
    sf::Vector2f canvasOffset;
    sf::Vector2f canvasSize;
    ViewMode currentViewMode;
    
    // Zoom and pan
    float zoomLevel;
    sf::Vector2f panOffset;
    
    // Visual elements
    std::vector<sf::Vector2f> neuronPositions;
    
    // Tooltip state
    int hoveredNeuronIndex;
    sf::Vector2f mousePosition;
    sf::Clock tooltipTimer;
    bool showTooltip;
    static constexpr float TOOLTIP_DELAY = 0.5f; // Show tooltip after 0.5 seconds

    // Optional target pattern image used for pattern convergence learning
    sf::Image targetPatternImage;
    bool targetPatternLoaded;
    
    void calculateNeuronPositions();
    void calculateGridPositions();
    void calculateCircularPositions();
    sf::Color getRainbowColor(float activation, bool hasFired) const;
    void updateFiringAnimations();
    void triggerFiringAnimation(size_t neuronIndex);
    void drawNeuron(const Neuron* neuron, const sf::Vector2f& position, size_t neuronIndex);
    void drawConnection(const Connection* connection, 
                       const sf::Vector2f& sourcePos, 
                       const sf::Vector2f& targetPos);
    void drawCurvedConnection(const Connection* connection,
                             const sf::Vector2f& sourcePos,
                             const sf::Vector2f& targetPos,
                             bool isReverse = false);
    void drawSelfConnection(const Connection* connection, const sf::Vector2f& neuronPos);
    sf::Vector2f calculateBezierPoint(const sf::Vector2f& p0, const sf::Vector2f& p1, 
                                     const sf::Vector2f& p2, float t);
    void drawTooltip(const std::string& text, const sf::Vector2f& position);
    int getNeuronAtPosition(const sf::Vector2f& position) const;

public:
    Visualizer(sf::RenderWindow* renderWindow, NeuronNetwork* neuronNetwork);
    
    void setCanvasArea(float x, float y, float width, float height);
    void render();
    void refreshLayout(); // Call this when the network structure changes
    bool loadFont(const std::string& fontPath); // Load font for text rendering
    bool loadTargetPatternImage(const std::string& imagePath);
    void clearTargetPatternImage();
    bool hasTargetPatternImage() const { return targetPatternLoaded; }
    std::vector<float> sampleTargetPatternAtNeurons() const;
    std::vector<float> captureCurrentNeuronPattern() const;
    
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
    void handleMouseDrag(int mouseX, int mouseY); // For panning
    void handleMouseScroll(int delta); // No-op (zoom removed)
    void handleMouseMove(int mouseX, int mouseY); // For tooltip hover detection
    void handleKeyPress(sf::Keyboard::Key key); // For zoom and pan
    
    // View mode controls
    void setViewMode(ViewMode mode);
    ViewMode getViewMode() const { return currentViewMode; }
};