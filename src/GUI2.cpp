#include "GUI2.h"
#include <algorithm>
#include <cmath>
#include <iostream>

GUI2::GUI2(sf::RenderWindow* win)
    : window(win), fontLoaded(false),
      canvasOffset(0, 0), canvasZoom(1.0f),
      canvasBounds(sf::Vector2f(0, 0), sf::Vector2f(10000, 10000)),
      currentMode(InteractionMode::IDLE),
      selectedModule(nullptr), draggedModule(nullptr),
      connectionSourceModule(nullptr), connectionSourcePort(-1),
      connectionSourceIsInput(false),
      moduleListVisible(true), hoveredModuleType(-1),
      parameterWindowVisible(false), parameterWindowModule(nullptr),
      showGrid(true), gridSpacing(50.0f) {
    
    // Initialize available module types
    availableModuleTypes = {
        "AudioManager",
        "Recorder",
        "NeuronNetwork",
        "BeatTracker",
        "Rhythmogram",
        "Quantizer"
    };
    
    // Set up UI bounds (SFML 3 style)
    moduleListBounds = sf::FloatRect(sf::Vector2f(10, 10), sf::Vector2f(200, 400));
    auto winSize = window->getSize();
    parameterWindowBounds = sf::FloatRect(sf::Vector2f(winSize.x - 310, 10), sf::Vector2f(300, 500));
    
    // Set up backgrounds
    canvasBackground.setFillColor(sf::Color(30, 30, 40));
    moduleListBackground.setFillColor(sf::Color(40, 40, 50, 230));
    moduleListBackground.setOutlineColor(sf::Color(100, 100, 120));
    moduleListBackground.setOutlineThickness(2.0f);
    parameterWindowBackground.setFillColor(sf::Color(40, 40, 50, 230));
    parameterWindowBackground.setOutlineColor(sf::Color(100, 100, 120));
    parameterWindowBackground.setOutlineThickness(2.0f);
}

GUI2::~GUI2() {
    modules.clear();
    connections.clear();
}

void GUI2::initialize() {
    loadResources();
}

bool GUI2::loadResources() {
    // Try to load a system font (SFML 3 uses openFromFile)
    if (font.openFromFile("/System/Library/Fonts/Helvetica.ttc") ||
        font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") ||
        font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
        fontLoaded = true;
        return true;
    }
    
    std::cerr << "Warning: Could not load font for GUI2" << std::endl;
    return false;
}

void GUI2::update(float deltaTime) {
    // Update all modules
    for (auto& module : modules) {
        if (module && module->isEnabled()) {
            module->process(deltaTime);
        }
    }
}

void GUI2::render() {
    if (!window) return;
    
    // Render canvas
    renderCanvas();
    renderGrid();
    renderConnections();
    renderModules();
    
    // Render UI panels
    if (moduleListVisible) {
        renderModuleList();
    }
    
    if (parameterWindowVisible && parameterWindowModule) {
        renderParameterWindow();
    }
    
    // Render connection being dragged
    if (currentMode == InteractionMode::DRAGGING_CONNECTION && connectionSourceModule) {
        sf::Vector2f startPos = canvasToScreen(
            connectionSourceModule->getPortPosition(connectionSourcePort, connectionSourceIsInput)
        );
        
        sf::Vertex line[] = {
            sf::Vertex{.position = startPos, .color = sf::Color(100, 200, 255)},
            sf::Vertex{.position = connectionDragPoint, .color = sf::Color(100, 200, 255)}
        };
        window->draw(line, 2, sf::PrimitiveType::Lines);
        
        // Draw circle at drag point
        sf::CircleShape dragCircle(6.0f);
        dragCircle.setPosition(connectionDragPoint - sf::Vector2f(6, 6));
        dragCircle.setFillColor(sf::Color(100, 200, 255));
        window->draw(dragCircle);
    }
}

void GUI2::renderCanvas() {
    canvasBackground.setSize(sf::Vector2f(window->getSize()));
    canvasBackground.setPosition(sf::Vector2f(0, 0));
    window->draw(canvasBackground);
}

void GUI2::renderGrid() {
    if (!showGrid) return;
    
    sf::Vector2f windowSize(window->getSize());
    sf::Color gridColor(60, 60, 70, 100);
    
    // Calculate grid bounds based on visible area
    float startX = std::floor((-canvasOffset.x) / gridSpacing) * gridSpacing;
    float startY = std::floor((-canvasOffset.y) / gridSpacing) * gridSpacing;
    float endX = startX + windowSize.x / canvasZoom + gridSpacing;
    float endY = startY + windowSize.y / canvasZoom + gridSpacing;
    
    // Vertical lines
    for (float x = startX; x < endX; x += gridSpacing) {
        sf::Vector2f screenPos = canvasToScreen(sf::Vector2f(x, 0));
        sf::Vertex line[] = {
            sf::Vertex{.position = sf::Vector2f(screenPos.x, 0), .color = gridColor},
            sf::Vertex{.position = sf::Vector2f(screenPos.x, windowSize.y), .color = gridColor}
        };
        window->draw(line, 2, sf::PrimitiveType::Lines);
    }
    
    // Horizontal lines
    for (float y = startY; y < endY; y += gridSpacing) {
        sf::Vector2f screenPos = canvasToScreen(sf::Vector2f(0, y));
        sf::Vertex line[] = {
            sf::Vertex{.position = sf::Vector2f(0, screenPos.y), .color = gridColor},
            sf::Vertex{.position = sf::Vector2f(windowSize.x, screenPos.y), .color = gridColor}
        };
        window->draw(line, 2, sf::PrimitiveType::Lines);
    }
}

void GUI2::renderModules() {
    for (auto& module : modules) {
        if (!module) continue;
        
        sf::Vector2f screenPos = canvasToScreen(module->getPosition());
        sf::Vector2f size = module->getSize() * canvasZoom;
        
        // Draw module rectangle
        sf::RectangleShape moduleRect(size);
        moduleRect.setPosition(screenPos);
        moduleRect.setFillColor(module->getColor());
        
        if (module->isSelected()) {
            moduleRect.setOutlineColor(sf::Color::Yellow);
            moduleRect.setOutlineThickness(3.0f);
        } else {
            moduleRect.setOutlineColor(sf::Color(200, 200, 200));
            moduleRect.setOutlineThickness(1.5f);
        }
        
        window->draw(moduleRect);
        
        // Draw module name
        if (fontLoaded) {
            sf::Text nameText(font, module->getName(), 14);
            nameText.setFillColor(sf::Color::White);
            nameText.setPosition(sf::Vector2f(screenPos.x + 5, screenPos.y + 5));
            window->draw(nameText);
        }
        
        // Draw input ports
        const auto& inputPorts = module->getInputPorts();
        for (size_t i = 0; i < inputPorts.size(); i++) {
            sf::Vector2f portPos = canvasToScreen(module->getPortPosition(i, true));
            sf::CircleShape portCircle(6.0f * canvasZoom);
            portCircle.setPosition(portPos - sf::Vector2f(6.0f * canvasZoom, 6.0f * canvasZoom));
            portCircle.setFillColor(sf::Color(100, 255, 100));
            portCircle.setOutlineColor(sf::Color::White);
            portCircle.setOutlineThickness(1.0f);
            window->draw(portCircle);
        }
        
        // Draw output ports
        const auto& outputPorts = module->getOutputPorts();
        for (size_t i = 0; i < outputPorts.size(); i++) {
            sf::Vector2f portPos = canvasToScreen(module->getPortPosition(i, false));
            sf::CircleShape portCircle(6.0f * canvasZoom);
            portCircle.setPosition(portPos - sf::Vector2f(6.0f * canvasZoom, 6.0f * canvasZoom));
            portCircle.setFillColor(sf::Color(255, 100, 100));
            portCircle.setOutlineColor(sf::Color::White);
            portCircle.setOutlineThickness(1.0f);
            window->draw(portCircle);
        }
    }
}

void GUI2::renderConnections() {
    for (const auto& conn : connections) {
        if (!conn.sourceModule || !conn.destModule) continue;
        
        sf::Vector2f startPos = canvasToScreen(
            conn.sourceModule->getPortPosition(conn.sourcePortIndex, false)
        );
        sf::Vector2f endPos = canvasToScreen(
            conn.destModule->getPortPosition(conn.destPortIndex, true)
        );
        
        // Draw curved connection using bezier-like approximation
        float midX = (startPos.x + endPos.x) / 2.0f;
        sf::Vector2f cp1(midX, startPos.y);
        sf::Vector2f cp2(midX, endPos.y);
        
        // Simple line for now
        sf::Vertex line[] = {
            sf::Vertex{.position = startPos, .color = conn.connectionColor},
            sf::Vertex{.position = endPos, .color = conn.connectionColor}
        };
        window->draw(line, 2, sf::PrimitiveType::Lines);
        
        // Draw arrow at end
        sf::CircleShape arrow(4.0f);
        arrow.setPosition(endPos - sf::Vector2f(4, 4));
        arrow.setFillColor(conn.connectionColor);
        window->draw(arrow);
    }
}

void GUI2::renderModuleList() {
    moduleListBackground.setPosition(moduleListBounds.position);
    moduleListBackground.setSize(moduleListBounds.size);
    window->draw(moduleListBackground);
    
    if (!fontLoaded) return;
    
    // Title
    sf::Text titleText(font, "Available Modules", 16);
    titleText.setFillColor(sf::Color::White);
    titleText.setStyle(sf::Text::Bold);
    titleText.setPosition(sf::Vector2f(moduleListBounds.position.x + 10, moduleListBounds.position.y + 10));
    window->draw(titleText);
    
    // Module list
    float yOffset = moduleListBounds.position.y + 40;
    for (size_t i = 0; i < availableModuleTypes.size(); i++) {
        sf::RectangleShape itemRect(sf::Vector2f(moduleListBounds.size.x - 20, 30));
        itemRect.setPosition(sf::Vector2f(moduleListBounds.position.x + 10, yOffset));
        
        if (static_cast<int>(i) == hoveredModuleType) {
            itemRect.setFillColor(sf::Color(80, 80, 100));
        } else {
            itemRect.setFillColor(sf::Color(60, 60, 70));
        }
        
        window->draw(itemRect);
        
        sf::Text moduleText(font, availableModuleTypes[i], 14);
        moduleText.setFillColor(sf::Color::White);
        moduleText.setPosition(sf::Vector2f(moduleListBounds.position.x + 15, yOffset + 5));
        window->draw(moduleText);
        
        yOffset += 35;
    }
}

void GUI2::renderParameterWindow() {
    if (!parameterWindowModule) return;
    
    parameterWindowBackground.setPosition(parameterWindowBounds.position);
    parameterWindowBackground.setSize(parameterWindowBounds.size);
    window->draw(parameterWindowBackground);
    
    if (!fontLoaded) return;
    
    // Title
    sf::Text titleText(font, parameterWindowModule->getName() + " Parameters", 16);
    titleText.setFillColor(sf::Color::White);
    titleText.setStyle(sf::Text::Bold);
    titleText.setPosition(sf::Vector2f(parameterWindowBounds.position.x + 10, parameterWindowBounds.position.y + 10));
    window->draw(titleText);
    
    // Parameters
    float yOffset = parameterWindowBounds.position.y + 50;
    const auto& params = parameterWindowModule->getParameters();
    
    for (const auto& param : params) {
        // Parameter name
        sf::Text paramNameText(font, param.name, 12);
        paramNameText.setFillColor(sf::Color::White);
        paramNameText.setPosition(sf::Vector2f(parameterWindowBounds.position.x + 15, yOffset));
        window->draw(paramNameText);
        
        // Parameter value
        std::string valueStr = std::to_string(param.value);
        if (!param.unit.empty()) {
            valueStr += " " + param.unit;
        }
        sf::Text paramValueText(font, valueStr, 12);
        paramValueText.setFillColor(sf::Color(200, 200, 255));
        paramValueText.setPosition(sf::Vector2f(parameterWindowBounds.position.x + 15, yOffset + 20));
        window->draw(paramValueText);
        
        // Slider representation
        float sliderWidth = parameterWindowBounds.size.x - 30;
        float normalizedValue = (param.value - param.minValue) / (param.maxValue - param.minValue);
        
        sf::RectangleShape sliderBg(sf::Vector2f(sliderWidth, 10));
        sliderBg.setPosition(sf::Vector2f(parameterWindowBounds.position.x + 15, yOffset + 40));
        sliderBg.setFillColor(sf::Color(60, 60, 70));
        window->draw(sliderBg);
        
        sf::RectangleShape sliderFill(sf::Vector2f(sliderWidth * normalizedValue, 10));
        sliderFill.setPosition(sf::Vector2f(parameterWindowBounds.position.x + 15, yOffset + 40));
        sliderFill.setFillColor(sf::Color(100, 150, 255));
        window->draw(sliderFill);
        
        yOffset += 65;
    }
    
    // Close button
    sf::RectangleShape closeButton(sf::Vector2f(80, 30));
    closeButton.setPosition(sf::Vector2f(parameterWindowBounds.position.x + parameterWindowBounds.size.x - 90,
                             parameterWindowBounds.position.y + 10));
    closeButton.setFillColor(sf::Color(100, 50, 50));
    window->draw(closeButton);
    
    sf::Text closeText(font, "Close", 14);
    closeText.setFillColor(sf::Color::White);
    closeText.setPosition(sf::Vector2f(closeButton.getPosition().x + 20, closeButton.getPosition().y + 8));
    window->draw(closeText);
}

void GUI2::handleMousePress(const sf::Event& event) {
    const auto* mousePress = event.getIf<sf::Event::MouseButtonPressed>();
    if (!mousePress || mousePress->button != sf::Mouse::Button::Left) return;
    
    sf::Vector2f mousePos(static_cast<float>(mousePress->position.x), 
                         static_cast<float>(mousePress->position.y));
    
    // Check module list
    if (moduleListVisible && moduleListBounds.contains(mousePos)) {
        float yOffset = moduleListBounds.position.y + 40;
        for (size_t i = 0; i < availableModuleTypes.size(); i++) {
            sf::FloatRect itemRect(sf::Vector2f(moduleListBounds.position.x + 10, yOffset), 
                                  sf::Vector2f(moduleListBounds.size.x - 20, 30));
            if (itemRect.contains(mousePos)) {
                // Add module at center of canvas
                sf::Vector2f canvasPos = screenToCanvas(
                    sf::Vector2f(window->getSize().x / 2, window->getSize().y / 2)
                );
                addModule(availableModuleTypes[i], canvasPos);
                return;
            }
            yOffset += 35;
        }
        return;
    }
    
    // Check parameter window close button
    if (parameterWindowVisible && parameterWindowModule) {
        sf::FloatRect closeButtonRect(
            sf::Vector2f(parameterWindowBounds.position.x + parameterWindowBounds.size.x - 90,
                        parameterWindowBounds.position.y + 10),
            sf::Vector2f(80, 30)
        );
        if (closeButtonRect.contains(mousePos)) {
            closeParameterWindow();
            return;
        }
        
        if (parameterWindowBounds.contains(mousePos)) {
            // Handle parameter interaction
            return;
        }
    }
    
    // Check modules and ports
    sf::Vector2f canvasPos = screenToCanvas(mousePos);
    
    for (auto& module : modules) {
        if (!module) continue;
        
        // Check if clicking on a port
        bool isInput = false;
        int portIndex = module->hitTestPort(canvasPos, isInput);
        
        if (portIndex >= 0) {
            startConnectionDrag(module.get(), portIndex, isInput);
            return;
        }
        
        // Check if clicking on module body
        if (module->contains(canvasPos)) {
            selectModule(module.get());
            draggedModule = module.get();
            dragOffset = canvasPos - module->getPosition();
            currentMode = InteractionMode::DRAGGING_MODULE;
            return;
        }
    }
    
    // Clicked on empty space
    deselectAll();
    currentMode = InteractionMode::PANNING_CANVAS;
}

void GUI2::handleMouseRelease(const sf::Event& event) {
    const auto* mouseRelease = event.getIf<sf::Event::MouseButtonReleased>();
    if (!mouseRelease || mouseRelease->button != sf::Mouse::Button::Left) return;
    
    sf::Vector2f mousePos(static_cast<float>(mouseRelease->position.x), 
                         static_cast<float>(mouseRelease->position.y));
    
    if (currentMode == InteractionMode::DRAGGING_CONNECTION) {
        finishConnectionDrag(mousePos);
    }
    
    currentMode = InteractionMode::IDLE;
    draggedModule = nullptr;
}

void GUI2::handleMouseMove(const sf::Event& event) {
    const auto* mouseMove = event.getIf<sf::Event::MouseMoved>();
    if (!mouseMove) return;
    
    sf::Vector2f mousePos(static_cast<float>(mouseMove->position.x), 
                         static_cast<float>(mouseMove->position.y));
    
    // Update hover state for module list
    if (moduleListVisible && moduleListBounds.contains(mousePos)) {
        float yOffset = moduleListBounds.position.y + 40;
        hoveredModuleType = -1;
        for (size_t i = 0; i < availableModuleTypes.size(); i++) {
            sf::FloatRect itemRect(sf::Vector2f(moduleListBounds.position.x + 10, yOffset), 
                                  sf::Vector2f(moduleListBounds.size.x - 20, 30));
            if (itemRect.contains(mousePos)) {
                hoveredModuleType = i;
                break;
            }
            yOffset += 35;
        }
    } else {
        hoveredModuleType = -1;
    }
    
    // Handle dragging
    if (currentMode == InteractionMode::DRAGGING_MODULE && draggedModule) {
        sf::Vector2f canvasPos = screenToCanvas(mousePos);
        draggedModule->setPosition(canvasPos - dragOffset);
    } else if (currentMode == InteractionMode::DRAGGING_CONNECTION) {
        updateConnectionDrag(mousePos);
    } else if (currentMode == InteractionMode::PANNING_CANVAS) {
        // Simple panning (would need to track previous mouse position)
    }
}

void GUI2::handleMouseWheel(const sf::Event& event) {
    const auto* wheelScroll = event.getIf<sf::Event::MouseWheelScrolled>();
    if (!wheelScroll) return;
    
    // Mouse wheel functionality disabled
    // (Previously used for zoom - removed per user request)
}

void GUI2::handleKeyPress(const sf::Event& event) {
    const auto* keyPress = event.getIf<sf::Event::KeyPressed>();
    if (!keyPress) return;
    
    if (keyPress->code == sf::Keyboard::Key::Delete || keyPress->code == sf::Keyboard::Key::Backspace) {
        if (selectedModule) {
            removeModule(selectedModule);
        }
    } else if (keyPress->code == sf::Keyboard::Key::M) {
        toggleModuleList();
    } else if (keyPress->code == sf::Keyboard::Key::G) {
        showGrid = !showGrid;
    } else if (keyPress->code == sf::Keyboard::Key::Enter && selectedModule) {
        openParameterWindow(selectedModule);
    }
}

ModularComponent* GUI2::addModule(const std::string& moduleType, const sf::Vector2f& position) {
    std::unique_ptr<ModularComponent> newModule;
    
    // Use MockModule for demo - replace with actual modules when ready
    if (moduleType == "AudioManager") {
        newModule = std::make_unique<MockModule>("Audio Manager", "AudioManager", sf::Color(150, 100, 200));
    } else if (moduleType == "Recorder") {
        newModule = std::make_unique<MockModule>("Recorder", "Recorder", sf::Color(200, 100, 100));
    } else if (moduleType == "NeuronNetwork") {
        newModule = std::make_unique<MockModule>("Neuron Network", "NeuronNetwork", sf::Color(100, 200, 150));
    } else if (moduleType == "BeatTracker") {
        newModule = std::make_unique<MockModule>("Beat Tracker", "BeatTracker", sf::Color(200, 150, 100));
    } else if (moduleType == "Rhythmogram") {
        newModule = std::make_unique<MockModule>("Rhythmogram", "Rhythmogram", sf::Color(150, 200, 150));
    } else if (moduleType == "Quantizer") {
        newModule = std::make_unique<MockModule>("Quantizer", "Quantizer", sf::Color(100, 150, 200));
    }
    
    if (newModule) {
        newModule->setPosition(position);
        newModule->initialize();
        ModularComponent* ptr = newModule.get();
        modules.push_back(std::move(newModule));
        return ptr;
    }
    
    return nullptr;
}

void GUI2::removeModule(ModularComponent* module) {
    if (!module) return;
    
    // Remove connections
    removeConnectionsForModule(module);
    
    // Remove module
    modules.erase(
        std::remove_if(modules.begin(), modules.end(),
            [module](const std::unique_ptr<ModularComponent>& m) {
                return m.get() == module;
            }),
        modules.end()
    );
    
    if (selectedModule == module) {
        selectedModule = nullptr;
    }
    
    if (parameterWindowModule == module) {
        closeParameterWindow();
    }
}

ModularComponent* GUI2::getModuleAt(const sf::Vector2f& position) {
    sf::Vector2f canvasPos = screenToCanvas(position);
    
    for (auto it = modules.rbegin(); it != modules.rend(); ++it) {
        if ((*it)->contains(canvasPos)) {
            return it->get();
        }
    }
    
    return nullptr;
}

void GUI2::selectModule(ModularComponent* module) {
    deselectAll();
    if (module) {
        module->setSelected(true);
        selectedModule = module;
    }
}

void GUI2::deselectAll() {
    for (auto& module : modules) {
        if (module) module->setSelected(false);
    }
    selectedModule = nullptr;
}

bool GUI2::createConnection(ModularComponent* sourceModule, int sourcePort,
                           ModularComponent* destModule, int destPort) {
    if (!sourceModule || !destModule) return false;
    
    // Check if connection already exists
    for (const auto& conn : connections) {
        if (conn.sourceModule == sourceModule && conn.sourcePortIndex == sourcePort &&
            conn.destModule == destModule && conn.destPortIndex == destPort) {
            return false; // Connection already exists
        }
    }
    
    connections.emplace_back(sourceModule, sourcePort, destModule, destPort);
    return true;
}

void GUI2::removeConnection(const VisualConnection& connection) {
    connections.erase(
        std::remove_if(connections.begin(), connections.end(),
            [&connection](const VisualConnection& c) {
                return c.sourceModule == connection.sourceModule &&
                       c.sourcePortIndex == connection.sourcePortIndex &&
                       c.destModule == connection.destModule &&
                       c.destPortIndex == connection.destPortIndex;
            }),
        connections.end()
    );
}

void GUI2::removeConnectionsForModule(ModularComponent* module) {
    connections.erase(
        std::remove_if(connections.begin(), connections.end(),
            [module](const VisualConnection& c) {
                return c.sourceModule == module || c.destModule == module;
            }),
        connections.end()
    );
}

bool GUI2::isValidConnection(ModularComponent* sourceModule, int sourcePort, bool sourceIsInput,
                            ModularComponent* destModule, int destPort, bool destIsInput) {
    if (!sourceModule || !destModule) return false;
    if (sourceModule == destModule) return false;
    if (sourceIsInput == destIsInput) return false; // Must connect input to output
    
    return true;
}

void GUI2::startConnectionDrag(ModularComponent* module, int portIndex, bool isInput) {
    connectionSourceModule = module;
    connectionSourcePort = portIndex;
    connectionSourceIsInput = isInput;
    currentMode = InteractionMode::DRAGGING_CONNECTION;
}

void GUI2::updateConnectionDrag(const sf::Vector2f& mousePos) {
    connectionDragPoint = mousePos;
}

void GUI2::finishConnectionDrag(const sf::Vector2f& mousePos) {
    sf::Vector2f canvasPos = screenToCanvas(mousePos);
    
    // Find module and port at this position
    for (auto& module : modules) {
        if (!module) continue;
        
        bool isInput = false;
        int portIndex = module->hitTestPort(canvasPos, isInput);
        
        if (portIndex >= 0) {
            // Check if this is a valid connection
            if (isValidConnection(connectionSourceModule, connectionSourcePort, 
                                connectionSourceIsInput, module.get(), portIndex, isInput)) {
                
                // Determine which is source and which is dest
                if (connectionSourceIsInput) {
                    // Dragged from input, so module is source
                    createConnection(module.get(), portIndex, connectionSourceModule, connectionSourcePort);
                } else {
                    // Dragged from output, so connectionSourceModule is source
                    createConnection(connectionSourceModule, connectionSourcePort, module.get(), portIndex);
                }
            }
            break;
        }
    }
    
    connectionSourceModule = nullptr;
    connectionSourcePort = -1;
}

sf::Vector2f GUI2::screenToCanvas(const sf::Vector2f& screenPos) const {
    return (screenPos - canvasOffset) / canvasZoom;
}

sf::Vector2f GUI2::canvasToScreen(const sf::Vector2f& canvasPos) const {
    return canvasPos * canvasZoom + canvasOffset;
}

void GUI2::openParameterWindow(ModularComponent* module) {
    parameterWindowModule = module;
    parameterWindowVisible = true;
}

void GUI2::closeParameterWindow() {
    parameterWindowModule = nullptr;
    parameterWindowVisible = false;
}

void GUI2::toggleModuleList() {
    moduleListVisible = !moduleListVisible;
}

void GUI2::saveCanvas(const std::string& filename) {
    // TODO: Implement JSON serialization
}

void GUI2::loadCanvas(const std::string& filename) {
    // TODO: Implement JSON deserialization
}
