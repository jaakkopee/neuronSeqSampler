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
#include "PresetManager.h"
#ifdef USE_TGUI
#include "GUI.h"
#endif
#include "RhythmInterpreter.h" // Ensure this is included after its dependencies
#include "SimpleSpectralDisplay.h"

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
                } else if constexpr (std::is_same_v<T, sf::Event::KeyPressed>) {
                    if (!eventConsumedByGUI) {
                        handleKeyPress(e.code);
                    }
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
    SimpleSpectralDisplay spectralDisplay;
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
        , spectralDisplay(network.getRhythmInterpreter())  // Initialize with rhythm interpreter
#ifdef USE_TGUI
        , guiManager(&gui, &window, &network, &visualizer, &recorder, &audioManager, &spectralDisplay, &activationInterval)
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
        
        // CRITICAL: Connect AudioManager to RhythmInterpreter for audio analysis
        audioManager.setRhythmInterpreter(network.getRhythmInterpreter());
        
        // Enable adaptive filter mode by default for high-frequency filter processing
        audioManager.setAdaptiveFilterMode(true);
        ESSENTIAL_PRINT("🎛️ Adaptive Filter Mode ENABLED by default for high-frequency processing");
        
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

        // Position spectral display (bottom area)
        spectralDisplay.setPosition(50.0f, 600.0f);
        spectralDisplay.setSize(600.0f, 150.0f);
        // Update rhythm interpreter reference after network initialization
        spectralDisplay.setRhythmInterpreter(network.getRhythmInterpreter());
        
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
            
            // Connection matrix setup removed: minimal RhythmInterpreter does not support setConnectionWeight
            DEBUG_PRINT("- Minimal RhythmInterpreter initialized");
            
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

    void handleKeyPress(sf::Keyboard::Key key) {
        switch (key) {
            case sf::Keyboard::Key::M:
                // Toggle rhythmogram matrix visibility
#ifdef USE_TGUI
                guiManager.toggleMatrixVisibility();
                ESSENTIAL_PRINT("🎛️ Toggled rhythmogram matrix visibility");
#else
                ESSENTIAL_PRINT("🎛️ Matrix toggle requires GUI support (TGUI not available)");
#endif
                break;
            case sf::Keyboard::Key::F:
                // Toggle filtered audio output
                {
                    bool currentMode = audioManager.isFilterModeEnabled();
                    audioManager.setAdaptiveFilterMode(!currentMode);
                    ESSENTIAL_PRINT("🎚️ Toggled filtered audio output: " << (!currentMode ? "ON" : "OFF"));
                }
                break;
            case sf::Keyboard::Key::Space:
                // Manual network activation
                if (network.getNeuronCount() > 0) {
                    network.activate();
                    ESSENTIAL_PRINT("🎵 Manual network activation");
                } else {
                    ESSENTIAL_PRINT("🎵 No neurons in network to activate");
                }
                break;
            case sf::Keyboard::Key::Num1:
            case sf::Keyboard::Key::Num2:
            case sf::Keyboard::Key::Num3:
            case sf::Keyboard::Key::Num4:
            case sf::Keyboard::Key::Num5:
            case sf::Keyboard::Key::Num6:
            case sf::Keyboard::Key::Num7:
            case sf::Keyboard::Key::Num8:
            case sf::Keyboard::Key::Num9:
                // Play samples with number keys
                {
                    int sampleIndex = static_cast<int>(key) - static_cast<int>(sf::Keyboard::Key::Num1);
                    audioManager.playSample(sampleIndex);
                    ESSENTIAL_PRINT("🎵 Playing sample " << (sampleIndex + 1));
                }
                break;
            case sf::Keyboard::Key::S:
                // Save preset (Ctrl+S would be better, but this is simpler for now)
                {
                    PresetManager::PresetInfo info;
                    info.name = "Quick Save";
                    info.description = "Quickly saved preset";
                    std::string filename = "presets/user/quicksave_" + std::to_string(std::time(nullptr)) + ".json";
                    if (PresetManager::savePreset(network, filename, info)) {
                        ESSENTIAL_PRINT("💾 Preset saved: " << filename);
                    } else {
                        ESSENTIAL_PRINT("❌ Failed to save preset");
                    }
                }
                break;
            case sf::Keyboard::Key::L:
                // Notify spectral display before loading (which calls clearNetwork)
                spectralDisplay.setRhythmInterpreter(nullptr);
                
                // Load factory drum pattern preset
                if (PresetManager::loadFactoryPreset(network, "drum_pattern")) {
                    ESSENTIAL_PRINT("📂 Loaded factory drum pattern preset");
                    
                    // Update AudioManager with new rhythm interpreter
                    if (network.getRhythmInterpreter()) {
                        audioManager.setRhythmInterpreter(network.getRhythmInterpreter());
                        ESSENTIAL_PRINT("🔄 AudioManager updated after preset load");
                    }
                    
                    // Update spectral display with new rhythm interpreter
                    spectralDisplay.setRhythmInterpreter(network.getRhythmInterpreter());
                    
                    // Refresh visualizer to show the loaded network
                    visualizer.refreshLayout();
#ifdef USE_TGUI
                    // Refresh GUI to show the loaded network
                    guiManager.refreshConnectionSliders();
                    guiManager.refreshNeuronSliders();
                    guiManager.refreshConnectionMatrix();
#endif
                } else {
                    ESSENTIAL_PRINT("❌ Failed to load factory preset");
                }
                break;
            case sf::Keyboard::Key::Q:
                // Toggle quantizer visibility
#ifdef USE_TGUI
                guiManager.toggleQuantizerVisibility();
                ESSENTIAL_PRINT("🎵 Toggled quantizer panel visibility");
#else
                ESSENTIAL_PRINT("🎵 Quantizer toggle requires GUI support (TGUI not available)");
#endif
                break;
            default:
                // Forward key to spectral display for its controls
                spectralDisplay.handleKeyPress(key);
                break;
        }
    }

    void update() {
    // Automatic network activation based on activation interval
    float elapsedMs = clock.getElapsedTime().asMilliseconds();
    if (elapsedMs >= activationInterval) {
        network.activate();
        clock.restart();
    }
    
    // Update spectral display
    spectralDisplay.update();
    
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
    
    // Render spectral display
    spectralDisplay.render(window);
    
#ifdef USE_TGUI
    // Render GUI
    gui.draw();
#endif
    window.display();
    }

};

int main() {
    // Initialize preset system
    PresetManager::createPresetDirectory();
    
    std::cout << "🎵 Neuron Sequence Sampler" << std::endl;
    std::cout << "💾 Preset Controls:" << std::endl;
    std::cout << "   S - Save current network as preset" << std::endl;
    std::cout << "   L - Load factory drum pattern preset" << std::endl;
    std::cout << std::endl;
    
    NeuronSeqSampler app;
    app.run();
    return 0;
}
