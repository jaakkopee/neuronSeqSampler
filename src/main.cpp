#include <SFML/Graphics.hpp>
#include <variant>
#include <optional>
#include <optional>
#include <iostream>
#include <map>

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

// Helper function to load preset sample files into AudioManager
void loadPresetSamplesIntoAudioManager(NeuronNetwork& network, AudioManager& audioManager) {
    const auto& neurons = network.getNeurons();
    ESSENTIAL_PRINT("🔄 Loading " << neurons.size() << " sample files into AudioManager...");
    
    // Map to track unique sample files and their assigned indices
    std::map<std::string, int> sampleFileToIndex;
    int nextAvailableIndex = 1; // Start from index 1
    
    for (size_t i = 0; i < neurons.size(); ++i) {
        const Neuron* neuron = neurons[i].get();
        std::string samplePath = neuron->getSampleFilePath();
        
        if (samplePath.empty()) {
            ESSENTIAL_PRINT("⚠️ Neuron " << i << " has no sample file path");
            continue;
        }
        
        int assignedIndex;
        
        // Check if we've already loaded this sample file
        auto it = sampleFileToIndex.find(samplePath);
        if (it != sampleFileToIndex.end()) {
            // Reuse existing index
            assignedIndex = it->second;
            ESSENTIAL_PRINT("🔄 Reusing sample " << assignedIndex << " for neuron " << i << ": " << samplePath);
        } else {
            // Load new sample file
            assignedIndex = nextAvailableIndex++;
            
            if (audioManager.loadSampleFromPath(assignedIndex, samplePath)) {
                sampleFileToIndex[samplePath] = assignedIndex;
                ESSENTIAL_PRINT("✅ Loaded sample " << assignedIndex << " for neuron " << i << ": " << samplePath);
            } else {
                ESSENTIAL_PRINT("❌ Failed to load sample for neuron " << i << ": " << samplePath);
                continue; // Skip updating neuron's sample index if loading failed
            }
        }
        
        // Update the neuron's sample index to point to the loaded sample
        // We need to cast away const to modify the neuron
        Neuron* mutableNeuron = const_cast<Neuron*>(neuron);
        mutableNeuron->setSampleIndex(assignedIndex);
    }
    
    ESSENTIAL_PRINT("🎵 Preset sample loading complete! Loaded " << sampleFileToIndex.size() << " unique samples.");
}

class NeuronSeqSampler {
private:
    void handleEvents() {
        while (auto eventOpt = window.pollEvent()) {
            if (!eventOpt) break;
            const auto& event = *eventOpt;
            
            bool eventConsumedByGUI = false;
            
#ifdef USE_TGUI
            // Always pass events to TGUI first for proper dialog/widget handling
            eventConsumedByGUI = gui.handleEvent(event);
            
            // WORKAROUND: TGUI sometimes doesn't consume TextEntered events even when EditBox has focus
            // Force consumption when text input is active to ensure EditBoxes work properly
            if (event.is<sf::Event::TextEntered>() && guiManager.isTextInputActive()) {
                eventConsumedByGUI = true;
            }
#endif
            
            // Debug GUI event consumption for mouse clicks
            if (event.is<sf::Event::MouseButtonPressed>() && eventConsumedByGUI) {
                if (const auto* e = event.getIf<sf::Event::MouseButtonPressed>()) {
                    std::cout << "⛔ GUI consumed mouse click at (" << e->position.x << ", " << e->position.y << ")" << std::endl;
                }
            }
            
            // Use explicit type checks and getIf for SFML Event (works for SFML 3)
            if (event.is<sf::Event::MouseButtonPressed>()) {
                if (!eventConsumedByGUI) {
                    if (const auto* e = event.getIf<sf::Event::MouseButtonPressed>()) {
                        if (e->button == sf::Mouse::Button::Left) {
                            // Check if click is in canvas area
                            bool isInCanvas = (e->position.x >= canvasLeft && e->position.x <= canvasRight && 
                                              e->position.y >= canvasTop && e->position.y <= canvasBottom);
                            
                            std::cout << "🖱️ Click at (" << e->position.x << ", " << e->position.y 
                                     << ") Canvas[" << canvasLeft << "," << canvasTop << " to " 
                                     << canvasRight << "," << canvasBottom << "] InCanvas: " 
                                     << (isInCanvas ? "YES" : "NO") << std::endl;
                            
                            if (isInCanvas) {
                                handleMouseDrag(e->position.x, e->position.y);
                            }
                        }
                    }
                }
            } else if (event.is<sf::Event::Closed>()) {
                window.close();
            } else if (event.is<sf::Event::MouseWheelScrolled>()) {
                if (const auto* e = event.getIf<sf::Event::MouseWheelScrolled>()) {
                    handleMouseScroll(e->delta);
                }
            } else if (event.is<sf::Event::MouseMoved>()) {
                if (!eventConsumedByGUI) {
                    if (const auto* e = event.getIf<sf::Event::MouseMoved>()) {
                        static int mouseEventCounter = 0;
                        if (++mouseEventCounter % 200 == 0) { // Print every 200th event to avoid spam
                            // Check if mouse is in canvas area for debugging
                            bool isInCanvas = (e->position.x >= canvasLeft && e->position.x <= canvasRight && 
                                              e->position.y >= canvasTop && e->position.y <= canvasBottom);
                            
                            std::cout << "🖱️ Mouse (" << e->position.x << ", " << e->position.y 
                                     << ") Canvas[" << canvasLeft << "," << canvasTop << " to " 
                                     << canvasRight << "," << canvasBottom << "] InCanvas: " 
                                     << (isInCanvas ? "YES" : "NO") << " " 
                                     << (isFullscreen ? "Fullscreen" : "Windowed") << std::endl;
                        }
                        visualizer.handleMouseMove(e->position.x, e->position.y);
                    }
                }
            } else if (event.is<sf::Event::KeyPressed>()) {
                if (const auto* e = event.getIf<sf::Event::KeyPressed>()) {
                    // Check if text input is currently active (EditBox has focus) or any dialog is open
#ifdef USE_TGUI
                    bool textInputActive = guiManager.isTextInputActive();
                    bool dialogOpen = guiManager.isDialogOpen();
                    
                    // Debug keyboard input
                    if (textInputActive || dialogOpen) {
                        std::cout << "⌨️ KeyPressed - TextInputActive: " << textInputActive 
                                  << " DialogOpen: " << dialogOpen 
                                  << " EventConsumed: " << eventConsumedByGUI << std::endl;
                    }
#else
                    bool textInputActive = false;
                    bool dialogOpen = false;
#endif
                    
                    // Forward only application-level global shortcuts even if GUI consumed the event.
                    // This avoids requiring a click on the visualization to use shortcuts while not
                    // hijacking normal text input which GUI will typically consume.
                    sf::Keyboard::Key code = e->code;
                    bool isGlobalShortcut = (code == sf::Keyboard::Key::M) || (code == sf::Keyboard::Key::Q) ||
                                            (code == sf::Keyboard::Key::Space) || (code == sf::Keyboard::Key::F) ||
                                            (code >= sf::Keyboard::Key::Num1 && code <= sf::Keyboard::Key::Num9) ||
                                            (code == sf::Keyboard::Key::S) || (code == sf::Keyboard::Key::L) ||
                                            (code == sf::Keyboard::Key::Equal) || (code == sf::Keyboard::Key::Add);

                    // Don't process keyboard shortcuts when text input is active or a dialog is open
                    if (!textInputActive && !dialogOpen && (!eventConsumedByGUI || isGlobalShortcut)) {
                        handleKeyPress(code);
                    }
                }
            } else if (event.is<sf::Event::TextEntered>()) {
                // TextEntered events are already handled by gui.handleEvent() above
                // This explicit check ensures they're not ignored in the final else clause
#ifdef USE_TGUI
                if (const auto* e = event.getIf<sf::Event::TextEntered>()) {
                    std::cout << "📝 TextEntered: unicode=" << e->unicode 
                              << " consumed=" << eventConsumedByGUI << std::endl;
                }
#endif
            } else {
                // Ignore other events
            }
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
    
    // Fullscreen state
    bool isFullscreen;
    sf::VideoMode windowedMode;
    sf::Vector2i windowedPosition;
    
    // Dynamic canvas bounds for mouse handling
    float canvasLeft, canvasTop, canvasRight, canvasBottom;

public:
    NeuronSeqSampler(bool enableTestingMode = false) 
        : window(sf::VideoMode({1280, 720}), "Neuron Sequence Sampler")
#ifdef USE_TGUI
        , gui(window)
#endif
        , audioManager("samples/girliepop/", true)  // Load default samples
        , network()
        , visualizer(&window, &network)
        , spectralDisplay(network.getRhythmInterpreter(), &network)  // Initialize with rhythm interpreter and network
#ifdef USE_TGUI
        , guiManager(&gui, &window, &network, &visualizer, &recorder, &audioManager, &spectralDisplay, &activationInterval)
#endif
        , activationInterval(100.0f) // milliseconds
        , testingMode(enableTestingMode)
        , audioStreamingEnabled(false)
        , isFullscreen(false)
        , windowedMode(sf::Vector2u(1280, 720))
        , windowedPosition(100, 100)
        , canvasLeft(80.0f), canvasTop(80.0f), canvasRight(780.0f), canvasBottom(780.0f)
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
        // Enable learning to adapt rhythm connections towards rhythmogram outputs
        network.setLearningEnabled(true);
        network.setLearningRate(0.02f);
        ESSENTIAL_PRINT("🧠 Learning enabled: weights adapt to rhythmogram targets");
        
        // Start with an empty network - users can add neurons via the menu
        
        // Set up visualizer colors
        visualizer.setNeuronColors(sf::Color::Cyan, sf::Color::Red);
        visualizer.setConnectionColors(sf::Color(200, 200, 200, 100), sf::Color::Yellow);
        
#ifdef USE_TGUI
        // Initialize GUI
        guiManager.initialize();
        // Set up adaptive layout based on current window size
        updateLayoutForWindowSize();
#endif
        // Update rhythm interpreter reference after network initialization
        spectralDisplay.setRhythmInterpreter(network.getRhythmInterpreter());
        spectralDisplay.setNeuronNetwork(&network);
        
        DEBUG_PRINT("\nAll changes to code by GitHub Copilot. The prompts were either feature additions or bug fixes for most cases.");
        
        ESSENTIAL_PRINT("Neuron Sequence Sampler initialized with empty network.");
        ESSENTIAL_PRINT("Use the 'Network' menu to add neurons and connections.");
        
        ESSENTIAL_PRINT("Controls:");
        ESSENTIAL_PRINT("  - Mouse: Click to activate neurons");
        ESSENTIAL_PRINT("  - Number keys: Activate specific neurons (when available)");
        ESSENTIAL_PRINT("  - Spacebar: Manual network activation");
        ESSENTIAL_PRINT("  - F key: Toggle filtered audio output (hearing filtered vs original)");
        ESSENTIAL_PRINT("  - + key: Toggle fullscreen mode 🖥️");
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
        // Click detection in the dynamic visualization area
        if (mouseX >= canvasLeft && mouseX <= canvasRight && mouseY >= canvasTop && mouseY <= canvasBottom) {
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

    void handleMouseScroll(int /*delta*/) {
        // Zoom disabled in visualizer; ignore scroll
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
                spectralDisplay.setNeuronNetwork(nullptr);
                
                // Load factory drum pattern preset
                if (PresetManager::loadFactoryPreset(network, "drum_pattern")) {
                    ESSENTIAL_PRINT("📂 Loaded factory drum pattern preset");
                    
                    // Load preset sample files into AudioManager
                    loadPresetSamplesIntoAudioManager(network, audioManager);
                    
                    // Update AudioManager with new rhythm interpreter
                    if (network.getRhythmInterpreter()) {
                        audioManager.setRhythmInterpreter(network.getRhythmInterpreter());
                        ESSENTIAL_PRINT("🔄 AudioManager updated after preset load");
                    }
                    
                    // Update spectral display with new rhythm interpreter
                    spectralDisplay.setRhythmInterpreter(network.getRhythmInterpreter());
                    spectralDisplay.setNeuronNetwork(&network);
                    
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
            case sf::Keyboard::Key::O:
                // Open file dialog to select input audio for rhythmogram analysis
#ifdef USE_TGUI
                guiManager.openInputFileDialog();
                ESSENTIAL_PRINT("📂 Opened audio file dialog for rhythmogram input");
#else
                ESSENTIAL_PRINT("📂 File dialog requires TGUI support");
#endif
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
            case sf::Keyboard::Key::Equal:  // + key (Shift+Equal)
            case sf::Keyboard::Key::Add:    // Numpad + key
                // Toggle fullscreen mode
                toggleFullscreen();
                break;
            case sf::Keyboard::Key::Z:
            case sf::Keyboard::Key::X:
            case sf::Keyboard::Key::Left:
            case sf::Keyboard::Key::Right:
            case sf::Keyboard::Key::Up:
            case sf::Keyboard::Key::Down:
                // Forward zoom and pan keys to visualizer
                visualizer.handleKeyPress(key);
                break;
            default:
                // Forward key to spectral display for its controls
                spectralDisplay.handleKeyPress(key);
                break;
        }
    }

    void toggleFullscreen() {
        if (isFullscreen) {
            // Switch to windowed mode
            windowedPosition = window.getPosition();
            window.create(windowedMode, "Neuron Sequence Sampler");
            window.setPosition(windowedPosition);
            
            // Reset window view for windowed mode
            sf::View defaultView(sf::FloatRect({0, 0}, {static_cast<float>(windowedMode.size.x), static_cast<float>(windowedMode.size.y)}));
            window.setView(defaultView);
            
            isFullscreen = false;
            ESSENTIAL_PRINT("🖥️ Switched to windowed mode (+ key to toggle)");
        } else {
            // Switch to fullscreen mode
            sf::Vector2u currentSize = window.getSize();
            windowedMode = sf::VideoMode(currentSize);
            windowedPosition = window.getPosition();
            
            // Try borderless window instead of true fullscreen to avoid cursor confinement
            auto desktopMode = sf::VideoMode::getDesktopMode();
            std::cout << "🖥️ Desktop mode: " << desktopMode.size.x << "x" << desktopMode.size.y << std::endl;
            
            // Use borderless window positioned at 0,0 instead of true fullscreen
            window.create(desktopMode, "Neuron Sequence Sampler");  // No style parameter = default borderless
            window.setPosition({0, 0});
            
            // Reset window view
            sf::View defaultView(sf::FloatRect({0, 0}, {static_cast<float>(desktopMode.size.x), static_cast<float>(desktopMode.size.y)}));
            window.setView(defaultView);
            std::cout << "🖥️ Using borderless window at 0,0 instead of true fullscreen" << std::endl;
            
            isFullscreen = true;
            ESSENTIAL_PRINT("🖥️ Switched to fullscreen mode (+ key to toggle)");
        }
        
        // Reinitialize TGUI after window recreation
#ifdef USE_TGUI
        gui.setTarget(window);
        // Clear any cached view/coordinate data that might be stale
        gui.getContainer()->removeAllWidgets();
        // Re-initialize GUI completely for new window size
        guiManager.initialize();
        updateLayoutForWindowSize();
#endif
    }

    void updateLayoutForWindowSize() {
        sf::Vector2u windowSize = window.getSize();
        float width = static_cast<float>(windowSize.x);
        float height = static_cast<float>(windowSize.y);
        
        // Calculate layout proportions for current window size
        float availableHeight = height * 0.75f; // Leave 1/5 for spectral + some margin
        float canvasSize = std::min(width * 0.65f, availableHeight); // Ensure square fits
        
        // Use minimal padding in fullscreen to avoid cursor clipping, normal padding in windowed mode
        float canvasPadding = isFullscreen ? 10.0f : 80.0f;
        
        // Debug: Print layout calculations
        std::cout << "📏 Layout Update: " << width << "x" << height 
                  << " Mode: " << (isFullscreen ? "Fullscreen" : "Windowed")
                  << " Padding: " << canvasPadding 
                  << " CanvasSize: " << canvasSize << std::endl;
        
        // Update visualizer canvas (square, left side)
        canvasLeft = canvasPadding;
        canvasTop = canvasPadding;
        canvasRight = canvasPadding + canvasSize;
        canvasBottom = canvasPadding + canvasSize;
        visualizer.setCanvasArea(canvasLeft, canvasTop, canvasRight, canvasBottom);
        
        // Update GUI area (right side)
        float guiX = canvasPadding + canvasSize + 20.0f;
        std::cout << "🎛️ GUI Area: x=" << guiX << " (should leave canvas 0 to " << (canvasPadding + canvasSize) << ")" << std::endl;
        guiManager.setGUIArea(guiX, 0.0f, width, height);
        
        // Update spectral display (bottom) - 1/5 of window height
        float spectralHeight = height / 5.0f; // 20% of window height
        float spectralY = height - spectralHeight - 20.0f; // Position from bottom with margin
        spectralDisplay.setPosition(canvasPadding, spectralY);
        spectralDisplay.setSize(canvasSize * 1.25f, spectralHeight); // 25% wider
        
        // Adjust neuron radius based on canvas size
        float neuronRadius = std::max(15.0f, canvasSize / 40.0f);
        visualizer.setNeuronRadius(neuronRadius);
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

    // Stream external input file to rhythmogram when active
    if (audioManager.isInputStreaming()) {
        auto chunk = audioManager.getNextInputChunk(512);
        if (!chunk.empty()) {
            network.processAudioForRhythm(chunk);
        }
    } else {
        // When no external input, use network's own audio output for rhythm analysis
        
        // This works whether recording is active or not
        auto outputChunk = recorder.getRealtimeAudioBuffer(512);
        if (!outputChunk.empty()) {
            network.processAudioForRhythm(outputChunk);
        }
    }
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
    // Prepare app initialization as background task
    std::atomic<bool> loadingComplete(false);
    
    auto initTask = []() {
        // Initialize preset system
        PresetManager::createPresetDirectory();
        
        std::cout << "🎵 Neuron Sequence Sampler" << std::endl;
        std::cout << "💾 Preset Controls:" << std::endl;
        std::cout << "   S - Save current network as preset" << std::endl;
        std::cout << "   L - Load factory drum pattern preset" << std::endl;
        std::cout << std::endl;
    };
    
    // Show startup animation while loading in background
    StartupAnimation::showWindow(initTask, &loadingComplete);
    
    NeuronSeqSampler app;
    app.run();
    return 0;
}
