#include <SFML/Graphics.hpp>
#include "GUI2.h"
#include <iostream>

/**
 * @brief Demo application for the modular GUI2 system
 * 
 * Controls:
 * - Left-click on module list to add modules to canvas
 * - Left-click and drag modules to move them
 * - Left-click on a port and drag to another port to create connections
 * - Left-click on a module and press Enter to open parameter window
 * - Press M to toggle module list
 * - Press G to toggle grid
 * - Press Delete/Backspace to delete selected module
 * - Mouse wheel to zoom in/out
 */
int main() {
    // Create window
    sf::RenderWindow window(sf::VideoMode(1600, 900), "neuronSeqSampler - Modular GUI");
    window.setFramerateLimit(60);
    
    // Create GUI2 system
    GUI2 gui(&window);
    gui.initialize();
    
    std::cout << "=== neuronSeqSampler Modular GUI2 ===" << std::endl;
    std::cout << "\nControls:" << std::endl;
    std::cout << "  - Click module list items to add modules" << std::endl;
    std::cout << "  - Drag modules to move them" << std::endl;
    std::cout << "  - Drag from port to port to create connections" << std::endl;
    std::cout << "  - Click module + Enter to open parameters" << std::endl;
    std::cout << "  - M: Toggle module list" << std::endl;
    std::cout << "  - G: Toggle grid" << std::endl;
    std::cout << "  - Delete/Backspace: Remove selected module" << std::endl;
    std::cout << "  - Mouse wheel: Zoom" << std::endl;
    std::cout << "\nAvailable Modules:" << std::endl;
    std::cout << "  - AudioManager: Audio playback and management" << std::endl;
    std::cout << "  - Recorder: Audio recording with filters" << std::endl;
    std::cout << "  - NeuronNetwork: Spiking neural network" << std::endl;
    std::cout << "  - BeatTracker: Agent-based beat tracking" << std::endl;
    std::cout << "  - Rhythmogram: Todd frequency band analysis" << std::endl;
    std::cout << "  - Quantizer: Musical timing quantization" << std::endl;
    std::cout << std::endl;
    
    // Main loop
    sf::Clock clock;
    
    while (window.isOpen()) {
        // Handle events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed) {
                gui.handleMousePress(event);
            } else if (event.type == sf::Event::MouseButtonReleased) {
                gui.handleMouseRelease(event);
            } else if (event.type == sf::Event::MouseMoved) {
                gui.handleMouseMove(event);
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                gui.handleMouseWheel(event);
            } else if (event.type == sf::Event::KeyPressed) {
                gui.handleKeyPress(event);
            }
        }
        
        // Update
        float deltaTime = clock.restart().asSeconds();
        gui.update(deltaTime);
        
        // Render
        window.clear();
        gui.render();
        window.display();
    }
    
    std::cout << "GUI2 closed successfully." << std::endl;
    return 0;
}
