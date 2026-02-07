#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <string>
#include "ModularComponent.h"
#include "MockModule.h"
// Full module includes commented out for simple demo
// Uncomment when full module implementations are ready
// #include "AudioManagerModule.h"
// #include "RecorderModule.h"
// #include "NeuronNetworkModule.h"
// #include "BeatTrackerModule.h"
// #include "RhythmogramModule.h"
// #include "QuantizerModule.h"

/**
 * @brief Modular GUI2 system for neuronSeqSampler
 * 
 * Features:
 * - Module selection from a list
 * - Drag-and-drop module placement on canvas
 * - Visual connection routing with drag-and-drop
 * - Parameter windows for each module
 * - Real-time signal flow visualization
 */
class GUI2 {
public:
    /**
     * @brief Module canvas interaction states
     */
    enum class InteractionMode {
        IDLE,               // No interaction
        DRAGGING_MODULE,    // Dragging a module
        DRAGGING_CONNECTION,// Creating a connection
        PANNING_CANVAS,     // Panning the view
        SELECTING           // Selecting modules
    };

    /**
     * @brief Visual connection representation
     */
    struct VisualConnection {
        ModularComponent* sourceModule;
        int sourcePortIndex;
        ModularComponent* destModule;
        int destPortIndex;
        sf::Color connectionColor;
        
        VisualConnection(ModularComponent* src, int srcPort, 
                        ModularComponent* dst, int dstPort)
            : sourceModule(src), sourcePortIndex(srcPort),
              destModule(dst), destPortIndex(dstPort),
              connectionColor(sf::Color(100, 200, 255)) {}
    };

private:
    // SFML window
    sf::RenderWindow* window;
    sf::Font font;
    bool fontLoaded;
    
    // Canvas properties
    sf::Vector2f canvasOffset;
    float canvasZoom;
    sf::FloatRect canvasBounds;
    
    // Module management
    std::vector<std::unique_ptr<ModularComponent>> modules;
    std::vector<VisualConnection> connections;
    std::vector<std::string> availableModuleTypes;
    
    // Interaction state
    InteractionMode currentMode;
    ModularComponent* selectedModule;
    ModularComponent* draggedModule;
    sf::Vector2f dragOffset;
    
    // Connection creation state
    ModularComponent* connectionSourceModule;
    int connectionSourcePort;
    bool connectionSourceIsInput;
    sf::Vector2f connectionDragPoint;
    
    // Module list panel
    sf::FloatRect moduleListBounds;
    bool moduleListVisible;
    int hoveredModuleType;
    
    // Parameter window
    sf::FloatRect parameterWindowBounds;
    bool parameterWindowVisible;
    ModularComponent* parameterWindowModule;
    
    // Canvas rendering
    sf::RectangleShape canvasBackground;
    sf::RectangleShape moduleListBackground;
    sf::RectangleShape parameterWindowBackground;
    
    // Grid rendering
    bool showGrid;
    float gridSpacing;

public:
    GUI2(sf::RenderWindow* win);
    ~GUI2();
    
    // ========================= INITIALIZATION =========================
    
    /**
     * @brief Initialize GUI2 system
     */
    void initialize();
    
    /**
     * @brief Load fonts and resources
     */
    bool loadResources();

    // ========================= UPDATE & RENDER =========================
    
    /**
     * @brief Update GUI state
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);
    
    /**
     * @brief Render the GUI
     */
    void render();
    
    /**
     * @brief Render the module canvas
     */
    void renderCanvas();
    
    /**
     * @brief Render all modules
     */
    void renderModules();
    
    /**
     * @brief Render all connections
     */
    void renderConnections();
    
    /**
     * @brief Render the module list panel
     */
    void renderModuleList();
    
    /**
     * @brief Render the parameter window
     */
    void renderParameterWindow();
    
    /**
     * @brief Render canvas grid
     */
    void renderGrid();

    // ========================= INPUT HANDLING =========================
    
    /**
     * @brief Handle mouse button press
     */
    void handleMousePress(const sf::Event& event);
    
    /**
     * @brief Handle mouse button release
     */
    void handleMouseRelease(const sf::Event& event);
    
    /**
     * @brief Handle mouse movement
     */
    void handleMouseMove(const sf::Event& event);
    
    /**
     * @brief Handle mouse wheel
     */
    void handleMouseWheel(const sf::Event& event);
    
    /**
     * @brief Handle keyboard input
     */
    void handleKeyPress(const sf::Event& event);

    // ========================= MODULE MANAGEMENT =========================
    
    /**
     * @brief Add a new module to the canvas
     * @param moduleType Type of module to add
     * @param position Position on canvas
     * @return Pointer to created module
     */
    ModularComponent* addModule(const std::string& moduleType, const sf::Vector2f& position);
    
    /**
     * @brief Remove a module from the canvas
     * @param module Module to remove
     */
    void removeModule(ModularComponent* module);
    
    /**
     * @brief Get module at screen position
     * @param position Screen position
     * @return Pointer to module or nullptr
     */
    ModularComponent* getModuleAt(const sf::Vector2f& position);
    
    /**
     * @brief Select a module
     */
    void selectModule(ModularComponent* module);
    
    /**
     * @brief Deselect all modules
     */
    void deselectAll();

    // ========================= CONNECTION MANAGEMENT =========================
    
    /**
     * @brief Create a connection between two ports
     * @param sourceModule Source module
     * @param sourcePort Source port index
     * @param destModule Destination module
     * @param destPort Destination port index
     * @return true if connection was created successfully
     */
    bool createConnection(ModularComponent* sourceModule, int sourcePort,
                         ModularComponent* destModule, int destPort);
    
    /**
     * @brief Remove a connection
     */
    void removeConnection(const VisualConnection& connection);
    
    /**
     * @brief Remove all connections to/from a module
     */
    void removeConnectionsForModule(ModularComponent* module);
    
    /**
     * @brief Check if connection is valid
     */
    bool isValidConnection(ModularComponent* sourceModule, int sourcePort, bool sourceIsInput,
                          ModularComponent* destModule, int destPort, bool destIsInput);
    
    /**
     * @brief Start creating a connection
     */
    void startConnectionDrag(ModularComponent* module, int portIndex, bool isInput);
    
    /**
     * @brief Update connection drag
     */
    void updateConnectionDrag(const sf::Vector2f& mousePos);
    
    /**
     * @brief Finish creating a connection
     */
    void finishConnectionDrag(const sf::Vector2f& mousePos);

    // ========================= UTILITY =========================
    
    /**
     * @brief Convert screen position to canvas position
     */
    sf::Vector2f screenToCanvas(const sf::Vector2f& screenPos) const;
    
    /**
     * @brief Convert canvas position to screen position
     */
    sf::Vector2f canvasToScreen(const sf::Vector2f& canvasPos) const;
    
    /**
     * @brief Open parameter window for a module
     */
    void openParameterWindow(ModularComponent* module);
    
    /**
     * @brief Close parameter window
     */
    void closeParameterWindow();
    
    /**
     * @brief Toggle module list visibility
     */
    void toggleModuleList();
    
    /**
     * @brief Save canvas state
     */
    void saveCanvas(const std::string& filename);
    
    /**
     * @brief Load canvas state
     */
    void loadCanvas(const std::string& filename);
};
