#include <SFML/Graphics.hpp>
#include "GUI2.h"
#include <iostream>

/**
 * @brief Simplified demo application for the modular GUI2 system
 * 
 * This version works with SFML 3.x and doesn't require the full
 * implementation of all modules - it demonstrates the GUI structure.
 */
int main() {
    // Create window (SFML 3 syntax)
    sf::RenderWindow window(sf::VideoMode({1600, 900}), "neuronSeqSampler - Modular GUI");
    window.setFramerateLimit(60);
    
    // Create GUI2 system
    GUI2 gui(&window);
    gui.initialize();
    
    std::cout << "=== neuronSeqSampler Modular GUI2 ===" << std::endl;
    std::cout << "\nNOTE: This is a GUI demonstration. Full module implementation pending." << std::endl;
    std::cout << "\nControls:" << std::endl;
    std::cout << "  - Click module list items to add modules" << std::endl;
    std::cout << "  - Drag modules to move them" << std::endl;
    std::cout << "  - Drag from port to port to create connections" << std::endl;
    std::cout << "  - Click module + Enter to open parameters" << std::endl;
    std::cout << "  - M: Toggle module list" << std::endl;
    std::cout << "  - G: Toggle grid" << std::endl;
    std::cout << "  - Delete/Backspace: Remove selected module" << std::endl;
    std::cout << "  - Mouse wheel: Zoom" << std::endl;
    std::cout << std::endl;
    
    // Main loop
    sf::Clock clock;
    
    while (window.isOpen()) {
        // Handle events (SFML 3 syntax)
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (event->is<sf::Event::MouseButtonPressed>()) {
                gui.handleMousePress(*event);
            } else if (event->is<sf::Event::MouseButtonReleased>()) {
                gui.handleMouseRelease(*event);
            } else if (event->is<sf::Event::MouseMoved>()) {
                gui.handleMouseMove(*event);
            } else if (event->is<sf::Event::MouseWheelScrolled>()) {
                gui.handleMouseWheel(*event);
            } else if (event->is<sf::Event::KeyPressed>()) {
                gui.handleKeyPress(*event);
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
