#include <SFML/Graphics.hpp>
#include <variant>
#include <optional>
#include <optional>
#include <iostream>

#ifdef USE_TGUI
#include <TGUI/Backend/SFML-Graphics.hpp>
#endif

#include "NeuronNetwork.h"
#include "AudioManager.h"
#include "Visualizer.h"
#include "Debug.h"
#include "Recorder.h"
#include "StartupAnimation.h"
#ifdef USE_TGUI
#include "GUI.h"
#endif
#include "RhythmInterpreter.h" // Ensure this is included after its dependencies

// Global debug flag definition
bool g_debugMode = false;

class NeuronSeqSampler {
private:
    void handleEvents() {
        while (auto eventOpt = window.pollEvent()) {
            if (!eventOpt) break;
            const auto& event = *eventOpt;
#ifdef USE_TGUI
            bool eventConsumedByGUI = gui.handleEvent(event);
#else
            bool eventConsumedByGUI = false;
#endif
            event.visit([&](const auto& e) {
                using T = std::decay_t<decltype(e)>;
                if constexpr (std::is_same_v<T, sf::Event::MouseButtonPressed>) {
                    if (!eventConsumedByGUI && e.button == sf::Mouse::Button::Left) {
                        handleMouseDrag(e.position.x, e.position.y);
                    }
                } else if constexpr (std::is_same_v<T, sf::Event::Closed>) {
                    window.close();
                } else if constexpr (std::is_same_v<T, sf::Event::MouseWheelScrolled>) {
                    handleMouseScroll(e.delta);
                } else {
                    // Ignore other events
                }
            });
        }
    }
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
    bool testingMode;
    
    // Filter mode state
    bool audioStreamingEnabled;

public:
    NeuronSeqSampler(bool enableTestingMode = false) 
        : window(sf::VideoMode({1024, 800}), "Neuron Sequence Sampler")
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
        , testingMode(enableTestingMode)
        , audioStreamingEnabled(false)
    {
        initialize();
        if (testingMode) {
            setupTestingNetwork();
        }
    }
    
    void initialize() {
        // Set up the network with audio manager
        network.setAudioManager(&audioManager);
        
        // Set up internal recording connection
        audioManager.setInternalRecorder(&recorder);
        
        // Start with an empty network - users can add neurons via the menu
        
        // Set up visualizer canvas area (left side of window)
        // the visualiser will draw the neurons and connections
        visualizer.setCanvasArea(50.0f, 50.0f, 700.0f, 700.0f);
        visualizer.setNeuronRadius(20.0f);
        visualizer.setNeuronColors(sf::Color::Cyan, sf::Color::Red);
        visualizer.setConnectionColors(sf::Color(200, 200, 200, 100), sf::Color::Yellow);
        
#ifdef USE_TGUI
        // Initialize GUI (right side of window)
        guiManager.initialize();
        // Set GUI area to the right side of the window with dimensions 324x800
        guiManager.setGUIArea(700.0f, 0.0f, 1024.0f, 800.0f);
#endif
        
        DEBUG_PRINT("\nAll changes to code by GitHub Copilot. The prompts were either feature additions or bug fixes for most cases.");
        
        ESSENTIAL_PRINT("Neuron Sequence Sampler initialized with empty network.");
        ESSENTIAL_PRINT("Use the 'Network' menu to add neurons and connections.");
        
        ESSENTIAL_PRINT("Controls:");
        ESSENTIAL_PRINT("  - Mouse: Click to activate neurons");
        ESSENTIAL_PRINT("  - Number keys: Activate specific neurons (when available)");
        ESSENTIAL_PRINT("  - Spacebar: Manual network activation");
        ESSENTIAL_PRINT("  - F key: Toggle filtered audio output (hearing filtered vs original)");
        ESSENTIAL_PRINT("  - M key: Toggle rhythmogram matrix visibility 🎛️");
        ESSENTIAL_PRINT("  - L buttons: Solo individual filter bands 🎚️");
        ESSENTIAL_PRINT("  - Number keys: Play samples (1-9)");
        ESSENTIAL_PRINT("\n🎛️ Rhythmogram analysis is always active - use gain sliders to control mapping!");
        ESSENTIAL_PRINT("  - GUI sliders: Adjust filter gains and connection weights");
        ESSENTIAL_PRINT("  - Menu: Add/remove neurons and connections");
    }
    
    void setupTestingNetwork() {
        ESSENTIAL_PRINT("Setting up testing network with 3 fully connected neurons...");
        
        // Load samples for testing
        bool kickLoaded = audioManager.loadSampleFromPath(1, "samples/kick/kick (ghost).wav");
        bool clapLoaded = audioManager.loadSampleFromPath(2, "samples/clap/clap (ghost).wav");
        bool bassLoaded = audioManager.loadSampleFromPath(3, "samples/808/ROBBERY 808 @prodopus.wav");
        
        if (!kickLoaded) {
            ESSENTIAL_PRINT("Warning: Could not load kick sample");
        }
        if (!clapLoaded) {
            ESSENTIAL_PRINT("Warning: Could not load clap sample");
        }
        if (!bassLoaded) {
            ESSENTIAL_PRINT("Warning: Could not load 808 sample");
        }
        
        // Create three neurons
        Neuron* kickNeuron = network.addNeuron(1, 0.0f, 1.0f, 0.5f, 0.0f); // Sample 1
        Neuron* clapNeuron = network.addNeuron(2, 0.0f, 1.0f, 0.5f, 0.0f); // Sample 2
        Neuron* bassNeuron = network.addNeuron(3, 0.0f, 1.0f, 0.5f, 0.0f); // Sample 3

        if (kickNeuron && clapNeuron && bassNeuron) {
            // Create fully connected network (each neuron connected to every other)
            network.connect(kickNeuron, clapNeuron, 0.6f);
            network.connect(kickNeuron, bassNeuron, 0.7f);
            network.connect(clapNeuron, kickNeuron, 0.5f);
            network.connect(clapNeuron, bassNeuron, 0.8f);
            network.connect(bassNeuron, kickNeuron, 0.4f);
            network.connect(bassNeuron, clapNeuron, 0.6f);
            
            ESSENTIAL_PRINT("Testing network created successfully!");
            ESSENTIAL_PRINT_STREAM("- Kick neuron (sample 1): " << (kickLoaded ? "✓" : "✗"));
            ESSENTIAL_PRINT_STREAM("- Clap neuron (sample 2): " << (clapLoaded ? "✓" : "✗"));
            ESSENTIAL_PRINT_STREAM("- 808 neuron (sample 3): " << (bassLoaded ? "✓" : "✗"));
            DEBUG_PRINT("- 6 connections created (fully connected)");
            
            // Initialize rhythmogram connection matrix with some default connections
            auto rhythmInterpreter = network.getRhythmInterpreter();
            if (rhythmInterpreter) {
                // Set up some meaningful rhythmogram connections for testing
                // Connect kick to lower frequencies (quarter note, eighth note)
                rhythmInterpreter->setConnectionWeight(3, 0, 0.4f); // Quarter (1Hz) → Kick
                rhythmInterpreter->setConnectionWeight(4, 0, 0.3f); // Eighth (2Hz) → Kick
                
                // Connect clap to mid frequencies (eighth, sixteenth)
                rhythmInterpreter->setConnectionWeight(4, 1, 0.5f); // Eighth (2Hz) → Clap  
                rhythmInterpreter->setConnectionWeight(5, 1, 0.4f); // 16th (4Hz) → Clap
                
                // Connect 808 to lower frequencies (whole, half, quarter)
                rhythmInterpreter->setConnectionWeight(1, 2, 0.6f); // Whole (0.25Hz) → 808
                rhythmInterpreter->setConnectionWeight(2, 2, 0.5f); // Half (0.5Hz) → 808
                rhythmInterpreter->setConnectionWeight(3, 2, 0.3f); // Quarter (1Hz) → 808
                
                DEBUG_PRINT("- Rhythmogram connection matrix initialized");
            }
            
#ifdef USE_TGUI
            // Refresh GUI to show the new network
            guiManager.refreshNeuronSliders();
            guiManager.refreshConnectionSliders();
            guiManager.refreshConnectionMatrix();
#endif
            // Refresh visualizer layout
            visualizer.refreshLayout();
            
        } else {
            ESSENTIAL_PRINT("Error: Failed to create neurons for testing network");
        }
    }
    
    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }
    /*
    void handleNeuronClick(int mouseX, int mouseY) {
        // Click detection in the visualization area
        if (mouseX >= 50 && mouseX <= 750 && mouseY >= 50 && mouseY <= 750) {
            if (network.getNeuronCount() > 0) {
                // For now, activate the first neuron - could be enhanced to detect specific neurons
                    sf::Event event;
                    while (window.pollEvent(event)) {
                        bool eventConsumedByGUI = gui.handleEvent(event);
                        if (event.type == sf::Event::Closed) {
                            window.close();
                        }
            }
        }
    }
    */

    void handleMouseScroll(int delta) {
        //zoom with mouse scroll
        visualizer.handleMouseScroll(delta);
    }

    void handleMouseDrag(int mouseX, int mouseY) {
        //pan and zoom with mouse
        visualizer.handleMouseDrag(mouseX, mouseY);
    }

    void update() {
    // Status monitoring
    static int statusCounter = 0;
    bool hasRhythmInterpreter = network.getRhythmInterpreter() != nullptr;
    bool isFilterModeEnabled = audioManager.isFilterModeEnabled();
#ifdef USE_TGUI
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
    NeuronSeqSampler app;
    app.run();
    return 0;
}
