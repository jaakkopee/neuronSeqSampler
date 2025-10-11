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
#include "RhythmInterpreter.h" // Ensure this is included after its dependencies

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
    bool testingMode;
    
    // Filter mode state
    bool audioStreamingEnabled;

public:
    NeuronSeqSampler(bool enableTestingMode = false) 
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
        
        std::cout << "\nAll changes to code by GitHub Copilot. The prompts were either feature additions or bug fixes for most cases.\n" << std::endl;
        
        std::cout << "Neuron Sequence Sampler initialized with empty network." << std::endl;
        std::cout << "Use the 'Network' menu to add neurons and connections." << std::endl;
        
        std::cout << "Controls:" << std::endl;
        std::cout << "  - Mouse: Click to activate neurons" << std::endl;
        std::cout << "  - Number keys: Activate specific neurons (when available)" << std::endl;
        std::cout << "  - Spacebar: Manual network activation" << std::endl;
        std::cout << "  - F key: Toggle filter mode (routes samples through filter bank) �️" << std::endl;
        std::cout << "  - L buttons: Solo individual filter bands 🎚️" << std::endl;
        std::cout << "  - Number keys: Play samples (1-9)" << std::endl;
        std::cout << "\n🔴 IMPORTANT: Press 'F' to enable filter mode, then use number keys to play samples!" << std::endl;
        std::cout << "  - GUI sliders: Adjust connection weights" << std::endl;
        std::cout << "  - Menu: Add/remove neurons and connections" << std::endl;
    }
    
    void setupTestingNetwork() {
        std::cout << "Setting up testing network with 3 fully connected neurons..." << std::endl;
        
        // Load samples for testing
        bool kickLoaded = audioManager.loadSampleFromPath(1, "samples/kick/kick (ghost).wav");
        bool clapLoaded = audioManager.loadSampleFromPath(2, "samples/clap/clap (ghost).wav");
        bool bassLoaded = audioManager.loadSampleFromPath(3, "samples/808/ROBBERY 808 @prodopus.wav");
        
        if (!kickLoaded) {
            std::cout << "Warning: Could not load kick sample" << std::endl;
        }
        if (!clapLoaded) {
            std::cout << "Warning: Could not load clap sample" << std::endl;
        }
        if (!bassLoaded) {
            std::cout << "Warning: Could not load 808 sample" << std::endl;
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
            
            std::cout << "Testing network created successfully!" << std::endl;
            std::cout << "- Kick neuron (sample 1): " << (kickLoaded ? "✓" : "✗") << std::endl;
            std::cout << "- Clap neuron (sample 2): " << (clapLoaded ? "✓" : "✗") << std::endl;
            std::cout << "- 808 neuron (sample 3): " << (bassLoaded ? "✓" : "✗") << std::endl;
            std::cout << "- 6 connections created (fully connected)" << std::endl;
            
#ifdef USE_TGUI
            // Refresh GUI to show the new network
            guiManager.refreshNeuronSliders();
            guiManager.refreshConnectionSliders();
            guiManager.refreshConnectionMatrix();
#endif
            // Refresh visualizer layout
            visualizer.refreshLayout();
            
        } else {
            std::cout << "Error: Failed to create neurons for testing network" << std::endl;
        }
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
            // Handle GUI events - check if event was consumed by GUI
            bool eventConsumedByGUI = gui.handleEvent(event);
            
            // Only handle mouse clicks if GUI didn't consume the event
            if (!eventConsumedByGUI && event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    handleMouseDrag(event.mouseButton.x, event.mouseButton.y);
                }
            }
#else
            // Handle mouse clicks on neurons (for manual activation) - no GUI
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    handleMouseDrag(event.mouseButton.x, event.mouseButton.y);
                }
            }
#endif

            // Handle mouse scroll for zooming
            if (event.type == sf::Event::MouseWheelScrolled) {
                handleMouseScroll(event.mouseWheelScroll.delta);
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
                else if (event.key.code == sf::Keyboard::M) {
                    // Toggle connection matrix visibility with 'M' key
                    guiManager.toggleMatrixVisibility();
                }
                else if (event.key.code == sf::Keyboard::F) {
                    // Toggle filtered audio output with 'F' key
                    audioStreamingEnabled = !audioStreamingEnabled;
                    if (audioStreamingEnabled && network.getRhythmInterpreter()) {
                        // Set up filter callback to route samples through filter bank
                        auto rhythmInterpreter = network.getRhythmInterpreter();
                        audioManager.setFilterCallback([rhythmInterpreter](const std::vector<float>& audioData) -> std::vector<float> {
                            // Process audio through rhythm interpreter and get filtered output
                            std::cout << "🔥  F-KEY FILTERED CALLBACK executing - returning filtered audio!" << std::endl;
                            rhythmInterpreter->processAudioFrame(audioData);
                            auto filtered = rhythmInterpreter->getProcessedAudioOutput();
                            std::cout << "🔥  F-KEY returning " << filtered.size() << " filtered samples" << std::endl;
                            return filtered;
                        });
                        std::cout << "🔥  F-KEY Filtered audio output ENABLED - samples route through filter bank" << std::endl;
                    } else {
                        // Disable filter callback for direct playback
                        audioManager.setFilterCallback(nullptr);
                        std::cout << "Filtered audio output DISABLED - direct sample playback" << std::endl;
                    }
                }
            }
        }
    }
    /*
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
        
        if (++statusCounter % 300 == 0) { // Every ~10 seconds at 60fps
            std::cout << "📊 Status - RhythmInterpreter: " << (hasRhythmInterpreter ? "READY" : "NOT READY")
                      << ", FilterMode: " << (isFilterModeEnabled ? "ON" : "OFF") << std::endl;
        }
        
        // Automatic network activation at intervals
        if (clock.getElapsedTime().asMilliseconds() >= activationInterval) {
            network.activate();
            clock.restart();
        }

#ifdef USE_TGUI
        // Update GUI
        guiManager.update();
#endif
    }    void render() {
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

int main(int argc, char* argv[]) {
    try {
        std::cout << "Starting Neuron Sequence Sampler..." << std::endl;
        
        // Parse command line arguments
        bool testingMode = false;
        for (int i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == "--testing") {
                testingMode = true;
                std::cout << "Testing mode enabled" << std::endl;
                break;
            }
        }
        
        NeuronSeqSampler app(testingMode);
        app.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}