#include <SFML/Graphics.hpp>
#include <iostream>

#ifdef USE_TGUI
#include <TGUI/Backend/SFML-Graphics.hpp>
#endif

#include "NeuronNetwork.h"
#include "AudioManager.h"
#include "Visualizer.h"
#include "Recorder.h"
#ifdef USE_TGUI
#include "GUI.h"
#endif

class NeuronSeqSampler {
private:
    sf::RenderWindow window;
#ifdef USE_TGUI
    tgui::Gui gui;
#endif
    
    AudioManager audioManager;
    NeuronNetwork network;
    Visualizer visualizer;
    Recorder recorder;
#ifdef USE_TGUI
    GUI guiManager;
#endif
    
    sf::Clock clock;
    float activationInterval;

public:
    NeuronSeqSampler() 
        : window(sf::VideoMode(1024, 800), "Neuron Sequence Sampler")
#ifdef USE_TGUI
        , gui(window)
#endif
        , audioManager("samples/girliepop/", true)  // Load default samples
        , network()
        , visualizer(&window, &network)
#ifdef USE_TGUI
        , guiManager(&gui, &window, &network, &visualizer, &recorder, &audioManager, &activationInterval)
#endif
        , activationInterval(100.0f) // milliseconds
    {
        initialize();
    }
    
    void initialize() {
        // Set up the network with audio manager
        network.setAudioManager(&audioManager);
        
        // Set up internal recording connection
        audioManager.setInternalRecorder(&recorder);
        
        // Start with an empty network - users can add neurons via the menu
        
        // Set up visualizer canvas area (left side of window)
        visualizer.setCanvasArea(50.0f, 50.0f, 700.0f, 700.0f);
        visualizer.setNeuronRadius(20.0f);
        visualizer.setNeuronColors(sf::Color::Cyan, sf::Color::Red);
        visualizer.setConnectionColors(sf::Color(200, 200, 200, 100), sf::Color::Yellow);
        
#ifdef USE_TGUI
        // Initialize GUI (right side of window)
        guiManager.initialize();
        guiManager.setGUIArea(400.0f, 0.0f, 624.0f, 800.0f);
#endif
        
        std::cout << "\nAll changes to code by GitHub Copilot. The prompts were either feature additions or bug fixes for most cases.\n" << std::endl;
        
        std::cout << "Neuron Sequence Sampler initialized with empty network." << std::endl;
        std::cout << "Use the 'Network' menu to add neurons and connections." << std::endl;
        
        std::cout << "Controls:" << std::endl;
        std::cout << "  - Mouse: Click to activate neurons" << std::endl;
        std::cout << "  - Number keys: Activate specific neurons (when available)" << std::endl;
        std::cout << "  - Spacebar: Manual network activation" << std::endl;
        std::cout << "  - R key: Toggle audio recording" << std::endl;
        std::cout << "  - GUI sliders: Adjust connection weights" << std::endl;
        std::cout << "  - Menu: Add/remove neurons and connections" << std::endl;
    }
    
    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }
    
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            
#ifdef USE_TGUI
            // Handle GUI events
            gui.handleEvent(event);
#endif
            
            // Handle mouse clicks on neurons (for manual activation)
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    handleNeuronClick(event.mouseButton.x, event.mouseButton.y);
                }
            }
            
            // Handle keyboard input
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    // Manual network activation
                    network.activate();
                    std::cout << "Manual network activation triggered" << std::endl;
                }
                else if (event.key.code == sf::Keyboard::R) {
                    // Toggle recording with 'R' key
                    if (recorder.isCurrentlyRecording()) {
                        recorder.stopRecording();
                        audioManager.stopInternalRecording();
                        std::cout << "Recording stopped" << std::endl;
                    } else {
                        if (event.key.shift) {
                            // Shift+R: External microphone recording
                            if (recorder.startRecording()) {
                                std::cout << "External recording started (press R again to stop)" << std::endl;
                            }
                        } else {
                            // R: Internal recording of NeuronSeqSampler output
                            if (recorder.startInternalRecording()) {
                                audioManager.startInternalRecording();
                                std::cout << "Internal recording started - capturing NeuronSeqSampler output (press R again to stop)" << std::endl;
                            }
                        }
                    }
                }
                else if (event.key.code >= sf::Keyboard::Num1 && 
                         event.key.code <= sf::Keyboard::Num9) {
                    // Direct neuron activation with number keys (1-9)
                    int neuronIndex = event.key.code - sf::Keyboard::Num1;
                    if (neuronIndex < static_cast<int>(network.getNeuronCount())) {
                        network.getNeuron(neuronIndex)->activate(0.5f);
                        std::cout << "Activated neuron " << (neuronIndex + 1) << std::endl;
                    } else if (network.getNeuronCount() == 0) {
                        std::cout << "No neurons in network. Use the Network menu to add neurons." << std::endl;
                    }
                }
            }
        }
    }
    
    void handleNeuronClick(int mouseX, int mouseY) {
        // Click detection in the visualization area
        if (mouseX >= 50 && mouseX <= 750 && mouseY >= 50 && mouseY <= 750) {
            if (network.getNeuronCount() > 0) {
                // For now, activate the first neuron - could be enhanced to detect specific neurons
                network.getNeuron(0)->activate(0.8f);
                std::cout << "Neuron activated by mouse click" << std::endl;
            } else {
                std::cout << "No neurons in network. Use the Network menu to add neurons." << std::endl;
            }
        }
    }
    
    void update() {
        // Automatic network activation at intervals
        if (clock.getElapsedTime().asMilliseconds() >= activationInterval) {
            network.activate();
            clock.restart();
        }
        
#ifdef USE_TGUI
        // Update GUI
        guiManager.update();
#endif
    }
    
    void render() {
        window.clear(sf::Color::Black);
        
        // Render the neural network visualization
        visualizer.render();
        
#ifdef USE_TGUI
        // Render GUI
        gui.draw();
#endif
        
        window.display();
    }
};

int main() {
    try {
        std::cout << "Starting Neuron Sequence Sampler..." << std::endl;
        
        NeuronSeqSampler app;
        app.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}