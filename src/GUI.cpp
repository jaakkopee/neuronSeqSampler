#include "GUI.h"
#include "NeuronNetwork.h"
#include "Connection.h"
#include "AudioManager.h"
#include "Visualizer.h"
#include "Recorder.h"
#include "RhythmInterpreter.h"
#include "SimpleSpectralDisplay.h"
#include <TGUI/Widgets/FileDialog.hpp>
#include "Debug.h"
#include <cmath>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <map>

GUI::GUI(tgui::Gui* tguiGui, sf::RenderWindow* renderWindow, NeuronNetwork* neuronNetwork, Visualizer* visualizerPtr, Recorder* recorderPtr, AudioManager* audioMgr, SimpleSpectralDisplay* spectralDisplayPtr, float* activationIntervalPtr)
    : gui(tguiGui)
    , window(renderWindow)
    , network(neuronNetwork)
    , visualizer(visualizerPtr)
    , recorder(recorderPtr)
    , audioManager(audioMgr)
    , spectralDisplay(spectralDisplayPtr)
    , activationInterval(activationIntervalPtr)
{
    // Initialize quantization system
    quantizer = std::make_unique<Quantizer>(120.0f, 44100); // Default 120 BPM, 44.1kHz
    quantizerWidget = std::make_unique<QuantizerWidget>(*quantizer);
}

void GUI::initialize() {
    createMenuBar();
    createControlPanel();
    createNeuronSliders();
    createConnectionSliders();
    createConnectionMatrixPanel();
    updateStatusDisplay();  // Initialize status display with current network state
    
    // Initialize quantizer widget
    if (quantizerWidget) {
        std::cout << "🎵 Reinitializing quantizer widget..." << std::endl;
        
        // Clean up previous initialization if it exists
        quantizerWidget->cleanup();
        
        // Create a popover ChildWindow for quantizer controls sized to fit the widget
        quantizerWindow = tgui::ChildWindow::create("Quantization");
        // Use a compact pixel size to better fit the quantizer content
        quantizerWindow->setSize(360, 420);
        // Position near top-center with a bit of padding
        quantizerWindow->setPosition("50% - 180", "8%");
        quantizerWindow->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 40));
        quantizerWindow->getRenderer()->setBorderColor(tgui::Color(80, 80, 80));
        quantizerWindow->getRenderer()->setBorders(2);
        quantizerWindow->setVisible(false); // start hidden; toggled by user
        quantizerWindow->setResizable(false);
        quantizerWindow->setTitleButtons(tgui::ChildWindow::TitleButton::None); // Remove close button
        
        gui->add(quantizerWindow, "QuantizerWindow");

        // Initialize the quantizer widget inside the child window
        quantizerWidget->initialize(quantizerWindow);
        // Make sure the quantizer main panel is centered and sized to fit nicely
        quantizerWidget->setPosition(20.0f, 20.0f, 320.0f, 380.0f);

        // Initialize quantizer BPM to match default BPM (120.0)
        updateQuantizerBPM(120.0f);
        
        std::cout << "✅ Quantizer widget reinitialized successfully" << std::endl;
    }
    
    // Connect quantizer to the neuron network
    if (network && quantizer) {
        network->setQuantizer(quantizer.get());
        std::cout << "🎵 Quantizer connected to neuron network" << std::endl;
    }
    
    // Enable rhythmogram analysis by default for neuron activation
    // Audio always plays directly, rhythmogram data drives neural network
    if (audioManager) {
        DEBUG_PRINT("🎛️  GUI: Enabling rhythmogram analysis for neuron activation (audio plays directly)");
        audioManager->setFilterMode(true);  // Enable rhythmogram analysis
    }
}

void GUI::createMenuBar() {
    // Create menu bar
    auto menuBar = tgui::MenuBar::create();
    menuBar->setSize("100%", "4%");
    menuBar->setPosition("0%", "0%");
    menuBar->getRenderer()->setBackgroundColor(tgui::Color(30, 30, 30));
    menuBar->getRenderer()->setTextColor(tgui::Color::White);
    gui->add(menuBar, "MenuBar");
    
    // Add "Network" menu
    menuBar->addMenu("Network");
    menuBar->addMenuItem("Network", "Add Neuron");
    menuBar->addMenuItem("Network", "Remove Neuron");
    menuBar->addMenuItem("Network", "Add Connection");
    menuBar->addMenuItem("Network", "Remove Connection");
    menuBar->addMenuItem("Network", "Reset Network");
    
    // Add "Recording" menu
    menuBar->addMenu("Recording");
    menuBar->addMenuItem("Recording", "Record NeuronSeq Output");
    menuBar->addMenuItem("Recording", "Record External Microphone");
    menuBar->addMenuItem("Recording", "Stop Recording");
    
    // Add "Presets" menu
    menuBar->addMenu("Presets");
    menuBar->addMenuItem("Presets", "Save Preset");
    menuBar->addMenuItem("Presets", "Load Preset");
    menuBar->addMenuItem("Presets", "Load Factory Drum Pattern");
    menuBar->addMenuItem("Presets", "Browse Presets");
    
    // Connect menu actions
    menuBar->connectMenuItem("Network", "Add Neuron", [this]() { this->addNeuron(); });
    menuBar->connectMenuItem("Network", "Remove Neuron", [this]() { this->removeNeuron(); });
    menuBar->connectMenuItem("Network", "Add Connection", [this]() { this->showConnectionDialog(); });
    menuBar->connectMenuItem("Network", "Remove Connection", [this]() { this->showRemoveConnectionDialog(); });
    menuBar->connectMenuItem("Network", "Reset Network", [this]() { this->resetNetwork(); });
    menuBar->connectMenuItem("Recording", "Record NeuronSeq Output", [this]() { this->startInternalRecording(); });
    menuBar->connectMenuItem("Recording", "Record External Microphone", [this]() { this->startExternalRecording(); });
    menuBar->connectMenuItem("Recording", "Stop Recording", [this]() { this->stopRecording(); });
    menuBar->connectMenuItem("Presets", "Save Preset", [this]() { this->showSavePresetDialog(); });
    menuBar->connectMenuItem("Presets", "Load Preset", [this]() { this->showLoadPresetDialog(); });
    menuBar->connectMenuItem("Presets", "Load Factory Drum Pattern", [this]() { this->loadFactoryDrumPattern(); });
    menuBar->connectMenuItem("Presets", "Browse Presets", [this]() { this->showPresetBrowser(); });
    
    // Adjust control panel position to account for menu bar
    controlPanelTopOffset = 4.0f; // 4% for menu bar
}

void GUI::createControlPanel() {
    // Create main control panel
    controlPanel = tgui::Panel::create({"20%", "96%"}); // Increased width for better slider fit
    controlPanel->setPosition("80%", "4%"); // Adjusted position for wider panel
    controlPanel->setSize({"20%", "96%"});
    controlPanel->getRenderer()->setBackgroundColor(tgui::Color(50, 50, 50, 180));
    gui->add(controlPanel, "ControlPanel");
    
    // Status label
    statusLabel = tgui::Label::create("Neuron Sequence Sampler");
    statusLabel->setPosition("5%", "2%");
    statusLabel->setTextSize(14);
    statusLabel->getRenderer()->setTextColor(tgui::Color::White);
    controlPanel->add(statusLabel, "StatusLabel");
    
    // Activation Interval Control
    activationIntervalLabel = tgui::Label::create("Update Rate: 100ms");
    activationIntervalLabel->setPosition("5%", "12%");
    activationIntervalLabel->setTextSize(10);
    activationIntervalLabel->getRenderer()->setTextColor(tgui::Color::White);
    controlPanel->add(activationIntervalLabel, "ActivationIntervalLabel");
    
    activationIntervalSlider = tgui::Slider::create();
    activationIntervalSlider->setPosition("5%", "16%");
    activationIntervalSlider->setSize("90%", "3%");
    activationIntervalSlider->setMinimum(1.0f);    // 1ms minimum (very fast)
    activationIntervalSlider->setMaximum(1000.0f); // 1000ms maximum (1 second)
    activationIntervalSlider->setStep(1.0f);       // 1ms increments
    activationIntervalSlider->setValue(activationInterval ? *activationInterval : 100.0f);
    
    // Connect slider to callback
    activationIntervalSlider->onValueChange([this](float value) {
        if (activationInterval) {
            *activationInterval = value;
            activationIntervalLabel->setText("Update Rate: " + std::to_string(static_cast<int>(value)) + "ms");
        }
    });
    controlPanel->add(activationIntervalSlider, "ActivationIntervalSlider");
    
    // View Mode Control
    viewModeLabel = tgui::Label::create("View Mode:");
    viewModeLabel->setPosition("5%", "21%");
    viewModeLabel->setTextSize(10);
    viewModeLabel->getRenderer()->setTextColor(tgui::Color::White);
    controlPanel->add(viewModeLabel, "ViewModeLabel");
    
    viewModeComboBox = tgui::ComboBox::create();
    viewModeComboBox->setPosition("5%", "24%");
    viewModeComboBox->setSize("90%", "4%");
    viewModeComboBox->addItem("Grid Layout");
    viewModeComboBox->addItem("Circular Layout");
    viewModeComboBox->setSelectedItem("Grid Layout");
    
    // Connect combobox to callback
    viewModeComboBox->onItemSelect([this](const tgui::String& item) {
        if (visualizer) {
            if (item == "Grid Layout") {
                visualizer->setViewMode(ViewMode::Grid);
            } else if (item == "Circular Layout") {
                visualizer->setViewMode(ViewMode::Circular);
            }
        }
    });
    controlPanel->add(viewModeComboBox, "ViewModeComboBox");
    
    // Spectral Display Contrast Control
    spectralContrastLabel = tgui::Label::create("Spectral Contrast: 100%");
    spectralContrastLabel->setPosition("5%", "29%");
    spectralContrastLabel->setTextSize(10);
    spectralContrastLabel->getRenderer()->setTextColor(tgui::Color::White);
    controlPanel->add(spectralContrastLabel, "SpectralContrastLabel");
    
    spectralContrastSlider = tgui::Slider::create();
    spectralContrastSlider->setPosition("5%", "32%");
    spectralContrastSlider->setSize("90%", "3%");
    spectralContrastSlider->setMinimum(10.0f);    // 10% contrast (low contrast)
    spectralContrastSlider->setMaximum(1000.0f);  // 1000% contrast (maximum contrast)
    spectralContrastSlider->setStep(5.0f);        // 5% increments
    spectralContrastSlider->setValue(100.0f);     // Default to normal contrast
    
    // Connect slider to callback
    spectralContrastSlider->onValueChange([this](float value) {
        if (spectralDisplay) {
            spectralDisplay->setContrast(value / 100.0f); // Convert percentage to factor
            spectralContrastLabel->setText("Spectral Contrast: " + std::to_string(static_cast<int>(value)) + "%");
        }
    });
    controlPanel->add(spectralContrastSlider, "SpectralContrastSlider");
    
    // Create scrollable panel for sliders - adjusted position for new control
    slidersPanel = tgui::ScrollablePanel::create({"95%", "62%"}); // Reduced height to accommodate opacity control
    slidersPanel->setPosition("2.5%", "36%"); // Moved down to accommodate opacity control
    slidersPanel->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 40, 200));
    controlPanel->add(slidersPanel, "SlidersPanel");
}

void GUI::createConnectionSliders() {
    if (!network) return;
    
    if (!slidersPanel) {
        return;
    }
    
    // Remove old connection sliders and labels from the panel
    for (auto& slider : connectionSliders) {
        slidersPanel->remove(slider);
    }
    for (auto& label : connectionLabels) {
        slidersPanel->remove(label);
    }
    for (auto& valueLabel : connectionValueLabels) {
        slidersPanel->remove(valueLabel);
    }
    
    connectionSliders.clear();
    connectionLabels.clear();
    connectionValueLabels.clear();
    
    const auto& connections = network->getConnections();
    float yPos = 15.0f; // Start at same height as neuron sliders (they're in different columns)
    
    for (size_t i = 0; i < connections.size(); ++i) {
        const Connection* conn = connections[i].get();
        
        // Create label - positioned in right column
        auto label = tgui::Label::create("C" + std::to_string(i + 1) + ":");
        label->setPosition(215, yPos);  // Right column start
        label->setTextSize(10);
        label->getRenderer()->setTextColor(tgui::Color::White);
        slidersPanel->add(label);
        connectionLabels.push_back(label);
        
        // Create slider - positioned in right column
        auto slider = tgui::Slider::create();
        slider->setPosition(250, yPos);  // Right column slider position
        slider->setSize(120, 16);  // Smaller width to fit right column
        slider->setMinimum(-1.2f);
        slider->setMaximum(1.2f);
        slider->setStep(0.01f);
        slider->setValue(conn->getWeight());
        
        // Connect slider to callback
        slider->onValueChange([this, i, slider](float value) {
            this->onSliderChanged(i, value);
            // Remove focus from the slider and unfocus all widgets so global key events work
            slider->setFocused(false);
            if (gui)
                gui->unfocusAllWidgets();
        });
        
        slidersPanel->add(slider);
        connectionSliders.push_back(slider);
        
        // Value label - positioned in right column
        auto valueLabel = tgui::Label::create(std::to_string(conn->getWeight()));
        valueLabel->setPosition(380, yPos);  // Right column value position
        valueLabel->setTextSize(9);
        valueLabel->getRenderer()->setTextColor(tgui::Color::Yellow);
        slidersPanel->add(valueLabel);
        connectionValueLabels.push_back(valueLabel);
        
        yPos += 22.0f;
    }
    
    DEBUG_PRINT_STREAM("Created " << connectionSliders.size() << " connection sliders");
}

void GUI::createNeuronSliders() {
    if (!network) return;
    
    // Remove only neuron-related widgets, not connection sliders
    for (auto& slider : neuronSliders) {
        slidersPanel->remove(slider);
    }
    for (auto& label : neuronLabels) {
        slidersPanel->remove(label);
    }
    for (auto& valueLabel : neuronValueLabels) {
        slidersPanel->remove(valueLabel);
    }
    for (auto& combo : activationFunctionCombos) {
        slidersPanel->remove(combo);
    }
    // Remove sample buttons if any
    for (auto& btn : neuronSampleButtons) {
        slidersPanel->remove(btn);
    }
    
    // Remove existing leak controls
    for (auto& s : neuronLeakSliders) slidersPanel->remove(s);
    for (auto& l : neuronLeakLabels) slidersPanel->remove(l);
    for (auto& v : neuronLeakValueLabels) slidersPanel->remove(v);
    neuronLeakSliders.clear();
    neuronLeakLabels.clear();
    neuronLeakValueLabels.clear();

    neuronSliders.clear();
    neuronLabels.clear();
    neuronValueLabels.clear();
    activationFunctionCombos.clear();
    neuronSampleButtons.clear();
    
    const auto& neurons = network->getNeurons();
    // Always start neuron sliders at a consistent position (not dependent on connection count)
    float yPos = 15.0f; // Fixed starting position for consistent behavior
    
    for (size_t i = 0; i < neurons.size(); ++i) {
        const Neuron* neuron = neurons[i].get();
        
        // Create label
        auto label = tgui::Label::create("N" + std::to_string(i + 1) + ":");
        label->setPosition(5, yPos);
        label->setTextSize(10);
        label->getRenderer()->setTextColor(tgui::Color::Cyan);
        slidersPanel->add(label);
        neuronLabels.push_back(label);
        
        // Create slider for activation increase per iteration
        auto slider = tgui::Slider::create();
        slider->setPosition(40, yPos);
        slider->setSize(120, 16);  // Slightly smaller to fit left column
        slider->setMinimum(-0.1f);
        slider->setMaximum(0.6f);
        slider->setStep(0.01f);
        slider->setValue(neuron->getActivationIncreasePerIteration());
        
        // Connect slider to callback (only once)
        slider->onValueChange([this, i, slider](float value) {
            this->onNeuronSliderChanged(i, value);
            // Remove focus from the slider and unfocus all widgets so global key events work
            slider->setFocused(false);
            if (gui)
                gui->unfocusAllWidgets();
        });
        
        slidersPanel->add(slider);
        neuronSliders.push_back(slider);
        
        // Value label
        auto valueLabel = tgui::Label::create(std::to_string(neuron->getActivationIncreasePerIteration()));
        valueLabel->setPosition(170, yPos);
        valueLabel->setTextSize(9);
        valueLabel->getRenderer()->setTextColor(tgui::Color::Cyan);
        slidersPanel->add(valueLabel);
        neuronValueLabels.push_back(valueLabel);
        
        yPos += 22.0f;
        
        // Create activation function dropdown below the slider
        auto funcCombo = tgui::ComboBox::create();
        funcCombo->setPosition(40, yPos);
        funcCombo->setSize(120, 16);
        funcCombo->addItem("Linear");
        funcCombo->addItem("Sigmoid");
        funcCombo->addItem("ReLU");
        funcCombo->addItem("Tanh");
        
        // Set current selection based on neuron's activation function
        switch (neuron->getActivationFunction()) {
            case ActivationFunction::Linear:
                funcCombo->setSelectedItem("Linear");
                break;
            case ActivationFunction::Sigmoid:
                funcCombo->setSelectedItem("Sigmoid");
                break;
            case ActivationFunction::ReLU:
                funcCombo->setSelectedItem("ReLU");
                break;
            case ActivationFunction::Tanh:
                funcCombo->setSelectedItem("Tanh");
                break;
        }
        
        // Connect to callback
        funcCombo->onItemSelect([this, i](const tgui::String& item) {
            this->onActivationFunctionChanged(i, item.toStdString());
        });
        
        slidersPanel->add(funcCombo);
        activationFunctionCombos.push_back(funcCombo);
        
        // Function label
        auto funcLabel = tgui::Label::create("Func:");
        funcLabel->setPosition(5, yPos);
        funcLabel->setTextSize(9);
        funcLabel->getRenderer()->setTextColor(tgui::Color::Green);
        slidersPanel->add(funcLabel);

        // Create sample selection button to the right of the function combo
        std::string sampleFile = neuron->getSampleFilePath();
        std::string sampleLabelText = "(no sample)";
        if (!sampleFile.empty()) {
            try {
                sampleLabelText = std::filesystem::path(sampleFile).filename().string();
            } catch (...) {
                sampleLabelText = "(no sample)";
            }
        }

        // Truncate long filenames to avoid UI overlap
        std::string displayLabel = sampleLabelText;
        const size_t maxLabelLen = 12;
        if (displayLabel.size() > maxLabelLen) {
            displayLabel = displayLabel.substr(0, maxLabelLen - 3) + "...";
        }

        auto sampleButton = tgui::Button::create(displayLabel);
        // Position sample button to the right of the function combo but left of connection controls
        // Func combo is at x=40 width=120 -> ends at 160. Connection column begins at ~215.
        // Use a small button in the gap (x=165..215)
        float sampleBtnX = 165.0f;
        sampleButton->setPosition(sampleBtnX, yPos); // align with function combo row
        sampleButton->setSize(50, 18);
        sampleButton->setTextSize(9);
        sampleButton->getRenderer()->setBackgroundColor(tgui::Color(70, 70, 70));
        sampleButton->getRenderer()->setTextColor(tgui::Color::White);

        // Capture index for the callback
        sampleButton->onPress([this, i]() {
            this->showChangeSampleDialog(i);
        });

        slidersPanel->add(sampleButton);
        neuronSampleButtons.push_back(sampleButton);

        // Leak rate slider row
        auto leakLabel = tgui::Label::create("Leak:");
        leakLabel->setPosition(5, yPos);
        leakLabel->setTextSize(9);
        leakLabel->getRenderer()->setTextColor(tgui::Color(200, 200, 200));
        slidersPanel->add(leakLabel);
        neuronLeakLabels.push_back(leakLabel);

        auto leakSlider = tgui::Slider::create();
        leakSlider->setPosition(40, yPos);
        leakSlider->setSize(120, 16);
        leakSlider->setMinimum(0.0f);
        leakSlider->setMaximum(0.3f);
        leakSlider->setStep(0.01f);
        leakSlider->setValue(neuron->getLeakRate());
        leakSlider->onValueChange([this, i, leakSlider](float value){
            if (!network || i >= network->getNeuronCount()) return;
            Neuron* n = network->getNeuron(i);
            if (n) {
                n->setLeakRate(value);
                // Update the value label
                if (i < neuronLeakValueLabels.size()) {
                    neuronLeakValueLabels[i]->setText(std::to_string(value));
                }
            }
            leakSlider->setFocused(false);
            if (gui) gui->unfocusAllWidgets();
        });
        slidersPanel->add(leakSlider);
        neuronLeakSliders.push_back(leakSlider);

        auto leakValueLabel = tgui::Label::create(std::to_string(neuron->getLeakRate()));
        leakValueLabel->setPosition(170, yPos);
        leakValueLabel->setTextSize(9);
        leakValueLabel->getRenderer()->setTextColor(tgui::Color(200, 200, 200));
        slidersPanel->add(leakValueLabel);
        neuronLeakValueLabels.push_back(leakValueLabel);

        yPos += 22.0f; // Extra spacing after each neuron's controls
    }
    
    DEBUG_PRINT_STREAM("Created " << neuronSliders.size() << " neuron sliders and " << activationFunctionCombos.size() << " activation function dropdowns");
}

void GUI::onNeuronSliderChanged(size_t neuronIndex, float value) {
    if (!network || neuronIndex >= network->getNeuronCount()) return;
    
    Neuron* neuron = network->getNeuron(neuronIndex);
    if (neuron) {
        neuron->setActivationIncreasePerIteration(value);
        std::cout << "Updated neuron " << neuronIndex << " activation increase per iteration to " << value << std::endl;
        
        // Update the value label using the proper container
        if (neuronIndex < neuronValueLabels.size()) {
            neuronValueLabels[neuronIndex]->setText(std::to_string(value));
        }
    }
}

void GUI::onActivationFunctionChanged(size_t neuronIndex, const std::string& functionName) {
    if (!network || neuronIndex >= network->getNeuronCount()) return;
    
    Neuron* neuron = network->getNeuron(neuronIndex);
    if (neuron) {
        ActivationFunction func = ActivationFunction::Linear; // default
        
        if (functionName == "Sigmoid") {
            func = ActivationFunction::Sigmoid;
        } else if (functionName == "ReLU") {
            func = ActivationFunction::ReLU;
        } else if (functionName == "Tanh") {
            func = ActivationFunction::Tanh;
        } else if (functionName == "Linear") {
            func = ActivationFunction::Linear;
        }
        
        neuron->setActivationFunction(func);
        std::cout << "Updated neuron " << neuronIndex << " activation function to " << functionName << std::endl;
    }
}

void GUI::onSliderChanged(size_t connectionIndex, float value) {
    if (!network || connectionIndex >= network->getConnectionCount()) return;
    
    Connection* conn = network->getConnection(connectionIndex);
    if (conn) {
        conn->setWeight(value);
        std::cout << "Updated connection " << connectionIndex << " weight to " << value << std::endl;
        
        // Update the value label using the proper container
        if (connectionIndex < connectionValueLabels.size()) {
            connectionValueLabels[connectionIndex]->setText(std::to_string(value));
        }
    }
}

void GUI::update() {
    updateStatusDisplay();
    
    // Only update connection matrix every 10 frames to reduce interference
    // But skip entirely if recent toggle interactions occurred
    if (toggleBlockCounter > 0) {
        toggleBlockCounter--;
    } else {
        matrixUpdateCounter++;
        if (matrixUpdateCounter >= 10) {
            updateConnectionMatrix();
            matrixUpdateCounter = 0;
        }
    }
}

void GUI::updateStatusDisplay() {
    if (!network || !statusLabel) return;
    
    std::string status = "Neurons: " + std::to_string(network->getNeuronCount()) + 
                        " | Connections: " + std::to_string(network->getConnectionCount());
    statusLabel->setText(status);
    
    // Update tempo displays when auto-tempo is enabled
    if (network->getRhythmInterpreter() && autodetectTempoToggle && bpmLabel && detectedTempoLabel) {
        auto rhythmInterpreter = network->getRhythmInterpreter();
        if (rhythmInterpreter->isAutoTempoEnabled()) {
            float detectedTempo = rhythmInterpreter->getDetectedTempo();
            std::ostringstream stream;
            stream << std::fixed << std::setprecision(1) << detectedTempo;
            bpmLabel->setText(stream.str());
            
            // Update detected tempo display
            std::ostringstream detectedStream;
            detectedStream << "Detected: " << std::fixed << std::setprecision(1) << detectedTempo;
            detectedTempoLabel->setText(detectedStream.str());
            
            // Update quantizer BPM to match detected tempo
            updateQuantizerBPM(detectedTempo);
        } else {
            // Clear detected tempo display when auto-tempo is off
            detectedTempoLabel->setText("Detected: --");
        }
    }
}

void GUI::render() {
    // TGUI rendering is handled automatically by the gui
}

void GUI::setGUIArea(float x, float y, float width, float height) {
    // Keep using percentage-based layout for TGUI compatibility
    // The right panel positioning is handled in individual widget creation
    // Parameters are received but not stored as GUI uses percentage layout
    
    // Keep control panel at percentage-based position for proper TGUI function
    if (controlPanel) {
        controlPanel->setPosition("70%", "0%");
        controlPanel->setSize("30%", "100%");
    }
}

void GUI::refreshConnectionSliders() {
    createConnectionSliders();
}

void GUI::refreshNeuronSliders() {
    createNeuronSliders();
}

void GUI::refreshConnectionMatrix() {
    // Prevent recursive calls and unnecessary refreshes
    if (isUpdatingMatrix) {
        return;
    }
    
    // Only recreate the panel if it doesn't exist or if the number of connections changed
    if (!connectionMatrixWindow || !network || !network->getRhythmInterpreter()) {
        createConnectionMatrixPanel();
        return;
    }
    
    // Check if connection count changed, which requires panel recreation
    size_t currentConnections = network->getConnectionCount();
    size_t expectedButtons = matrixToggleButtons.empty() ? 0 : matrixToggleButtons[0].size();
    
    if (currentConnections != expectedButtons) {
        // Connection count changed, need to recreate
        createConnectionMatrixPanel();
    } else {
        // Network structure unchanged, force one matrix update
        forceMatrixUpdate();
    }
}

void GUI::setSliderValue(size_t connectionIndex, float value) {
    if (connectionIndex < connectionSliders.size()) {
        connectionSliders[connectionIndex]->setValue(value);
    }
}

void GUI::addNeuron() {
    showAddNeuronDialog();
}

void GUI::removeNeuron() {
    if (!network) return;
    
    size_t neuronCount = network->getNeuronCount();
    if (neuronCount == 0) {
        // Show message that there are no neurons to remove
        auto messageDialog = tgui::ChildWindow::create("No Neurons");
        messageDialog->setSize(300, 150);
        messageDialog->setPosition("50%", "50%");
        // Style the no neurons dialog
        messageDialog->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 50, 240));
        messageDialog->getRenderer()->setBorderColor(tgui::Color(80, 80, 100));
        messageDialog->getRenderer()->setTitleColor(tgui::Color::Yellow);
        
        auto message = tgui::Label::create("There are no neurons to remove.\n\nAdd some neurons first.");
        message->setPosition(10, 40);
        message->setSize(280, 60);
        message->getRenderer()->setTextColor(tgui::Color::White);
        messageDialog->add(message);
        
        auto okButton = tgui::Button::create("OK");
        okButton->setPosition(100, 110);
        okButton->setSize(100, 30);
        okButton->onPress([=]() {
            messageDialog->close();
        });
        messageDialog->add(okButton);
        
        gui->add(messageDialog);
        return;
    }
    
    // Create a dialog to select which neuron to remove
    auto dialog = tgui::ChildWindow::create("Remove Neuron");
    dialog->setSize("300", "150");
    dialog->setPosition("50% - 150", "50% - 75");
    // Style the remove neuron dialog
    dialog->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 50, 240));
    dialog->getRenderer()->setBorderColor(tgui::Color(80, 120, 180));
    dialog->getRenderer()->setTitleColor(tgui::Color::White);
    
    auto label = tgui::Label::create("Select neuron to remove:");
    label->setPosition("10", "30");
    label->getRenderer()->setTextColor(tgui::Color::White);
    dialog->add(label);
    
    auto comboBox = tgui::ComboBox::create();
    comboBox->setPosition("10", "60");
    comboBox->setSize("200", "25");
    // Style the combobox
    comboBox->getRenderer()->setBackgroundColor(tgui::Color(60, 60, 70));
    comboBox->getRenderer()->setTextColor(tgui::Color::White);
    comboBox->getRenderer()->setBorderColor(tgui::Color(80, 120, 180));
    
    // Populate with neuron indices
    for (size_t i = 0; i < neuronCount; ++i) {
        comboBox->addItem("Neuron " + std::to_string(i + 1));
    }
    dialog->add(comboBox);
    
    auto removeButton = tgui::Button::create("Remove");
    removeButton->setPosition("10", "100");
    removeButton->setSize("80", "30");
    // Style the remove button with red theme
    removeButton->getRenderer()->setBackgroundColor(tgui::Color(120, 40, 40));
    removeButton->getRenderer()->setBackgroundColorHover(tgui::Color(150, 50, 50));
    removeButton->getRenderer()->setTextColor(tgui::Color::White);
    removeButton->getRenderer()->setBorderColor(tgui::Color(180, 60, 60));
    removeButton->onClick([this, dialog, comboBox]() {
        int selectedIndex = comboBox->getSelectedItemIndex();
        if (selectedIndex >= 0) {
            std::cout << "Removing neuron " << (selectedIndex + 1) << std::endl;
            
            if (network->removeNeuron(selectedIndex)) {
                std::cout << "Successfully removed neuron. Network now has " 
                         << network->getNeuronCount() << " neurons and " 
                         << network->getConnectionCount() << " connections" << std::endl;
                
                // Refresh GUI and visualizer
                refreshNeuronSliders();
                refreshConnectionSliders();
                refreshConnectionMatrix();
                updateStatusDisplay();
                
                // Update visualizer layout since neuron positions may have changed
                if (visualizer) {
                    visualizer->refreshLayout();
                }
            } else {
                std::cerr << "Failed to remove neuron " << (selectedIndex + 1) << std::endl;
            }
        }
        dialog->close();
    });
    dialog->add(removeButton);
    
    auto cancelButton = tgui::Button::create("Cancel");
    cancelButton->setPosition("100", "100");
    cancelButton->setSize("80", "30");
    cancelButton->onClick([dialog]() { dialog->close(); });
    dialog->add(cancelButton);
    
    gui->add(dialog);
}

void GUI::showConnectionDialog() {
    if (!network) return;
    
    size_t neuronCount = network->getNeuronCount();
    if (neuronCount < 1) {
        // Show message that we need at least 1 neuron (self-connections are now allowed)
        auto messageDialog = tgui::ChildWindow::create("Cannot Add Connection");
        messageDialog->setSize(300, 150);
        messageDialog->setPosition("50%", "50%");
        // Improve dialog styling - dark background with bright text
        messageDialog->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 50, 240));
        messageDialog->getRenderer()->setBorderColor(tgui::Color(80, 80, 100));
        messageDialog->getRenderer()->setTitleColor(tgui::Color::Yellow);
        
        auto message = tgui::Label::create("You need at least 1 neuron\nto create a connection.\n\nAdd a neuron first.");
        message->setPosition(10, 40);
        message->setSize(280, 60);
        message->getRenderer()->setTextColor(tgui::Color::White);
        messageDialog->add(message);
        
        auto okButton = tgui::Button::create("OK");
        okButton->setPosition(100, 110);
        okButton->setSize(100, 30);
        // Style the button for better visibility
        okButton->getRenderer()->setBackgroundColor(tgui::Color(60, 100, 60));
        okButton->getRenderer()->setTextColor(tgui::Color::White);
        okButton->getRenderer()->setBorderColor(tgui::Color(100, 150, 100));
        okButton->onPress([=]() {
            messageDialog->close();
        });
        messageDialog->add(okButton);
        
        gui->add(messageDialog);
        return;
    }
    
    auto dialog = tgui::ChildWindow::create("Add Connection");
    dialog->setSize("380", "230");
    dialog->setPosition("50% - 190", "50% - 115");
    // Improve dialog styling - dark background with bright accents
    dialog->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 50, 240));
    dialog->getRenderer()->setBorderColor(tgui::Color(80, 120, 180));
    dialog->getRenderer()->setTitleColor(tgui::Color::Cyan);
    
    auto fromLabel = tgui::Label::create("From Neuron:");
    fromLabel->setPosition("10", "30");
    fromLabel->getRenderer()->setTextColor(tgui::Color::White);
    dialog->add(fromLabel);
    
    auto fromCombo = tgui::ComboBox::create();
    fromCombo->setPosition("120", "30");
    fromCombo->setSize("200", "25");
    fromCombo->getRenderer()->setBackgroundColor(tgui::Color(60, 60, 70));
    fromCombo->getRenderer()->setTextColor(tgui::Color::White);
    
    auto toLabel = tgui::Label::create("To Neuron:");
    toLabel->setPosition("10", "70");
    toLabel->getRenderer()->setTextColor(tgui::Color::White);
    dialog->add(toLabel);
    
    auto toCombo = tgui::ComboBox::create();
    toCombo->setPosition("120", "70");
    toCombo->setSize("200", "25");
    toCombo->getRenderer()->setBackgroundColor(tgui::Color(60, 60, 70));
    toCombo->getRenderer()->setTextColor(tgui::Color::White);
    
    auto weightLabel = tgui::Label::create("Weight:");
    weightLabel->setPosition("10", "110");
    weightLabel->getRenderer()->setTextColor(tgui::Color::White);
    dialog->add(weightLabel);
    
    // Add info label for self-connections
    auto infoLabel = tgui::Label::create("Tip: Same neuron = self-connection (arc)");
    infoLabel->setPosition("10", "140");
    infoLabel->setTextSize(10);
    infoLabel->getRenderer()->setTextColor(tgui::Color::Yellow);
    dialog->add(infoLabel);
    
    auto weightSpin = tgui::SpinControl::create();
    weightSpin->setPosition("120", "110");
    weightSpin->setSize("120", "25");
    weightSpin->setMinimum(-1.2f);
    weightSpin->setMaximum(1.2f);
    weightSpin->setStep(0.1f);
    weightSpin->setValue(0.5f);  // Default to positive weight
    // Note: SpinControl styling not supported in this TGUI version
    dialog->add(weightSpin);
    
    // Populate neuron lists
    for (size_t i = 0; i < neuronCount; ++i) {
        std::string neuronName = "Neuron " + std::to_string(i + 1);
        fromCombo->addItem(neuronName);
        toCombo->addItem(neuronName);
    }
    dialog->add(fromCombo);
    dialog->add(toCombo);
    
    auto addButton = tgui::Button::create("Add");
    addButton->setPosition("10", "170");
    addButton->setSize("80", "30");
    // Style the Add button
    addButton->getRenderer()->setBackgroundColor(tgui::Color(60, 120, 60));
    addButton->getRenderer()->setTextColor(tgui::Color::White);
    addButton->getRenderer()->setBorderColor(tgui::Color(100, 200, 100));
    addButton->onClick([this, dialog, fromCombo, toCombo, weightSpin]() {
        int fromIndex = fromCombo->getSelectedItemIndex();
        int toIndex = toCombo->getSelectedItemIndex();
        float weight = weightSpin->getValue();
        
        if (fromIndex >= 0 && toIndex >= 0) {  // Allow self-connections (fromIndex == toIndex)
            // Get the neurons
            Neuron* sourceNeuron = network->getNeuron(fromIndex);
            Neuron* targetNeuron = network->getNeuron(toIndex);
            
            if (sourceNeuron && targetNeuron) {
                // Create the connection
                auto connection = network->connect(sourceNeuron, targetNeuron, weight);
                
                if (fromIndex == toIndex) {
                    std::cout << "Added self-connection to neuron " << (fromIndex + 1) 
                             << " (arc) with weight " << weight << std::endl;
                } else {
                    std::cout << "Added connection from neuron " << (fromIndex + 1) 
                             << " to neuron " << (toIndex + 1) << " with weight " << weight << std::endl;
                }
                std::cout << "Network now has " << network->getConnectionCount() << " connections" << std::endl;
                
                // Refresh GUI and visualizer
                refreshNeuronSliders();
                refreshConnectionSliders();
                refreshConnectionMatrix();
                updateStatusDisplay();
                
                // Update visualizer (connections don't need layout refresh, just visual update)
                if (visualizer) {
                    // The visualizer will automatically pick up new connections in the next render
                    std::cout << "Visualizer notified of new connection" << std::endl;
                }
                
                dialog->close();
            } else {
                std::cerr << "Error: Could not find neurons for connection" << std::endl;
            }
        } else {
            std::cout << "Invalid selection: from=" << fromIndex << ", to=" << toIndex << std::endl;
        }
    });
    dialog->add(addButton);
    
    auto cancelButton = tgui::Button::create("Cancel");
    cancelButton->setPosition("100", "170");
    cancelButton->setSize("80", "30");
    // Style the Cancel button
    cancelButton->getRenderer()->setBackgroundColor(tgui::Color(120, 60, 60));
    cancelButton->getRenderer()->setTextColor(tgui::Color::White);
    cancelButton->getRenderer()->setBorderColor(tgui::Color(200, 100, 100));
    cancelButton->onClick([dialog]() { dialog->close(); });
    dialog->add(cancelButton);
    
    gui->add(dialog);
}

void GUI::showRemoveConnectionDialog() {
    if (!network) return;
    
    size_t connectionCount = network->getConnectionCount();
    if (connectionCount == 0) {
        // Show message that there are no connections to remove
        auto messageDialog = tgui::ChildWindow::create("No Connections");
        messageDialog->setSize(300, 150);
        messageDialog->setPosition("50%", "50%");
        
        auto message = tgui::Label::create("There are no connections to remove.\n\nAdd some connections first.");
        message->setPosition(10, 40);
        message->setSize(280, 60);
        message->getRenderer()->setTextColor(tgui::Color::White);
        messageDialog->add(message);
        
        auto okButton = tgui::Button::create("OK");
        okButton->setPosition(100, 110);
        okButton->setSize(100, 30);
        okButton->onPress([=]() {
            messageDialog->close();
        });
        messageDialog->add(okButton);
        
        gui->add(messageDialog);
        return;
    }
    
    auto dialog = tgui::ChildWindow::create("Remove Connection");
    dialog->setSize("400", "150");
    dialog->setPosition("50% - 200", "50% - 75");
    dialog->getRenderer()->setTitleColor(tgui::Color::White);
    
    auto label = tgui::Label::create("Select connection to remove:");
    label->setPosition("10", "30");
    dialog->add(label);
    
    auto comboBox = tgui::ComboBox::create();
    comboBox->setPosition("10", "60");
    comboBox->setSize("350", "25");
    
    // Populate with connection descriptions
    const auto& connections = network->getConnections();
    const auto& neurons = network->getNeurons();
    
    for (size_t i = 0; i < connections.size(); ++i) {
        const Connection* conn = connections[i].get();
        
        // Find source and target neuron indices
        int sourceIndex = -1, targetIndex = -1;
        for (size_t j = 0; j < neurons.size(); ++j) {
            if (neurons[j].get() == conn->getSource()) {
                sourceIndex = j;
            }
            if (neurons[j].get() == conn->getTarget()) {
                targetIndex = j;
            }
        }
        
        std::string description = "Connection " + std::to_string(i + 1) + ": ";
        if (sourceIndex >= 0 && targetIndex >= 0) {
            description += "Neuron " + std::to_string(sourceIndex + 1) + 
                          " → Neuron " + std::to_string(targetIndex + 1) + 
                          " (weight: " + std::to_string(conn->getWeight()) + ")";
        } else {
            description += "Unknown connection";
        }
        
        comboBox->addItem(description);
    }
    dialog->add(comboBox);
    
    auto removeButton = tgui::Button::create("Remove");
    removeButton->setPosition("10", "100");
    removeButton->setSize("80", "30");
    removeButton->onClick([this, dialog, comboBox]() {
        int selectedIndex = comboBox->getSelectedItemIndex();
        if (selectedIndex >= 0) {
            std::cout << "Removing connection " << (selectedIndex + 1) << std::endl;
            
            if (network->removeConnection(selectedIndex)) {
                std::cout << "Successfully removed connection. Network now has " 
                         << network->getConnectionCount() << " connections" << std::endl;
                
                // Refresh GUI (connections don't need visualizer layout refresh)
                refreshNeuronSliders();
                refreshConnectionSliders();
                updateStatusDisplay();
                
                // Visualizer will automatically pick up the change in next render
                std::cout << "Connection removed from visualizer" << std::endl;
            } else {
                std::cerr << "Failed to remove connection " << (selectedIndex + 1) << std::endl;
            }
        }
        dialog->close();
    });
    dialog->add(removeButton);
    
    auto cancelButton = tgui::Button::create("Cancel");
    cancelButton->setPosition("100", "100");
    cancelButton->setSize("80", "30");
    cancelButton->onClick([dialog]() { dialog->close(); });
    dialog->add(cancelButton);
    
    gui->add(dialog);
}

void GUI::resetNetwork() {
    if (!network) return;
    
    // Create confirmation dialog
    auto confirmDialog = tgui::ChildWindow::create("Reset Network");
    confirmDialog->setSize(350, 150);
    confirmDialog->setPosition("50%", "50%");
    
    auto message = tgui::Label::create("Are you sure you want to reset the network?\n\nThis will remove all neurons and connections.");
    message->setPosition(10, 40);
    message->setSize(330, 60);
    message->getRenderer()->setTextColor(tgui::Color::White);
    confirmDialog->add(message);
    
    auto yesButton = tgui::Button::create("Yes, Reset");
    yesButton->setPosition(50, 110);
    yesButton->setSize(100, 30);
    yesButton->onPress([=]() {
        std::cout << "Resetting network..." << std::endl;
        
        // Step 1: Notify spectral display that rhythm interpreter will be cleared
        // This prevents any further access attempts during the reset process
        if (spectralDisplay) {
            spectralDisplay->setRhythmInterpreter(nullptr);
            std::cout << "🔄 Spectral display cleared" << std::endl;
        }
        
        // Step 2: Clear the entire network (this deletes the rhythm interpreter)
        network->clearNetwork();
        
        std::cout << "Network reset complete. Neurons: " << network->getNeuronCount() 
                 << ", Connections: " << network->getConnectionCount() << std::endl;
        
        // Step 3: Reinitialize rhythm interpreter
        network->initializeRhythmInterpreter();
        
        // Step 4: Update AudioManager with new rhythm interpreter
        auto audioManager = network->getAudioManager();
        if (audioManager && network->getRhythmInterpreter()) {
            audioManager->setRhythmInterpreter(network->getRhythmInterpreter());
            std::cout << "🔄 AudioManager reconnected to new rhythm interpreter" << std::endl;
        }
        
        // Step 5: Update spectral display with new rhythm interpreter
        if (spectralDisplay && network->getRhythmInterpreter()) {
            spectralDisplay->setRhythmInterpreter(network->getRhythmInterpreter());
            std::cout << "🔄 Spectral display reconnected to new rhythm interpreter" << std::endl;
        }
        
        // Refresh GUI and visualizer
        refreshNeuronSliders();
        refreshConnectionSliders();
        updateStatusDisplay();
        
        // Update visualizer layout
        if (visualizer) {
            visualizer->refreshLayout();
        }
        
        confirmDialog->close();
    });
    confirmDialog->add(yesButton);
    
    auto noButton = tgui::Button::create("Cancel");
    noButton->setPosition(200, 110);
    noButton->setSize(100, 30);
    noButton->onPress([=]() {
        confirmDialog->close();
    });
    confirmDialog->add(noButton);
    
    gui->add(confirmDialog);
}

void GUI::showAddNeuronDialog() {
    if (!network) return;
    
    // Create dialog window
    auto dialog = tgui::ChildWindow::create("Add New Neuron");
    dialog->setSize(400, 350);
    dialog->setPosition("50%", "50%");
    dialog->getRenderer()->setTitleBarHeight(30);
    
    // Sample directory selection
    auto dirLabel = tgui::Label::create("Sample Directory:");
    dirLabel->setPosition(10, 40);
    dirLabel->setSize(150, 25);
    dialog->add(dirLabel);
    
    auto dirComboBox = tgui::ComboBox::create();
    dirComboBox->setPosition(10, 70);
    dirComboBox->setSize(200, 25);
    
    // Populate with available sample directories
    auto sampleDirs = getAllSampleDirectories();
    for (const auto& dir : sampleDirs) {
        dirComboBox->addItem(dir);
    }
    if (!sampleDirs.empty()) {
        dirComboBox->setSelectedItem(sampleDirs[0]);
    }
    dialog->add(dirComboBox, "dirComboBox");
    
    // Sample file selection
    auto fileLabel = tgui::Label::create("Sample File:");
    fileLabel->setPosition(10, 110);
    fileLabel->setSize(150, 25);
    dialog->add(fileLabel);
    
    auto fileListBox = tgui::ListBox::create();
    fileListBox->setPosition(10, 140);
    fileListBox->setSize(380, 120);
    dialog->add(fileListBox, "fileListBox");
    
    // Update file list when directory changes
    dirComboBox->onItemSelect([=]() {
        auto selectedDir = dirComboBox->getSelectedItem();
        if (!selectedDir.empty()) {
            fileListBox->removeAllItems();
            auto files = getSampleFiles("samples/" + selectedDir.toStdString());
            for (const auto& file : files) {
                fileListBox->addItem(file);
            }
        }
    });
    
    // Initialize file list with first directory
    if (!sampleDirs.empty()) {
        auto files = getSampleFiles("samples/" + sampleDirs[0]);
        for (const auto& file : files) {
            fileListBox->addItem(file);
        }
    }
    
    // Neuron parameters
    auto thresholdLabel = tgui::Label::create("Threshold:");
    thresholdLabel->setPosition(220, 70);
    thresholdLabel->setSize(80, 25);
    dialog->add(thresholdLabel);
    
    auto thresholdBox = tgui::EditBox::create();
    thresholdBox->setPosition(300, 70);
    thresholdBox->setSize(80, 25);
    thresholdBox->setText("1.0");
    dialog->add(thresholdBox, "thresholdBox");
    
    // Buttons
    auto addButton = tgui::Button::create("Add Neuron");
    addButton->setPosition(10, 280);
    addButton->setSize(100, 30);
    addButton->onPress([=]() {
        auto selectedFile = fileListBox->getSelectedItem();
        auto selectedDir = dirComboBox->getSelectedItem();
        auto thresholdText = thresholdBox->getText();
        
        if (!selectedFile.empty() && !selectedDir.empty()) {
            try {
                float threshold = std::stof(thresholdText.toStdString());
                
                // Create the full file path
                std::string fullPath = "samples/" + selectedDir.toStdString() + "/" + selectedFile.toStdString();
                
                // Get next available sample index
                int sampleIndex = static_cast<int>(network->getNeuronCount()) + 1;
                
                // Load the sample file into AudioManager
                auto audioManager = network->getAudioManager();
                if (audioManager) {
                    if (!audioManager->loadSampleFromPath(sampleIndex, fullPath)) {
                        std::cerr << "Failed to load sample file: " << fullPath << std::endl;
                        return; // Don't create neuron if sample loading failed
                    }
                }
                
                // Add neuron to network with the sample file path
                auto neuron = network->addNeuron(sampleIndex, 0.0f, threshold, 1.0f, 0.0f, ActivationFunction::Linear, fullPath);
                
                std::cout << "Added neuron with sample: " << fullPath << std::endl;
                std::cout << "Sample index: " << sampleIndex << ", Threshold: " << threshold << std::endl;
                std::cout << "Network now has " << network->getNeuronCount() << " neurons" << std::endl;
                
                // Refresh GUI completely
                refreshNeuronSliders();
                refreshConnectionSliders();
                refreshConnectionMatrix();
                updateStatusDisplay();
                
                // Update visualizer layout for new neuron
                if (visualizer) {
                    visualizer->refreshLayout();
                }
                
                // Close add neuron dialog
                dialog->close();
                
            } catch (const std::exception& e) {
                std::cerr << "Error adding neuron: " << e.what() << std::endl;
            }
        }
    });
    dialog->add(addButton);
    
    auto cancelButton = tgui::Button::create("Cancel");
    cancelButton->setPosition(120, 280);
    cancelButton->setSize(100, 30);
    cancelButton->onPress([=]() {
        dialog->close();
    });
    dialog->add(cancelButton);
    
    gui->add(dialog);
}

void GUI::showChangeSampleDialog(size_t neuronIndex) {
    if (!network) return;

    if (neuronIndex >= network->getNeuronCount()) return;

    Neuron* neuron = network->getNeuron(neuronIndex);
    if (!neuron) return;

    // Create dialog window for changing sample
    auto dialog = tgui::ChildWindow::create("Change Neuron Sample");
    dialog->setSize(420, 360);
    dialog->setPosition("50% - 210", "50% - 180");
    dialog->getRenderer()->setTitleBarHeight(30);
    dialog->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 40, 240));

    auto dirLabel = tgui::Label::create("Sample Directory:");
    dirLabel->setPosition(10, 40);
    dirLabel->setSize(150, 25);
    dirLabel->getRenderer()->setTextColor(tgui::Color::White);
    dialog->add(dirLabel);

    auto dirComboBox = tgui::ComboBox::create();
    dirComboBox->setPosition(10, 70);
    dirComboBox->setSize(200, 25);
    dialog->add(dirComboBox, "dirComboBox");

    auto fileLabel = tgui::Label::create("Sample File:");
    fileLabel->setPosition(10, 110);
    fileLabel->setSize(150, 25);
    dialog->add(fileLabel);

    auto fileListBox = tgui::ListBox::create();
    fileListBox->setPosition(10, 140);
    fileListBox->setSize(400, 160);
    dialog->add(fileListBox, "fileListBox");

    // Populate directories
    auto sampleDirs = getAllSampleDirectories();
    for (const auto& d : sampleDirs) dirComboBox->addItem(d);
    if (!sampleDirs.empty()) dirComboBox->setSelectedItem(sampleDirs[0]);

    // Update file list when directory changes
    dirComboBox->onItemSelect([=]() {
        auto selectedDir = dirComboBox->getSelectedItem();
        if (!selectedDir.empty()) {
            fileListBox->removeAllItems();
            auto files = getSampleFiles("samples/" + selectedDir.toStdString());
            for (const auto& file : files) fileListBox->addItem(file);
        }
    });

    // Initialize file list with first directory
    if (!sampleDirs.empty()) {
        auto files = getSampleFiles("samples/" + sampleDirs[0]);
        for (const auto& file : files) fileListBox->addItem(file);
    }

    // Pre-select current file if known
    std::string curPath = neuron->getSampleFilePath();
    if (!curPath.empty()) {
        try {
            auto p = std::filesystem::path(curPath);
            auto parent = p.parent_path().filename().string();
            auto fname = p.filename().string();
            if (!parent.empty()) {
                dirComboBox->setSelectedItem(parent);
                fileListBox->removeAllItems();
                auto files = getSampleFiles("samples/" + parent);
                for (const auto& file : files) fileListBox->addItem(file);
                // Try to pre-select matching file in the list if available.
                // TGUI ListBox selection API varies between versions; leave unselected if method not present.
                (void)fname; // silence unused variable warning when selection is skipped
            }
        } catch (...) {
            // ignore
        }
    }

    // Buttons
    auto changeButton = tgui::Button::create("Change Sample");
    changeButton->setPosition(10, 310);
    changeButton->setSize(140, 30);
    changeButton->onPress([=]() {
        auto selectedDir = dirComboBox->getSelectedItem();
        auto selectedFile = fileListBox->getSelectedItem();

        if (selectedDir.empty() || selectedFile.empty()) return;

        std::string fullPath = "samples/" + selectedDir.toStdString() + "/" + selectedFile.toStdString();

        if (!audioManager) {
            std::cerr << "No AudioManager available to load sample" << std::endl;
            dialog->close();
            return;
        }

        int assignedIndex = neuron->getSampleIndex();
        if (assignedIndex <= 0) {
            // Provide a default mapping if neuron doesn't have an index
            assignedIndex = static_cast<int>(neuronIndex) + 1;
            neuron->setSampleIndex(assignedIndex);
        }

        if (!audioManager->loadSampleFromPath(assignedIndex, fullPath)) {
            std::cerr << "Failed to load sample: " << fullPath << std::endl;
            // Show small error dialog
            auto err = tgui::ChildWindow::create("Error");
            err->setSize(300, 120);
            err->setPosition("50% - 150", "50% - 60");
            auto msg = tgui::Label::create("Failed to load sample file: " + selectedFile.toStdString());
            msg->setPosition(10, 20);
            msg->setSize(280, 60);
            err->add(msg);
            auto ok = tgui::Button::create("OK");
            ok->setPosition(100, 70);
            ok->setSize(100, 30);
            ok->onPress([err]() { err->close(); });
            err->add(ok);
            gui->add(err);
            return;
        }

        // Update neuron's sample file path
        neuron->setSampleFilePath(fullPath);

        // Update sample button text to show filename
        if (neuronIndex < neuronSampleButtons.size()) {
            try {
                std::string label = std::filesystem::path(fullPath).filename().string();
                // Truncate long filenames to fit the small button
                const size_t maxLabelLen = 12;
                std::string display = label;
                if (display.size() > maxLabelLen) display = display.substr(0, maxLabelLen - 3) + "...";
                neuronSampleButtons[neuronIndex]->setText(display);
            } catch (...) {
                std::string display = selectedFile.toStdString();
                const size_t maxLabelLen = 12;
                if (display.size() > maxLabelLen) display = display.substr(0, maxLabelLen - 3) + "...";
                neuronSampleButtons[neuronIndex]->setText(display);
            }
        }

        std::cout << "Neuron " << neuronIndex << " sample changed to " << fullPath << std::endl;

        dialog->close();
    });
    dialog->add(changeButton);

    auto cancelButton = tgui::Button::create("Cancel");
    cancelButton->setPosition(160, 310);
    cancelButton->setSize(100, 30);
    cancelButton->onPress([=]() { dialog->close(); });
    dialog->add(cancelButton);

    gui->add(dialog);
}

std::vector<std::string> GUI::getSampleFiles(const std::string& directory) {
    std::vector<std::string> files;
    
    try {
        if (std::filesystem::exists(directory) && std::filesystem::is_directory(directory)) {
            for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    auto filename = entry.path().filename().string();
                    // Check for audio file extensions
                    auto ext = entry.path().extension().string();
                    if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac") {
                        files.push_back(filename);
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading directory " << directory << ": " << e.what() << std::endl;
    }
    
    return files;
}

std::vector<std::string> GUI::getAllSampleDirectories() {
    std::vector<std::string> directories;
    
    try {
        std::string samplesPath = "samples";
        if (std::filesystem::exists(samplesPath) && std::filesystem::is_directory(samplesPath)) {
            for (const auto& entry : std::filesystem::directory_iterator(samplesPath)) {
                if (entry.is_directory()) {
                    directories.push_back(entry.path().filename().string());
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading samples directory: " << e.what() << std::endl;
    }
    
    return directories;
}

void GUI::startRecording() {
    // For backward compatibility, default to internal recording
    startInternalRecording();
}

void GUI::startInternalRecording() {
    showInternalRecordingDialog();
}

void GUI::startExternalRecording() {
    showExternalRecordingDialog();
}

void GUI::stopRecording() {
    if (!recorder) {
        std::cerr << "No recorder available" << std::endl;
        return;
    }
    
    // Stop internal recording if active
    if (audioManager && audioManager->isRecordingOutput()) {
        audioManager->stopInternalRecording();
    }
    
    if (recorder->isCurrentlyRecording()) {
        recorder->stopRecording();
        
        // Show success dialog
        auto dialog = tgui::ChildWindow::create("Recording Stopped");
        dialog->setSize(300, 150);
        dialog->setPosition("50%", "50%");
        
        auto message = tgui::Label::create("Recording stopped and saved successfully!\n\nDuration: " + 
                                          std::to_string(recorder->getRecordingDuration()) + " seconds");
        message->setPosition(10, 40);
        message->setSize(280, 60);
        message->getRenderer()->setTextColor(tgui::Color::White);
        dialog->add(message);
        
        auto okButton = tgui::Button::create("OK");
        okButton->setPosition(100, 110);
        okButton->setSize(100, 30);
        okButton->onPress([=]() {
            dialog->close();
        });
        dialog->add(okButton);
        
        gui->add(dialog);
    } else {
        // Show message that recording is not active
        auto dialog = tgui::ChildWindow::create("Not Recording");
        dialog->setSize(300, 120);
        dialog->setPosition("50%", "50%");
        
        auto message = tgui::Label::create("No recording is currently active.");
        message->setPosition(10, 40);
        message->setSize(280, 30);
        message->getRenderer()->setTextColor(tgui::Color::White);
        dialog->add(message);
        
        auto okButton = tgui::Button::create("OK");
        okButton->setPosition(100, 80);
        okButton->setSize(100, 30);
        okButton->onPress([=]() {
            dialog->close();
        });
        dialog->add(okButton);
        
        gui->add(dialog);
    }
}

void GUI::showRecordingDialog() {
    if (!recorder) {
        std::cerr << "No recorder available" << std::endl;
        return;
    }
    
    // Check if already recording
    if (recorder->isCurrentlyRecording()) {
        // Show message that recording is already active
        auto dialog = tgui::ChildWindow::create("Already Recording");
        dialog->setSize(300, 150);
        dialog->setPosition("50%", "50%");
        
        auto message = tgui::Label::create("Recording is already in progress.\n\nUse 'Stop Recording' to end the current session.");
        message->setPosition(10, 40);
        message->setSize(280, 60);
        message->getRenderer()->setTextColor(tgui::Color::White);
        dialog->add(message);
        
        auto okButton = tgui::Button::create("OK");
        okButton->setPosition(100, 110);
        okButton->setSize(100, 30);
        okButton->onPress([=]() {
            dialog->close();
        });
        dialog->add(okButton);
        
        gui->add(dialog);
        return;
    }
    
    // Create recording dialog
    auto dialog = tgui::ChildWindow::create("Start Recording");
    dialog->setSize(400, 200);
    dialog->setPosition("50%", "50%");
    dialog->getRenderer()->setTitleBarHeight(30);
    
    // Filename input
    auto filenameLabel = tgui::Label::create("Filename:");
    filenameLabel->setPosition(10, 40);
    filenameLabel->setSize(100, 25);
    dialog->add(filenameLabel);
    
    auto filenameBox = tgui::EditBox::create();
    filenameBox->setPosition(10, 70);
    filenameBox->setSize(300, 25);
    
    // Generate default filename with timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    std::stringstream ss;
    ss << "recording_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".wav";
    filenameBox->setText(ss.str());
    
    dialog->add(filenameBox, "filenameBox");
    
    // Instructions
    auto instructions = tgui::Label::create("Enter filename for the recording.\nThe file will be saved in the current directory.");
    instructions->setPosition(10, 110);
    instructions->setSize(380, 40);
    instructions->getRenderer()->setTextColor(tgui::Color(200, 200, 200));
    instructions->setTextSize(11);
    dialog->add(instructions);
    
    // Buttons
    auto startButton = tgui::Button::create("Start Recording");
    startButton->setPosition(10, 160);
    startButton->setSize(120, 30);
    startButton->onPress([=]() {
        auto filename = filenameBox->getText().toStdString();
        
        if (filename.empty()) {
            std::cerr << "Filename cannot be empty" << std::endl;
            return;
        }
        
        // Ensure .wav extension
        if (filename.length() < 4 || filename.substr(filename.length() - 4) != ".wav") {
            filename += ".wav";
        }
        
        if (recorder->startRecording(filename)) {
            std::cout << "Started recording to: " << filename << std::endl;
            
            // Show recording status dialog
            auto statusDialog = tgui::ChildWindow::create("Recording Active");
            statusDialog->setSize(300, 150);
            statusDialog->setPosition("50%", "50%");
            
            auto statusMessage = tgui::Label::create("Recording in progress...\n\nFile: " + filename + 
                                                   "\n\nUse 'Stop Recording' menu to end.");
            statusMessage->setPosition(10, 40);
            statusMessage->setSize(280, 70);
            statusMessage->getRenderer()->setTextColor(tgui::Color::White);
            statusDialog->add(statusMessage);
            
            auto closeButton = tgui::Button::create("OK");
            closeButton->setPosition(100, 115);
            closeButton->setSize(100, 30);
            closeButton->onPress([=]() {
                statusDialog->close();
            });
            statusDialog->add(closeButton);
            
            gui->add(statusDialog);
            
        } else {
            std::cerr << "Failed to start recording" << std::endl;
            
            // Show error dialog
            auto errorDialog = tgui::ChildWindow::create("Recording Error");
            errorDialog->setSize(300, 150);
            errorDialog->setPosition("50%", "50%");
            
            auto errorMessage = tgui::Label::create("Failed to start recording.\n\nPlease check that a microphone is available.");
            errorMessage->setPosition(10, 40);
            errorMessage->setSize(280, 60);
            errorMessage->getRenderer()->setTextColor(tgui::Color::Red);
            errorDialog->add(errorMessage);
            
            auto okButton = tgui::Button::create("OK");
            okButton->setPosition(100, 110);
            okButton->setSize(100, 30);
            okButton->onPress([=]() {
                errorDialog->close();
            });
            errorDialog->add(okButton);
            
            gui->add(errorDialog);
        }
        
        dialog->close();
    });
    dialog->add(startButton);
    
    auto cancelButton = tgui::Button::create("Cancel");
    cancelButton->setPosition(140, 160);
    cancelButton->setSize(100, 30);
    cancelButton->onPress([=]() {
        dialog->close();
    });
    dialog->add(cancelButton);
    
    gui->add(dialog);
}

void GUI::showInternalRecordingDialog() {
    if (!recorder || !audioManager) {
        std::cerr << "Recorder or AudioManager not available" << std::endl;
        return;
    }
    
    // Check if already recording
    if (recorder->isCurrentlyRecording()) {
        // Show message that recording is already active
        auto dialog = tgui::ChildWindow::create("Already Recording");
        dialog->setSize(300, 150);
        dialog->setPosition("50%", "50%");
        
        auto message = tgui::Label::create("Recording is already in progress.\n\nUse 'Stop Recording' to end the current session.");
        message->setPosition(10, 40);
        message->setSize(280, 60);
        message->getRenderer()->setTextColor(tgui::Color::White);
        dialog->add(message);
        
        auto okButton = tgui::Button::create("OK");
        okButton->setPosition(100, 110);
        okButton->setSize(100, 30);
        okButton->onPress([=]() {
            dialog->close();
        });
        dialog->add(okButton);
        
        gui->add(dialog);
        return;
    }
    
    // Create internal recording dialog
    auto dialog = tgui::ChildWindow::create("Record NeuronSeq Output");
    dialog->setSize(450, 250);
    dialog->setPosition("50%", "50%");
    dialog->getRenderer()->setTitleBarHeight(30);
    
    // Description
    auto description = tgui::Label::create("This will record the audio output from NeuronSeqSampler\n(the samples being played by neurons).");
    description->setPosition(10, 40);
    description->setSize(430, 40);
    description->getRenderer()->setTextColor(tgui::Color(180, 180, 255));
    description->setTextSize(12);
    dialog->add(description);
    
    // Filename input
    auto filenameLabel = tgui::Label::create("Filename:");
    filenameLabel->setPosition(10, 90);
    filenameLabel->setSize(100, 25);
    dialog->add(filenameLabel);
    
    auto filenameBox = tgui::EditBox::create();
    filenameBox->setPosition(10, 120);
    filenameBox->setSize(350, 25);
    
    // Generate default filename with timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    std::stringstream ss;
    ss << "neuronseq_output_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".wav";
    filenameBox->setText(ss.str());
    
    dialog->add(filenameBox, "filenameBox");
    
    // Instructions
    auto instructions = tgui::Label::create("Play some neurons after starting the recording to capture their output.\nThe recording will capture each sample as it's triggered.");
    instructions->setPosition(10, 160);
    instructions->setSize(430, 30);
    instructions->getRenderer()->setTextColor(tgui::Color(200, 200, 200));
    instructions->setTextSize(11);
    dialog->add(instructions);
    
    // Buttons
    auto startButton = tgui::Button::create("Start Recording");
    startButton->setPosition(10, 205);
    startButton->setSize(140, 30);
    startButton->onPress([=]() {
        auto filename = filenameBox->getText().toStdString();
        
        if (filename.empty()) {
            std::cerr << "Filename cannot be empty" << std::endl;
            return;
        }
        
        // Ensure .wav extension
        if (filename.length() < 4 || filename.substr(filename.length() - 4) != ".wav") {
            filename += ".wav";
        }
        
        if (recorder->startInternalRecording(filename)) {
            audioManager->startInternalRecording();
            std::cout << "Started internal recording to: " << filename << std::endl;
            
            // Show recording status dialog
            auto statusDialog = tgui::ChildWindow::create("Internal Recording Active");
            statusDialog->setSize(350, 180);
            statusDialog->setPosition("50%", "50%");
            
            auto statusMessage = tgui::Label::create("Recording NeuronSeq output...\n\nFile: " + filename + 
                                                   "\n\nTrigger neurons to capture their samples!\nUse 'Stop Recording' menu to end.");
            statusMessage->setPosition(10, 40);
            statusMessage->setSize(330, 100);
            statusMessage->getRenderer()->setTextColor(tgui::Color::White);
            statusDialog->add(statusMessage);
            
            auto closeButton = tgui::Button::create("OK");
            closeButton->setPosition(125, 145);
            closeButton->setSize(100, 30);
            closeButton->onPress([=]() {
                statusDialog->close();
            });
            statusDialog->add(closeButton);
            
            gui->add(statusDialog);
            
        } else {
            std::cerr << "Failed to start internal recording" << std::endl;
        }
        
        dialog->close();
    });
    dialog->add(startButton);
    
    auto cancelButton = tgui::Button::create("Cancel");
    cancelButton->setPosition(160, 205);
    cancelButton->setSize(100, 30);
    cancelButton->onPress([=]() {
        dialog->close();
    });
    dialog->add(cancelButton);
    
    gui->add(dialog);
}

void GUI::showExternalRecordingDialog() {
    // This is essentially the same as the original showRecordingDialog
    // but with clarification that it's for external microphone
    if (!recorder) {
        std::cerr << "No recorder available" << std::endl;
        return;
    }
    
    // Check if already recording
    if (recorder->isCurrentlyRecording()) {
        // Show message that recording is already active
        auto dialog = tgui::ChildWindow::create("Already Recording");
        dialog->setSize(300, 150);
        dialog->setPosition("50%", "50%");
        
        auto message = tgui::Label::create("Recording is already in progress.\n\nUse 'Stop Recording' to end the current session.");
        message->setPosition(10, 40);
        message->setSize(280, 60);
        message->getRenderer()->setTextColor(tgui::Color::White);
        dialog->add(message);
        
        auto okButton = tgui::Button::create("OK");
        okButton->setPosition(100, 110);
        okButton->setSize(100, 30);
        okButton->onPress([=]() {
            dialog->close();
        });
        dialog->add(okButton);
        
        gui->add(dialog);
        return;
    }
    
    // Create external recording dialog
    auto dialog = tgui::ChildWindow::create("Record External Microphone");
    dialog->setSize(450, 280);
    dialog->setPosition("50%", "50%");
    dialog->getRenderer()->setTitleBarHeight(30);
    
    // Description
    auto description = tgui::Label::create("This will record from your system's microphone input.\nNote: JACK must be stopped for microphone access to work.");
    description->setPosition(10, 40);
    description->setSize(430, 40);
    description->getRenderer()->setTextColor(tgui::Color(255, 180, 180));
    description->setTextSize(12);
    dialog->add(description);
    
    // Filename input
    auto filenameLabel = tgui::Label::create("Filename:");
    filenameLabel->setPosition(10, 90);
    filenameLabel->setSize(100, 25);
    dialog->add(filenameLabel);
    
    auto filenameBox = tgui::EditBox::create();
    filenameBox->setPosition(10, 120);
    filenameBox->setSize(350, 25);
    
    // Generate default filename with timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    std::stringstream ss;
    ss << "microphone_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".wav";
    filenameBox->setText(ss.str());
    
    dialog->add(filenameBox, "filenameBox");
    
    // Instructions
    auto instructions = tgui::Label::create("Requirements:\n• Run './manage_jack.sh stop' before recording\n• Run './optimize_mic.sh' for best quality\n• Ensure microphone is connected and enabled");
    instructions->setPosition(10, 160);
    instructions->setSize(430, 60);
    instructions->getRenderer()->setTextColor(tgui::Color(200, 200, 200));
    instructions->setTextSize(11);
    dialog->add(instructions);
    
    // Buttons
    auto startButton = tgui::Button::create("Start Recording");
    startButton->setPosition(10, 235);
    startButton->setSize(140, 30);
    startButton->onPress([=]() {
        auto filename = filenameBox->getText().toStdString();
        
        if (filename.empty()) {
            std::cerr << "Filename cannot be empty" << std::endl;
            return;
        }
        
        // Ensure .wav extension
        if (filename.length() < 4 || filename.substr(filename.length() - 4) != ".wav") {
            filename += ".wav";
        }
        
        if (recorder->startRecording(filename)) {
            std::cout << "Started external recording to: " << filename << std::endl;
            
            // Show recording status dialog
            auto statusDialog = tgui::ChildWindow::create("External Recording Active");
            statusDialog->setSize(350, 150);
            statusDialog->setPosition("50%", "50%");
            
            auto statusMessage = tgui::Label::create("Recording from microphone...\n\nFile: " + filename + 
                                                   "\n\nUse 'Stop Recording' menu to end.");
            statusMessage->setPosition(10, 40);
            statusMessage->setSize(330, 70);
            statusMessage->getRenderer()->setTextColor(tgui::Color::White);
            statusDialog->add(statusMessage);
            
            auto closeButton = tgui::Button::create("OK");
            closeButton->setPosition(125, 115);
            closeButton->setSize(100, 30);
            closeButton->onPress([=]() {
                statusDialog->close();
            });
            statusDialog->add(closeButton);
            
            gui->add(statusDialog);
            
        } else {
            std::cerr << "Failed to start external recording" << std::endl;
            
            // Show error dialog with specific troubleshooting
            auto errorDialog = tgui::ChildWindow::create("Recording Error");
            errorDialog->setSize(400, 200);
            errorDialog->setPosition("50%", "50%");
            
            auto errorMessage = tgui::Label::create("Failed to start microphone recording.\n\nTroubleshooting:\n• Stop JACK: ./manage_jack.sh stop\n• Optimize audio: ./optimize_mic.sh\n• Check microphone connection");
            errorMessage->setPosition(10, 40);
            errorMessage->setSize(380, 120);
            errorMessage->getRenderer()->setTextColor(tgui::Color::Red);
            errorMessage->setTextSize(11);
            errorDialog->add(errorMessage);
            
            auto okButton = tgui::Button::create("OK");
            okButton->setPosition(150, 165);
            okButton->setSize(100, 30);
            okButton->onPress([=]() {
                errorDialog->close();
            });
            errorDialog->add(okButton);
            
            gui->add(errorDialog);
        }
        
        dialog->close();
    });
    dialog->add(startButton);
    
    auto cancelButton = tgui::Button::create("Cancel");
    cancelButton->setPosition(160, 235);
    cancelButton->setSize(100, 30);
    cancelButton->onPress([=]() {
        dialog->close();
    });
    dialog->add(cancelButton);
    
    gui->add(dialog);
}
void GUI::createConnectionMatrixPanel() {
    // Remove existing window if it exists
    if (connectionMatrixWindow) {
        // Preserve current visibility state before removing window
        matrixVisible = connectionMatrixWindow->isVisible();
        gui->remove(connectionMatrixWindow);
        connectionMatrixWindow = nullptr;
        connectionMatrixPanel = nullptr;
        matrixToggleButtons.clear();
        matrixGainSliders.clear();
        filterLabels.clear();
        neuronColumnLabels.clear();
        rhythmogramScaleSlider = nullptr;
        rhythmogramScaleLabel = nullptr;
        bpmSlider = nullptr;
        bpmLabel = nullptr;
        autodetectTempoToggle = nullptr;
        detectedTempoLabel = nullptr;
        // Learning controls
        learningToggle = nullptr;
        learningRateSlider = nullptr;
        learningRateLabel = nullptr;
        weightDecaySlider = nullptr;
        weightDecayLabel = nullptr;
        resetRhythmWeightsButton = nullptr;
        // BeatRoot controls
        beatRootToggle = nullptr;
        beatRootStatusLabel = nullptr;
        beatRootOnsetThresholdSlider = nullptr;
        beatRootOnsetThresholdLabel = nullptr;
        beatRootBeatToleranceSlider = nullptr;
        beatRootBeatToleranceLabel = nullptr;
        beatRootMaxAgentsSlider = nullptr;
        beatRootMaxAgentsLabel = nullptr;
        beatRootAutoInitToggle = nullptr;
        beatRootResetButton = nullptr;
        beatRootInitButton = nullptr;
    }
    // Always use the current value of matrixVisible when creating the panel, even if it did not exist before
    
    if (!network || !network->getRhythmInterpreter()) {
        return; // No rhythm interpreter available
    }
    
    auto rhythmInterpreter = network->getRhythmInterpreter();
    size_t numFilters = rhythmInterpreter->getBandCount();
    size_t numConnections = network->getConnectionCount();
    
    // Create panel even if no neurons yet - it will show as empty but ready
    
    // Create a popover ChildWindow to host the rhythmogram (non-transparent, centered)
    connectionMatrixWindow = tgui::ChildWindow::create("Rhythmogram Mapping");
    connectionMatrixWindow->setSize("90%", "75%");
    connectionMatrixWindow->setPosition("50% - 45%", "50% - 37.5%");
    // Make the window semi-transparent so the neuron visualization is visible beneath
    connectionMatrixWindow->getRenderer()->setBackgroundColor(tgui::Color(20, 20, 20, 200));
    connectionMatrixWindow->getRenderer()->setBorderColor(tgui::Color(80, 80, 80));
    connectionMatrixWindow->getRenderer()->setBorders(2);
    connectionMatrixWindow->setVisible(matrixVisible);
    connectionMatrixWindow->setTitleButtons(tgui::ChildWindow::TitleButton::None); // Remove close button

    // Inner scrollable panel - all existing matrix widgets will be added to this panel
    connectionMatrixPanel = tgui::ScrollablePanel::create();
    connectionMatrixPanel->setPosition("0%", "0%");
    connectionMatrixPanel->setSize("100%", "100%");
    // Make inner panel transparent to allow underlying visualization to show through
    connectionMatrixPanel->getRenderer()->setBackgroundColor(tgui::Color(0,0,0,0));
    connectionMatrixWindow->add(connectionMatrixPanel);

    gui->add(connectionMatrixWindow, "ConnectionMatrixPanel");

    // Compute content size estimates (used for laying out controls inside the popover)
    float contentWidth = std::max(350.0f, static_cast<float>(270 + numConnections * 80 + 280)); // estimate
    float contentHeight = std::max(550.0f, static_cast<float>(150 + numFilters * 60 + 150)); // estimate
    
    // Set content size for scrollable panel now that we have computed the dimensions
    connectionMatrixPanel->setContentSize(tgui::Vector2f(contentWidth, contentHeight));
    
    // Title label
    std::string title = numConnections == 0 ? "🎛️ Rhythmogram Mapping (" + std::to_string(numFilters) + "×0) - Add connections first" : 
                                             "🎛️ Rhythmogram Mapping (" + std::to_string(numFilters) + "×" + std::to_string(numConnections) + ")";
    matrixTitleLabel = tgui::Label::create(title);
    matrixTitleLabel->setPosition(5, 5);
    matrixTitleLabel->setTextSize(14);
    matrixTitleLabel->getRenderer()->setTextColor(tgui::Color::White);
    connectionMatrixPanel->add(matrixTitleLabel);
    
    // Add quick action buttons
    auto clearAllButton = tgui::Button::create("Clear All");
    clearAllButton->setPosition(5, matrixTitleLabel->getPosition().y + 20);
    clearAllButton->setSize(70, 20);
    clearAllButton->setTextSize(10);
    clearAllButton->getRenderer()->setBackgroundColor(tgui::Color(80, 40, 40));
    clearAllButton->onPress([this]() {
        if (!network) return;
        
        // Clear all rhythm connections
        for (size_t f = 0; f < matrixToggleButtons.size(); ++f) {
            for (size_t c = 0; c < matrixToggleButtons[f].size(); ++c) {
                network->clearRhythmConnection(f, c);
                
                // Update button appearance
                matrixToggleButtons[f][c]->setText("○");
                matrixToggleButtons[f][c]->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 40));
                matrixToggleButtons[f][c]->getRenderer()->setTextColor(tgui::Color(100, 100, 100));
                
                // Hide gain controls
                if (f < matrixGainSliders.size() && c < matrixGainSliders[f].size()) {
                    matrixGainSliders[f][c]->setValue(30.0f);
                    matrixGainSliders[f][c]->setVisible(false);
                }
                if (f < matrixGainDisplays.size() && c < matrixGainDisplays[f].size()) {
                    matrixGainDisplays[f][c]->setVisible(false);
                }
            }
        }
        std::cout << "🔄 Cleared all rhythm connections" << std::endl;
    });
    connectionMatrixPanel->add(clearAllButton);
    
    auto randomizeButton = tgui::Button::create("Random");
    randomizeButton->setPosition(80, matrixTitleLabel->getPosition().y + 20);
    randomizeButton->setSize(70, 20);
    randomizeButton->setTextSize(10);
    randomizeButton->getRenderer()->setBackgroundColor(tgui::Color(40, 80, 40));
    randomizeButton->onPress([this]() {
        if (!network) return;
        
        // Randomize rhythm connections (30% chance per connection)
        std::srand(static_cast<unsigned>(std::time(nullptr))); // Seed random number generator
        
        for (size_t f = 0; f < matrixToggleButtons.size(); ++f) {
            for (size_t c = 0; c < matrixToggleButtons[f].size(); ++c) {
                bool shouldConnect = (std::rand() % 100) < 30; // 30% chance
                
                if (shouldConnect) {
                    // Create random connection with random gain (0.1 to 0.8)
                    float randomGain = 0.1f + (static_cast<float>(std::rand()) / RAND_MAX) * 0.7f;
                    network->setRhythmConnection(f, c, randomGain);
                    
                    // Update button appearance
                    matrixToggleButtons[f][c]->setText("●");
                    matrixToggleButtons[f][c]->getRenderer()->setBackgroundColor(tgui::Color(60, 120, 60));
                    matrixToggleButtons[f][c]->getRenderer()->setTextColor(tgui::Color::White);
                    
                    // Show and set gain controls
                    if (f < matrixGainSliders.size() && c < matrixGainSliders[f].size()) {
                        matrixGainSliders[f][c]->setValue(randomGain * 100.0f);
                        matrixGainSliders[f][c]->setVisible(true);
                    }
                    if (f < matrixGainDisplays.size() && c < matrixGainDisplays[f].size()) {
                        matrixGainDisplays[f][c]->setText(std::to_string(static_cast<int>(randomGain * 100)));
                        matrixGainDisplays[f][c]->setVisible(true);
                    }
                } else {
                    // Clear connection
                    network->clearRhythmConnection(f, c);
                    
                    // Update button appearance
                    matrixToggleButtons[f][c]->setText("○");
                    matrixToggleButtons[f][c]->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 40));
                    matrixToggleButtons[f][c]->getRenderer()->setTextColor(tgui::Color(100, 100, 100));
                    
                    // Hide gain controls
                    if (f < matrixGainSliders.size() && c < matrixGainSliders[f].size()) {
                        matrixGainSliders[f][c]->setValue(30.0f);
                        matrixGainSliders[f][c]->setVisible(false);
                    }
                    if (f < matrixGainDisplays.size() && c < matrixGainDisplays[f].size()) {
                        matrixGainDisplays[f][c]->setVisible(false);
                    }
                }
            }
        }
        std::cout << "🎲 Randomized rhythm connections (30% density)" << std::endl;
    });
    connectionMatrixPanel->add(randomizeButton);
    
    auto connectAllButton = tgui::Button::create("Connect All");
    connectAllButton->setPosition(155, matrixTitleLabel->getPosition().y + 20);
    connectAllButton->setSize(70, 20);
    connectAllButton->setTextSize(10);
    connectAllButton->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 80));
    connectAllButton->onPress([this]() {
        if (!network) return;
        
        // Connect all rhythm connections with default gain
        float defaultGain = 0.3f;
        
        for (size_t f = 0; f < matrixToggleButtons.size(); ++f) {
            for (size_t c = 0; c < matrixToggleButtons[f].size(); ++c) {
                network->setRhythmConnection(f, c, defaultGain);
                
                // Update button appearance
                matrixToggleButtons[f][c]->setText("●");
                matrixToggleButtons[f][c]->getRenderer()->setBackgroundColor(tgui::Color(60, 120, 60));
                matrixToggleButtons[f][c]->getRenderer()->setTextColor(tgui::Color::White);
                
                // Show and set gain controls
                if (f < matrixGainSliders.size() && c < matrixGainSliders[f].size()) {
                    matrixGainSliders[f][c]->setValue(defaultGain * 100.0f);
                    matrixGainSliders[f][c]->setVisible(true);
                }
                if (f < matrixGainDisplays.size() && c < matrixGainDisplays[f].size()) {
                    matrixGainDisplays[f][c]->setText(std::to_string(static_cast<int>(defaultGain * 100)));
                    matrixGainDisplays[f][c]->setVisible(true);
                }
            }
        }
        std::cout << "🔗 Connected all rhythm connections (30% gain)" << std::endl;
    });
    connectionMatrixPanel->add(connectAllButton);
    
    // Rhythmogram frequency bands (Todd, 1994) - Logarithmic distribution for rhythmic hierarchy
    // Dynamic band names and tooltips
    std::vector<std::string> filterNames(numFilters);
    std::vector<std::string> filterTooltips(numFilters);
    auto freqs = rhythmInterpreter->getBandFrequencies();
    for (size_t i = 0; i < numFilters; ++i) {
        float f = (i < freqs.size()) ? freqs[i] : 0.0f;
        // Format frequency display
        std::ostringstream fs;
        if (f < 1.0f) {
            fs << std::fixed << std::setprecision(3) << f;
        } else {
            fs << std::fixed << std::setprecision(1) << f;
        }
        filterNames[i] = "Band " + std::to_string(i+1) + " (" + fs.str() + "Hz)";
        filterTooltips[i] = "Rhythm band " + std::to_string(i+1) + " at ~" + fs.str() + " Hz";
    }
    
    filterLabels.clear();
    filterGainSliders.clear();
    filterGainDisplays.clear();
    filterOutputDisplays.clear();
    for (size_t f = 0; f < numFilters; ++f) {
        // Reverse vertical order: Onset at top (f=7 -> y=90), Phrase at bottom (f=0 -> y=90+420)
        size_t displayRow = (numFilters - 1) - f;
        
        auto label = tgui::Label::create(filterNames[f]);
        label->setPosition(5, 90 + displayRow * 60); // Reversed position: Onset top, Phrase bottom
        label->setTextSize(10);
        label->getRenderer()->setTextColor(tgui::Color(180, 180, 180));
        
        // Tooltip available via hover (handled elsewhere)
        
        connectionMatrixPanel->add(label);
        filterLabels.push_back(label);
        
        // Add filter gain slider next to each frequency label
        auto gainSlider = tgui::Slider::create(0.0f, 5.0f);
        gainSlider->setValue(1.0f); // Default gain for minimal RhythmInterpreter
        gainSlider->setStep(0.1f); // 0.1x step increments
        gainSlider->setPosition(5, 105 + displayRow * 60); // Just below the label, reversed position
        gainSlider->setSize(65, 15); // Small horizontal slider
        gainSlider->getRenderer()->setTrackColor(tgui::Color(60, 60, 60));
        gainSlider->getRenderer()->setThumbColor(tgui::Color(100, 140, 100));
        // Filter gain control from 0.0x to 5.0x with 0.1x steps
        
        // Connect slider to filter gain control
        gainSlider->onValueChange([this, f](float value) {
            if (network && network->getRhythmInterpreter()) {
                network->getRhythmInterpreter()->setFilterGain(f, value);
            }
            // Update filter gain display with proper formatting (one decimal place)
            std::ostringstream stream;
            stream << std::fixed << std::setprecision(1) << value << "x";
            filterGainDisplays[f]->setText(stream.str());
        });
        
        connectionMatrixPanel->add(gainSlider);
        filterGainSliders.push_back(gainSlider);
        
        // Add filter gain value display (shows current gain setting)
        auto gainDisplay = tgui::Label::create("1.0x");
        gainDisplay->setPosition(75, 105 + displayRow * 60); // Right of the gain slider, reversed position
        gainDisplay->setSize(25, 15); // Small label
        gainDisplay->setTextSize(8);
        gainDisplay->getRenderer()->setTextColor(tgui::Color(140, 140, 200));
        gainDisplay->getRenderer()->setBackgroundColor(tgui::Color(15, 15, 25));
        gainDisplay->getRenderer()->setBorderColor(tgui::Color(40, 40, 60));
        gainDisplay->getRenderer()->setBorders(1);
        
        connectionMatrixPanel->add(gainDisplay);
        filterGainDisplays.push_back(gainDisplay);
        
        // Add filter output display (label showing current output level)
        auto outputDisplay = tgui::Label::create("0.0");
        outputDisplay->setPosition(105, 95 + displayRow * 60); // Per decamille displays, reversed position
        outputDisplay->setSize(40, 15); // Small label
        outputDisplay->setTextSize(8);
        outputDisplay->getRenderer()->setTextColor(tgui::Color(100, 200, 100));
        outputDisplay->getRenderer()->setBackgroundColor(tgui::Color(20, 20, 20));
        outputDisplay->getRenderer()->setBorderColor(tgui::Color(60, 60, 60));
        outputDisplay->getRenderer()->setBorders(1);
        
        connectionMatrixPanel->add(outputDisplay);
        filterOutputDisplays.push_back(outputDisplay);
        

    }
    
    // Neuron column labels (horizontal) - only if we have neurons
    neuronColumnLabels.clear();
    if (numConnections > 0) {
        for (size_t c = 0; c < numConnections; ++c) {
            auto label = tgui::Label::create("C" + std::to_string(c + 1));
            label->setPosition(150 + c * 80, 40); // Increased spacing to 80px for wider gaps between connections
            label->setTextSize(10);
            label->getRenderer()->setTextColor(tgui::Color(180, 180, 180));
            connectionMatrixPanel->add(label);
            neuronColumnLabels.push_back(label);

            // Removed redundant band selection widget
        }
    }
    
    // Create matrix of toggle buttons and gain sliders - only if we have neurons
    matrixToggleButtons.clear();
    matrixGainSliders.clear();
    matrixGainDisplays.clear();
    
    if (numConnections > 0) {
        for (size_t f = 0; f < numFilters; ++f) {
            // Reverse vertical order: Onset at top (f=7 -> y=90), Phrase at bottom (f=0 -> y=90+420)
            size_t displayRow = (numFilters - 1) - f;
            
            std::vector<tgui::Button::Ptr> buttonRow;
            std::vector<tgui::Slider::Ptr> sliderRow;
            std::vector<tgui::Label::Ptr> displayRowVec;
            
            for (size_t c = 0; c < numConnections; ++c) {
            // Toggle button
            auto toggleButton = tgui::Button::create("○");
            toggleButton->setPosition(150 + c * 80, 90 + displayRow * 60); // Aligned with connection column labels, reversed position
            toggleButton->setSize(20, 20);
            toggleButton->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 40));
            toggleButton->getRenderer()->setTextColor(tgui::Color(100, 100, 100));
            toggleButton->getRenderer()->setBorderColor(tgui::Color(80, 80, 80));
            toggleButton->getRenderer()->setBorders(1);
            
            // Add tooltip with filter description
            auto tooltip = tgui::Label::create(filterTooltips[f] + "\n→ Connection " + std::to_string(c + 1));
            tooltip->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 200));
            tooltip->getRenderer()->setTextColor(tgui::Color::White);
            tooltip->setTextSize(10);
            toggleButton->setToolTip(tooltip);
            
            // Decouple initial UI state from current weights: always start disconnected
            float currentWeight = network->getRhythmConnection(f, c);
            
            // Toggle connection callback
            toggleButton->onPress([this, f, c, toggleButton]() {
                if (!network) return;
                
                // Get current connection state
                float currentWeight = network->getRhythmConnection(f, c);
                bool wasConnected = std::abs(currentWeight) > 0.001f;
                
                if (wasConnected) {
                    // Deactivate: Clear connection and reset gain
                    network->clearRhythmConnection(f, c);
                    toggleButton->setText("○");
                    toggleButton->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 40));
                    toggleButton->getRenderer()->setTextColor(tgui::Color(100, 100, 100));
                    // Reset gain slider to default value and hide it
                    matrixGainSliders[f][c]->setValue(30.0f); // Reset to default 30% (0.3 * 100)
                    matrixGainSliders[f][c]->setVisible(false);
                    // Hide connection gain display
                    matrixGainDisplays[f][c]->setVisible(false);
                } else {
                    // Connect with default gain
                    float defaultGain = 0.3f;
                    network->setRhythmConnection(f, c, defaultGain);
                    toggleButton->setText("●");
                    toggleButton->getRenderer()->setBackgroundColor(tgui::Color(60, 120, 60));
                    toggleButton->getRenderer()->setTextColor(tgui::Color::White);
                    matrixGainSliders[f][c]->setValue(defaultGain * 100.0f); // Convert to 0-100 range
                    matrixGainSliders[f][c]->setVisible(true);
                    // Show and update connection gain display
                    matrixGainDisplays[f][c]->setText(std::to_string(static_cast<int>(defaultGain * 100)));
                    matrixGainDisplays[f][c]->setVisible(true);
                }
                
                // Block matrix updates for several frames after toggle interaction
                toggleBlockCounter = 30; // Block for ~0.5 seconds at 60fps
            });
            
            connectionMatrixPanel->add(toggleButton);
            buttonRow.push_back(toggleButton);
            
            // Gain slider (only visible when connected)
            auto gainSlider = tgui::Slider::create(0.0f, 100.0f);
            gainSlider->setPosition(175 + c * 80, 90 + displayRow * 60); // Right of toggle button, reversed position
            gainSlider->setSize(15, 20);
            // Initialize with a neutral default; do not reflect live weight
            gainSlider->setValue(30.0f);
            gainSlider->setVisible(false);
            
            // Gain change callback
            gainSlider->onValueChange([this, f, c](float value) {
                if (!network) return;
                
                float weight = value / 100.0f; // Convert from 0-100 to 0-1 range
                
                // Preserve sign if it was negative
                float currentWeight = network->getRhythmConnection(f, c);
                if (currentWeight < 0) {
                    weight = -weight;
                }
                
                // Update the rhythm connection weight
                network->setRhythmConnection(f, c, weight);
                
                // Update connection gain display
                matrixGainDisplays[f][c]->setText(std::to_string(static_cast<int>(value)));
            });
            
            connectionMatrixPanel->add(gainSlider);
            sliderRow.push_back(gainSlider);
            
            // Connection gain value display (shows current connection weight)
            auto connectionGainDisplay = tgui::Label::create("0.0");
            connectionGainDisplay->setPosition(195 + c * 80, 90 + displayRow * 60); // Right of the gain slider, reversed position
            connectionGainDisplay->setSize(20, 20); // Small square label
            connectionGainDisplay->setTextSize(7);
            connectionGainDisplay->getRenderer()->setTextColor(tgui::Color(200, 200, 140));
            connectionGainDisplay->getRenderer()->setBackgroundColor(tgui::Color(25, 25, 15));
            connectionGainDisplay->getRenderer()->setBorderColor(tgui::Color(60, 60, 40));
            connectionGainDisplay->getRenderer()->setBorders(1);
            connectionGainDisplay->setVisible(false);
            
            connectionMatrixPanel->add(connectionGainDisplay);
            displayRowVec.push_back(connectionGainDisplay);
        }
        
            matrixToggleButtons.push_back(buttonRow);
            matrixGainSliders.push_back(sliderRow);
            matrixGainDisplays.push_back(displayRowVec);
        }
    }
    
    // Add BPM slider with better spacing from the right edge
    float scaleSliderX = contentWidth - 160; // Reserved X for right-side controls
    float bpmSliderX = contentWidth - 80; // Position BPM slider

    // Channel count control label
    auto channelsLabel = tgui::Label::create("Channels");
    channelsLabel->setPosition(scaleSliderX - 10, 15);
    channelsLabel->setTextSize(10);
    channelsLabel->getRenderer()->setTextColor(tgui::Color(180, 180, 180));
    connectionMatrixPanel->add(channelsLabel);

    // Channels ComboBox (4-16)
    auto channelsCombo = tgui::ComboBox::create();
    channelsCombo->setPosition(scaleSliderX + 5, 15);
    channelsCombo->setSize(60, 18);
    channelsCombo->setTextSize(10);
    channelsCombo->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 40));
    channelsCombo->getRenderer()->setTextColor(tgui::Color(220, 220, 220));
    channelsCombo->getRenderer()->setBorderColor(tgui::Color(80, 80, 80));
    channelsCombo->getRenderer()->setBorders(1);
    const std::vector<int> channelOptions = {4, 6, 8, 10, 12, 16};
    for (int opt : channelOptions) {
        channelsCombo->addItem(std::to_string(opt), std::to_string(opt));
    }
    channelsCombo->setSelectedItemById(std::to_string(static_cast<int>(numFilters)));
    channelsCombo->onItemSelect([this, channelsCombo](const tgui::String& /*item*/){
        if (!network || !network->getRhythmInterpreter()) return;
        // Map index to channel options {4,6,8,10,12,16}
        const std::vector<int> channelOptions = {4, 6, 8, 10, 12, 16};
        int idx = channelsCombo->getSelectedItemIndex();
        size_t newCount = 8;
        if (idx >= 0 && static_cast<size_t>(idx) < channelOptions.size())
            newCount = static_cast<size_t>(channelOptions[idx]);
        network->getRhythmInterpreter()->setBandCount(newCount);
        // Recreate matrix panel to reflect new channel count
        createConnectionMatrixPanel();
    });
    connectionMatrixPanel->add(channelsCombo);
    
    // BPM control label - make more prominent with better spacing
    auto bpmLabelTitle = tgui::Label::create("BPM");
    bpmLabelTitle->setPosition(bpmSliderX - 20, 45); // Move left and up slightly
    bpmLabelTitle->setTextSize(12); // Larger text
    bpmLabelTitle->getRenderer()->setTextColor(tgui::Color(200, 200, 100)); // Yellowish for visibility
    connectionMatrixPanel->add(bpmLabelTitle);
    
    // Vertical BPM slider (30.0 - 300.0, default 120.0, step 0.1) - expanded range for better autodetect
    bpmSlider = tgui::Slider::create(30.0f, 300.0f);
    bpmSlider->setValue(120.0f); // Default BPM for minimal RhythmInterpreter
    bpmSlider->setStep(0.1f);
    bpmSlider->setPosition(bpmSliderX - 10, 65); // Move left and up slightly  
    bpmSlider->setSize(20, 300); // Original slider size
    bpmSlider->setOrientation(tgui::Orientation::Vertical);
    bpmSlider->getRenderer()->setTrackColor(tgui::Color(60, 60, 60));
    bpmSlider->getRenderer()->setThumbColor(tgui::Color(100, 140, 100));
    
    // Connect BPM slider to BPM control (only when autodetect is disabled)
    bpmSlider->onValueChange([this](float value) {
        if (network && network->getRhythmInterpreter()) {
            // Minimal RhythmInterpreter: getAutodetectTempo and setBPM methods not supported
            // Update BPM display with proper formatting (one decimal place)
            std::ostringstream stream;
            stream << std::fixed << std::setprecision(1) << value;
            bpmLabel->setText(stream.str());
            // Update frequency labels to reflect new tempo scaling
            updateFrequencyLabels();
            // Update quantizer BPM to match global tempo
            updateQuantizerBPM(value);
        }
    });
    
    connectionMatrixPanel->add(bpmSlider);
    
    // BPM value display - LARGE for easy reading
    bpmLabel = tgui::Label::create("120.0");
    bpmLabel->setPosition(bpmSliderX - 35, 370); // Centered under moved slider position  
    bpmLabel->setSize(70, 35); // Larger for better visibility
    bpmLabel->setTextSize(16); // Larger text for better readability
    bpmLabel->getRenderer()->setTextColor(tgui::Color(150, 255, 150)); // Brighter green
    bpmLabel->getRenderer()->setBackgroundColor(tgui::Color(20, 40, 20)); // Darker background
    bpmLabel->getRenderer()->setBorderColor(tgui::Color(60, 120, 60)); // Brighter border
    bpmLabel->getRenderer()->setBorders(2); // Thicker border
    
    // Initialize BPM display with current value
    std::ostringstream bpmInitStream;
    bpmInitStream << std::fixed << std::setprecision(1) << 120.0f; // Default BPM
    bpmLabel->setText(bpmInitStream.str());
    
    connectionMatrixPanel->add(bpmLabel);
    
    // Add tempo control section label for better visibility with proper spacing
    auto tempoSectionLabel = tgui::Label::create("TEMPO CONTROLS");
    tempoSectionLabel->setPosition(bpmSliderX - 50, 415); // Move further left and down
    tempoSectionLabel->setTextSize(10);
    tempoSectionLabel->getRenderer()->setTextColor(tgui::Color(200, 200, 100));
    connectionMatrixPanel->add(tempoSectionLabel);
    
    // Add Autodetect Tempo toggle button with better spacing
    autodetectTempoToggle = tgui::Button::create("AUTO TEMPO");
    autodetectTempoToggle->setPosition(bpmSliderX - 40, 440); // More space below section label
    autodetectTempoToggle->setSize(80, 30); // Larger for better visibility
    autodetectTempoToggle->setTextSize(10); // Larger text
    autodetectTempoToggle->getRenderer()->setBackgroundColor(tgui::Color(60, 60, 60)); // Lighter background
    autodetectTempoToggle->getRenderer()->setTextColor(tgui::Color(220, 220, 100)); // Yellowish text
    autodetectTempoToggle->getRenderer()->setBorderColor(tgui::Color(120, 120, 60)); // Yellow border
    autodetectTempoToggle->getRenderer()->setBorders(2); // Thicker border
    
    // Set initial state (default is OFF)
    // Auto-tempo is available now with the enhanced RhythmInterpreter
    
    // Connect autodetect toggle to tempo control
    autodetectTempoToggle->onPress([this]() {
        if (network && network->getRhythmInterpreter()) {
            auto rhythmInterpreter = network->getRhythmInterpreter();
            bool currentState = rhythmInterpreter->isAutoTempoEnabled();
            bool newState = !currentState;
            
            // Enable/disable auto-tempo functionality
            rhythmInterpreter->setAutoTempoEnabled(newState);
            
            // Update toggle appearance
            if (newState) {
                autodetectTempoToggle->getRenderer()->setBackgroundColor(tgui::Color(100, 200, 100)); // Bright green when active
                autodetectTempoToggle->getRenderer()->setTextColor(tgui::Color(255, 255, 255));
                autodetectTempoToggle->setText("AUTO ON");
                
                // Disable BPM slider when autodetect is ON
                if (bpmSlider) {
                    bpmSlider->getRenderer()->setThumbColor(tgui::Color(60, 60, 60)); // Grayed out
                }
            } else {
                autodetectTempoToggle->getRenderer()->setBackgroundColor(tgui::Color(60, 60, 60)); // Default gray
                autodetectTempoToggle->getRenderer()->setTextColor(tgui::Color(220, 220, 100));
                autodetectTempoToggle->setText("AUTO TEMPO");
                
                // Re-enable BPM slider when autodetect is OFF
                if (bpmSlider) {
                    bpmSlider->getRenderer()->setThumbColor(tgui::Color(100, 140, 100)); // Normal color
                }
            }
        }
    });
    
    connectionMatrixPanel->add(autodetectTempoToggle);
    
    // Add detected tempo status display with better spacing
    detectedTempoLabel = tgui::Label::create("Detected: --");
    detectedTempoLabel->setPosition(bpmSliderX - 45, 480); // More space below auto-tempo button
    detectedTempoLabel->setSize(90, 25); // Slightly taller
    detectedTempoLabel->setTextSize(10); // Slightly larger text
    detectedTempoLabel->getRenderer()->setTextColor(tgui::Color(150, 150, 255)); // Blue for detected tempo
    detectedTempoLabel->getRenderer()->setBackgroundColor(tgui::Color(10, 10, 30));
    detectedTempoLabel->getRenderer()->setBorderColor(tgui::Color(50, 50, 100));
    detectedTempoLabel->getRenderer()->setBorders(1);
    connectionMatrixPanel->add(detectedTempoLabel);

    // =============================================================================
    // Learning Controls
    // =============================================================================
    // Section label
    auto learningSectionLabel = tgui::Label::create("LEARNING");
    learningSectionLabel->setPosition(scaleSliderX - 55, 415);
    learningSectionLabel->setTextSize(10);
    learningSectionLabel->getRenderer()->setTextColor(tgui::Color(200, 150, 200));
    connectionMatrixPanel->add(learningSectionLabel);

    // Toggle button
    bool learningInitiallyEnabled = network && network->isLearningEnabled();
    learningToggle = tgui::Button::create(learningInitiallyEnabled ? "LEARN ON" : "LEARN OFF");
    learningToggle->setPosition(scaleSliderX - 60, 440);
    learningToggle->setSize(100, 30);
    learningToggle->getRenderer()->setBackgroundColor(learningInitiallyEnabled ? tgui::Color(100, 80, 140) : tgui::Color(60, 60, 60));
    learningToggle->getRenderer()->setTextColor(tgui::Color(255, 255, 255));
    learningToggle->getRenderer()->setBorderColor(tgui::Color(120, 90, 160));
    learningToggle->getRenderer()->setBorders(2);
    learningToggle->onPress([this]() {
        if (!network) return;
        bool enabled = network->isLearningEnabled();
        network->setLearningEnabled(!enabled);
        bool nowEnabled = network->isLearningEnabled();
        learningToggle->setText(nowEnabled ? "LEARN ON" : "LEARN OFF");
        learningToggle->getRenderer()->setBackgroundColor(nowEnabled ? tgui::Color(100, 80, 140) : tgui::Color(60, 60, 60));
    });
    connectionMatrixPanel->add(learningToggle);

    // Learning rate slider (0.0 - 0.1)
    auto lrLabelTitle = tgui::Label::create("Rate");
    lrLabelTitle->setPosition(scaleSliderX - 60, 480);
    lrLabelTitle->setTextSize(10);
    lrLabelTitle->getRenderer()->setTextColor(tgui::Color(200, 200, 200));
    connectionMatrixPanel->add(lrLabelTitle);

    learningRateSlider = tgui::Slider::create(0.0f, 0.1f);
    learningRateSlider->setValue(network ? network->getLearningRate() : 0.02f);
    learningRateSlider->setStep(0.001f);
    learningRateSlider->setPosition(scaleSliderX - 60, 500);
    learningRateSlider->setSize(100, 18);
    learningRateSlider->getRenderer()->setTrackColor(tgui::Color(60, 60, 60));
    learningRateSlider->getRenderer()->setThumbColor(tgui::Color(180, 130, 200));
    learningRateSlider->onValueChange([this](float value) {
        if (!network) return;
        network->setLearningRate(value);
        std::ostringstream s;
        s << std::fixed << std::setprecision(3) << value;
        if (learningRateLabel) learningRateLabel->setText(s.str());
    });
    connectionMatrixPanel->add(learningRateSlider);

    learningRateLabel = tgui::Label::create("0.020");
    learningRateLabel->setPosition(scaleSliderX + 45, 497);
    learningRateLabel->setSize(50, 22);
    learningRateLabel->setTextSize(10);
    learningRateLabel->getRenderer()->setTextColor(tgui::Color(230, 200, 255));
    learningRateLabel->getRenderer()->setBackgroundColor(tgui::Color(25, 15, 25));
    learningRateLabel->getRenderer()->setBorderColor(tgui::Color(100, 80, 140));
    learningRateLabel->getRenderer()->setBorders(1);
    {
        float lr = network ? network->getLearningRate() : 0.02f;
        std::ostringstream s; s << std::fixed << std::setprecision(3) << lr;
        learningRateLabel->setText(s.str());
    }
    connectionMatrixPanel->add(learningRateLabel);

    // Weight decay slider (0.0 - 0.01)
    auto wdLabelTitle = tgui::Label::create("Decay");
    wdLabelTitle->setPosition(scaleSliderX - 60, 525);
    wdLabelTitle->setTextSize(10);
    wdLabelTitle->getRenderer()->setTextColor(tgui::Color(200, 200, 200));
    connectionMatrixPanel->add(wdLabelTitle);

    weightDecaySlider = tgui::Slider::create(0.0f, 0.01f);
    weightDecaySlider->setValue(network ? network->getWeightDecay() : 0.0005f);
    weightDecaySlider->setStep(0.0001f);
    weightDecaySlider->setPosition(scaleSliderX - 60, 545);
    weightDecaySlider->setSize(100, 18);
    weightDecaySlider->getRenderer()->setTrackColor(tgui::Color(60, 60, 60));
    weightDecaySlider->getRenderer()->setThumbColor(tgui::Color(180, 130, 200));
    weightDecaySlider->onValueChange([this](float value) {
        if (!network) return;
        network->setWeightDecay(value);
        std::ostringstream s;
        s << std::fixed << std::setprecision(4) << value;
        if (weightDecayLabel) weightDecayLabel->setText(s.str());
    });
    connectionMatrixPanel->add(weightDecaySlider);

    weightDecayLabel = tgui::Label::create("0.0005");
    weightDecayLabel->setPosition(scaleSliderX + 45, 542);
    weightDecayLabel->setSize(50, 22);
    weightDecayLabel->setTextSize(10);
    weightDecayLabel->getRenderer()->setTextColor(tgui::Color(230, 200, 255));
    weightDecayLabel->getRenderer()->setBackgroundColor(tgui::Color(25, 15, 25));
    weightDecayLabel->getRenderer()->setBorderColor(tgui::Color(100, 80, 140));
    weightDecayLabel->getRenderer()->setBorders(1);
    {
        float wd = network ? network->getWeightDecay() : 0.0005f;
        std::ostringstream s; s << std::fixed << std::setprecision(4) << wd;
        weightDecayLabel->setText(s.str());
    }
    connectionMatrixPanel->add(weightDecayLabel);

    // Mapping gain slider (0.0 - 1.0)
    auto mgLabelTitle = tgui::Label::create("MapGain");
    mgLabelTitle->setPosition(scaleSliderX - 60, 595);
    mgLabelTitle->setTextSize(10);
    mgLabelTitle->getRenderer()->setTextColor(tgui::Color(200, 200, 200));
    connectionMatrixPanel->add(mgLabelTitle);

    auto mappingGainSlider = tgui::Slider::create(0.0f, 1.0f);
    mappingGainSlider->setValue(network ? network->getMappingGain() : 0.2f);
    mappingGainSlider->setStep(0.01f);
    mappingGainSlider->setPosition(scaleSliderX - 60, 615);
    mappingGainSlider->setSize(100, 18);
    mappingGainSlider->getRenderer()->setTrackColor(tgui::Color(60, 60, 60));
    mappingGainSlider->getRenderer()->setThumbColor(tgui::Color(180, 130, 200));
    mappingGainSlider->onValueChange([this](float value) {
        if (!network) return;
        network->setMappingGain(value);
    });
    connectionMatrixPanel->add(mappingGainSlider);

    // Reset weights button
    resetRhythmWeightsButton = tgui::Button::create("Reset Weights");
    resetRhythmWeightsButton->setPosition(scaleSliderX - 60, 570);
    resetRhythmWeightsButton->setSize(100, 26);
    resetRhythmWeightsButton->getRenderer()->setBackgroundColor(tgui::Color(80, 60, 60));
    resetRhythmWeightsButton->getRenderer()->setBorderColor(tgui::Color(140, 100, 100));
    resetRhythmWeightsButton->getRenderer()->setTextColor(tgui::Color::White);
    resetRhythmWeightsButton->onPress([this]() {
        if (!network) return;
        network->resetRhythmWeights(0.0f);
        // Force matrix UI refresh so sliders reflect zeros
        forceMatrixUpdate();
    });
    connectionMatrixPanel->add(resetRhythmWeightsButton);

    // =========================================================================
    // Input Audio Playback Controls
    // =========================================================================
    audioControlsLabel = tgui::Label::create("AUDIO");
    audioControlsLabel->setPosition(bpmSliderX - 45, 605);
    audioControlsLabel->setTextSize(10);
    audioControlsLabel->getRenderer()->setTextColor(tgui::Color(200, 200, 200));
    connectionMatrixPanel->add(audioControlsLabel);

    // Play
    inputPlayButton = tgui::Button::create("Play");
    inputPlayButton->setPosition(bpmSliderX - 70, 625);
    inputPlayButton->setSize(50, 24);
    inputPlayButton->getRenderer()->setBackgroundColor(tgui::Color(60, 100, 60));
    inputPlayButton->getRenderer()->setTextColor(tgui::Color::White);
    inputPlayButton->onPress([this]() {
        if (!audioManager) return;
        if (!audioManager->hasInputFile()) {
            openInputFileDialog();
        } else {
            audioManager->startInputPlayback();
        }
    });
    connectionMatrixPanel->add(inputPlayButton);

    // Pause
    inputPauseButton = tgui::Button::create("Pause");
    inputPauseButton->setPosition(bpmSliderX - 15, 625);
    inputPauseButton->setSize(60, 24);
    inputPauseButton->getRenderer()->setBackgroundColor(tgui::Color(100, 100, 60));
    inputPauseButton->getRenderer()->setTextColor(tgui::Color::White);
    inputPauseButton->onPress([this]() {
        if (!audioManager) return;
        audioManager->pauseInputPlayback();
    });
    connectionMatrixPanel->add(inputPauseButton);

    // Stop
    inputStopButton = tgui::Button::create("Stop");
    inputStopButton->setPosition(bpmSliderX + 55, 625);
    inputStopButton->setSize(50, 24);
    inputStopButton->getRenderer()->setBackgroundColor(tgui::Color(120, 60, 60));
    inputStopButton->getRenderer()->setTextColor(tgui::Color::White);
    inputStopButton->onPress([this]() {
        if (!audioManager) return;
        audioManager->stopInputPlayback();
    });
    connectionMatrixPanel->add(inputStopButton);
    
    // ============================================================================
    // BeatRoot Controls - DISABLED BY DEFAULT
    // ============================================================================
    
    // BeatRoot controls have been removed from the GUI to simplify the interface
    // The BeatRoot system is disabled by default in RhythmInterpreter
    
    /* BeatRoot controls disabled in minimal RhythmInterpreter */
}

void GUI::updateConnectionMatrix() {
    if (!connectionMatrixPanel || !network || !network->getRhythmInterpreter()) {
        return;
    }
    
    // Skip automatic updates unless explicitly allowed (to fix multiple toggle issue)
    if (!allowMatrixUpdates) {
        return;
    }
    
    // Skip update completely if matrix is being updated by user interactions
    if (isUpdatingMatrix) {
        return;
    }
    
    isUpdatingMatrix = true; // Prevent recursive calls
    
    auto rhythmInterpreter = network->getRhythmInterpreter();
    size_t numFilters = rhythmInterpreter->getBandCount();
    size_t numConnections = network->getConnectionCount();
    
    // Update title
    if (matrixTitleLabel) {
        matrixTitleLabel->setText("🎛️ Rhythmogram Mapping (" + std::to_string(numFilters) + "×" + std::to_string(numConnections) + ")");
    }
    
    // Update slider values and displays without auto-changing toggle states
    for (size_t f = 0; f < std::min(numFilters, matrixToggleButtons.size()); ++f) {
        for (size_t c = 0; c < std::min(numConnections, matrixToggleButtons[f].size()); ++c) {
            float weight = network->getRhythmConnection(f, c); // Get actual mapping weight
            // Respect current UI toggle state; do not auto-change it based on weight
            bool uiConnected = (matrixToggleButtons[f][c]->getText() == "●");
            
            // Do not change gain slider value automatically; preserve user input
            matrixGainSliders[f][c]->setVisible(uiConnected);
            
            // Update connection gain display
            if (f < matrixGainDisplays.size() && c < matrixGainDisplays[f].size()) {
                matrixGainDisplays[f][c]->setText(std::to_string(static_cast<int>(std::abs(weight) * 100)));
                matrixGainDisplays[f][c]->setVisible(uiConnected);
            }
        }
    }
    
    // Update filter gain displays
    for (size_t f = 0; f < std::min(numFilters, filterGainDisplays.size()); ++f) {
        float gainValue = rhythmInterpreter->getFilterGain(f);
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(1) << gainValue << "x";
        filterGainDisplays[f]->setText(stream.str());
    }
    
    // Update filter output displays with real-time levels
    auto filterOutputs = rhythmInterpreter->getFilterOutputs(); // This method exists in minimal RhythmInterpreter
    static int guiDebugCounter = 0;
    
    for (size_t f = 0; f < std::min(numFilters, filterOutputDisplays.size()) && f < filterOutputs.size(); ++f) {
        float outputLevel = std::min(1.0f, std::abs(filterOutputs[f])); // Clamp to 0-1 range
        
        // Debug: Log rhythmogram output levels occasionally  
        if (++guiDebugCounter % 50 == 0 && f < 8) { // Show all 8 rhythmogram bands
            std::ostringstream name;
            name << "Band " << (f+1);
            std::string bandName = name.str();
            DEBUG_PRINT_STREAM("🎛️  Rhythmogram " << f << " (" << bandName << ") level: " << filterOutputs[f] 
                      << " (clamped: " << outputLevel << ")");
        }

        // Format with appropriate precision for both large and small values
        std::string levelText;
        if (outputLevel >= 0.0001f) {
            // Use per-mille (‰) for values >= 0.01%
            levelText = std::to_string(static_cast<int>(outputLevel * 10000)) + "‰";
        } else {
            // Use scientific notation for very small values
            std::ostringstream stream;
            stream << std::scientific << std::setprecision(1) << outputLevel;
            levelText = stream.str();
        }
        filterOutputDisplays[f]->setText(levelText);
        
        // Color coding: green for low, yellow for medium, red for high levels
        if (outputLevel < 0.3f) {
            filterOutputDisplays[f]->getRenderer()->setTextColor(tgui::Color(100, 200, 100));
        } else if (outputLevel < 0.7f) {
            filterOutputDisplays[f]->getRenderer()->setTextColor(tgui::Color(200, 200, 100));
        } else {
            filterOutputDisplays[f]->getRenderer()->setTextColor(tgui::Color(200, 100, 100));
        }
    }
    
    // Skip slider value updates if user is interacting (prevents overriding user input)
    // Only update displays, not the slider values themselves during normal operation
    
    // Rhythmogram scale control removed
    
    // Update BPM display (but don't override slider value unless auto-tempo is active)
    if (bpmSlider && bpmLabel) {
        bool autodetectActive = network && network->getRhythmInterpreter() && 
                               network->getRhythmInterpreter()->isAutoTempoEnabled();
        
        float currentBPM;
        if (autodetectActive) {
            // When auto-tempo is active, show detected tempo and update slider
            currentBPM = network->getRhythmInterpreter()->getDetectedTempo();
            bpmSlider->setValue(currentBPM);
        } else {
            // When manual control, get BPM from slider (don't override user input)
            currentBPM = bpmSlider->getValue();
        }
        
        // Check if BPM has changed (for frequency label updates)
        static float lastBPM = currentBPM;
        bool bpmChanged = (std::abs(currentBPM - lastBPM) > 0.1f);
        
        // Format BPM display with autodetect indicator
        std::ostringstream bpmStream;
        bpmStream << std::fixed << std::setprecision(1) << currentBPM;
        if (autodetectActive) {
            bpmStream << "🎵"; // Musical note to indicate autodetection
        }
        bpmLabel->setText(bpmStream.str());
        
        // Update frequency labels when BPM changes
        if (bpmChanged) {
            updateFrequencyLabels();
            lastBPM = currentBPM;
        }
        
        // Update slider visual state based on autodetect status
        if (autodetectActive) {
            bpmSlider->getRenderer()->setThumbColor(tgui::Color(60, 60, 60)); // Grayed out when autodetecting
        } else {
            bpmSlider->getRenderer()->setThumbColor(tgui::Color(100, 140, 100)); // Normal green when manual
        }
    }
    
    // Update autodetect toggle button appearance
    if (autodetectTempoToggle) {
        bool autodetectActive = network && network->getRhythmInterpreter() && 
                               network->getRhythmInterpreter()->isAutoTempoEnabled();
        if (autodetectActive) {
            autodetectTempoToggle->getRenderer()->setBackgroundColor(tgui::Color(100, 200, 100)); // Bright green when active
            autodetectTempoToggle->getRenderer()->setTextColor(tgui::Color(255, 255, 255));
            autodetectTempoToggle->setText("AUTO ON");
        } else {
            autodetectTempoToggle->getRenderer()->setBackgroundColor(tgui::Color(60, 60, 60)); // Default gray
            autodetectTempoToggle->getRenderer()->setTextColor(tgui::Color(220, 220, 100));
            autodetectTempoToggle->setText("AUTO TEMPO");
        }
    }
    
    // BeatRoot controls disabled in minimal RhythmInterpreter
    
    isUpdatingMatrix = false; // Reset the flag
}

void GUI::toggleMatrixVisibility() {
    if (!connectionMatrixWindow) {
        std::cout << "⚠️ Matrix window not initialized - creating now..." << std::endl;
        createConnectionMatrixPanel(); // Try to initialize if not done yet
        if (!connectionMatrixWindow) {
            std::cout << "❌ Failed to create matrix window" << std::endl;
            return;
        }
    }

    // Sync matrixVisible with actual window state before toggling
    matrixVisible = connectionMatrixWindow->isVisible();
    matrixVisible = !matrixVisible;
    connectionMatrixWindow->setVisible(matrixVisible);
    
    // Force the window to the front if showing
    if (matrixVisible && gui) {
        connectionMatrixWindow->moveToFront();
    }
    
    std::cout << "🎛️ Rhythmogram Mapping " << (matrixVisible ? "shown" : "hidden") 
              << " (press M to toggle)" << std::endl;
}

void GUI::forceMatrixUpdate() {
    allowMatrixUpdates = true;
    updateConnectionMatrix();
    // Keep allowMatrixUpdates = true to allow future automatic updates
}

void GUI::toggleQuantizerVisibility() {
    if (quantizerWindow) {
        bool currentState = quantizerWindow->isVisible();
        bool newState = !currentState;
        quantizerWindow->setVisible(newState);
        
        // Also ensure the internal widget is visible when the window is shown
        if (quantizerWidget) {
            quantizerWidget->setVisible(newState);
        }
        
        // Force the window to the front if showing
        if (newState && gui) {
            quantizerWindow->moveToFront();
        }
        
        std::cout << "🎵 Quantizer " << (newState ? "shown" : "hidden") << " (press Q to toggle)" << std::endl;
    } else if (quantizerWidget) {
        // Fallback: toggle internal widget visibility
        std::cout << "⚠️ Quantizer window not found, using widget fallback..." << std::endl;
        quantizerWidget->toggleVisibility();
        std::cout << "🎵 Quantizer " << (quantizerWidget->isVisible() ? "shown" : "hidden") 
                  << " (press Q to toggle)" << std::endl;
    } else {
        std::cout << "❌ Quantizer not initialized - cannot toggle visibility" << std::endl;
    }
}

void GUI::updateQuantizerBPM(float bpm) {
    // Only update if BPM changed significantly (more than 0.1 BPM difference)
    if (std::abs(bpm - lastQuantizerBPM) < 0.1f) {
        return;
    }
    
    lastQuantizerBPM = bpm;
    
    if (quantizer) {
        quantizer->setBPM(bpm);
    }
    if (quantizerWidget) {
        quantizerWidget->updateFromQuantizer();
    }
}

void GUI::updateFrequencyLabels() {
    if (!network || !network->getRhythmInterpreter()) {
        return;
    }
    auto rhythmInterpreter = network->getRhythmInterpreter();
    auto freqs = rhythmInterpreter->getBandFrequencies();
    // Update spectral display BPM indicator (manual)
    if (spectralDisplay && bpmSlider) {
        spectralDisplay->setManualBPM(bpmSlider->getValue());
    }
    for (size_t i = 0; i < filterLabels.size(); ++i) {
        float f = (i < freqs.size()) ? freqs[i] : 0.0f;
        std::ostringstream fs;
        if (f < 1.0f) fs << std::fixed << std::setprecision(3) << f;
        else fs << std::fixed << std::setprecision(1) << f;
        std::string labelText = std::string("Band ") + std::to_string(i+1) + " (" + fs.str() + "Hz)";
        filterLabels[i]->setText(labelText);
    }
}

// Frequency response visualization method temporarily removed due to TGUI widget limitations

bool GUI::isTextInputActive() const {
    if (!gui) {
        return false;
    }
    
    // Check if any EditBox currently has focus by recursively searching through all widgets
    return checkWidgetTreeForFocusedEditBox(gui->getContainer());
}

bool GUI::checkWidgetTreeForFocusedEditBox(tgui::Container::Ptr container) const {
    for (auto& widget : container->getWidgets()) {
        // Check if this widget is an EditBox and has focus
        auto editBox = std::dynamic_pointer_cast<tgui::EditBox>(widget);
        if (editBox && editBox->isFocused()) {
            return true;
        }
        
        // Recursively check containers (panels, child windows, etc.)
        auto childContainer = std::dynamic_pointer_cast<tgui::Container>(widget);
        if (childContainer && checkWidgetTreeForFocusedEditBox(childContainer)) {
            return true;
        }
    }
    return false;
}

// ================================================================================================
// Preset Management Methods
// ================================================================================================

void GUI::showSavePresetDialog() {
    auto dialog = tgui::ChildWindow::create("Save Preset");
    dialog->setSize(400, 250);
    dialog->setPosition("50%", "50%");
    dialog->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 40));
    dialog->getRenderer()->setBorderColor(tgui::Color::White);
    
    // Name input
    auto nameLabel = tgui::Label::create("Preset Name:");
    nameLabel->setPosition(20, 30);
    nameLabel->getRenderer()->setTextColor(tgui::Color::White);
    dialog->add(nameLabel);
    
    auto nameInput = tgui::EditBox::create();
    nameInput->setSize(300, 30);
    nameInput->setPosition(20, 55);
    nameInput->setText("My Preset");
    dialog->add(nameInput);
    
    // Description input
    auto descLabel = tgui::Label::create("Description:");
    descLabel->setPosition(20, 95);
    descLabel->getRenderer()->setTextColor(tgui::Color::White);
    dialog->add(descLabel);
    
    auto descInput = tgui::EditBox::create();
    descInput->setSize(300, 30);
    descInput->setPosition(20, 120);
    descInput->setText("Custom neural network preset");
    dialog->add(descInput);
    
    // Buttons
    auto saveButton = tgui::Button::create("Save");
    saveButton->setPosition(80, 180);
    saveButton->setSize(100, 30);
    saveButton->onPress([=]() {
        PresetManager::PresetInfo info;
        info.name = nameInput->getText().toStdString();
        info.description = descInput->getText().toStdString();
        info.author = "User";
        
        std::string filename = "presets/user/" + info.name + ".json";
        
        if (PresetManager::savePreset(*network, filename, info)) {
            std::cout << "✅ Preset saved: " << filename << std::endl;
        } else {
            std::cout << "❌ Failed to save preset" << std::endl;
        }
        
        dialog->close();
    });
    dialog->add(saveButton);
    
    auto cancelButton = tgui::Button::create("Cancel");
    cancelButton->setPosition(220, 180);
    cancelButton->setSize(100, 30);
    cancelButton->onPress([=]() {
        dialog->close();
    });
    dialog->add(cancelButton);
    
    gui->add(dialog);
}

void GUI::showLoadPresetDialog() {
    auto dialog = tgui::ChildWindow::create("Load Preset");
    dialog->setSize(500, 400);
    dialog->setPosition("50%", "50%");
    dialog->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 40));
    dialog->getRenderer()->setBorderColor(tgui::Color::White);
    
    // Preset list
    auto presetList = tgui::ListBox::create();
    presetList->setSize(450, 280);
    presetList->setPosition(25, 40);
    
    // Force dark theme colors for ListBox using direct renderer methods
    auto renderer = presetList->getRenderer();
    renderer->setBackgroundColor(tgui::Color(60, 60, 60));
    renderer->setBackgroundColorHover(tgui::Color(70, 70, 70));
    renderer->setTextColor(tgui::Color::White);
    renderer->setTextColorHover(tgui::Color::White);
    renderer->setSelectedBackgroundColor(tgui::Color(80, 120, 200));
    renderer->setSelectedBackgroundColorHover(tgui::Color(90, 130, 210));
    renderer->setBorderColor(tgui::Color(100, 100, 100));
    renderer->setBorders(1);
    
    // Populate with available presets
    auto factoryPresets = PresetManager::getAvailablePresets("presets/factory/");
    auto userPresets = PresetManager::getAvailablePresets("presets/user/");
    
    presetList->addItem("--- Factory Presets ---");
    for (const auto& preset : factoryPresets) {
        auto info = PresetManager::getPresetInfo(preset);
        std::string displayName = info.name.empty() ? 
            std::filesystem::path(preset).stem().string() : info.name;
        std::string filename = std::filesystem::path(preset).stem().string();
        presetList->addItem("F: " + displayName + " (" + filename + ")", preset);
    }
    
    presetList->addItem("--- User Presets ---");
    for (const auto& preset : userPresets) {
        auto info = PresetManager::getPresetInfo(preset);
        std::string displayName = info.name.empty() ? 
            std::filesystem::path(preset).stem().string() : info.name;
        std::string filename = std::filesystem::path(preset).stem().string();
        presetList->addItem("U: " + displayName + " (" + filename + ")", preset);
    }
    
    dialog->add(presetList);
    
    // Load button
    auto loadButton = tgui::Button::create("Load");
    loadButton->setPosition(150, 340);
    loadButton->setSize(100, 30);
    loadButton->onPress([=]() {
        auto selectedItem = presetList->getSelectedItem();
        if (!selectedItem.empty() && presetList->getSelectedItemId() != "") {
            std::string filename = presetList->getSelectedItemId().toStdString();
            if (!filename.empty() && filename != "--- Factory Presets ---" && filename != "--- User Presets ---") {
                // Notify spectral display before loading (which calls clearNetwork)
                if (spectralDisplay) {
                    spectralDisplay->setRhythmInterpreter(nullptr);
                }
                
                if (PresetManager::loadPreset(*network, filename)) {
                    std::cout << "✅ Preset loaded: " << filename << std::endl;
                    
                    // Load sample files into AudioManager
                    loadPresetSamplesIntoAudioManager();
                    
                    // Update AudioManager with new rhythm interpreter
                    auto audioManager = network->getAudioManager();
                    if (audioManager && network->getRhythmInterpreter()) {
                        audioManager->setRhythmInterpreter(network->getRhythmInterpreter());
                        std::cout << "🔄 AudioManager updated after preset load" << std::endl;
                    }
                    
                    // Update spectral display with new rhythm interpreter
                    if (spectralDisplay) {
                        spectralDisplay->setRhythmInterpreter(network->getRhythmInterpreter());
                    }
                    
                    visualizer->refreshLayout();  // Refresh visualizer layout
                    refreshNeuronSliders();      // Refresh neurons first (consistent with manual adding)
                    refreshConnectionSliders();
                    refreshConnectionMatrix();
                } else {
                    std::cout << "❌ Failed to load preset" << std::endl;
                }
            }
        }
        dialog->close();
    });
    dialog->add(loadButton);
    
    auto cancelButton = tgui::Button::create("Cancel");
    cancelButton->setPosition(270, 340);
    cancelButton->setSize(100, 30);
    cancelButton->onPress([=]() {
        dialog->close();
    });
    dialog->add(cancelButton);
    
    gui->add(dialog);
}

void GUI::loadFactoryDrumPattern() {
    // Notify spectral display before loading (which calls clearNetwork)
    if (spectralDisplay) {
        spectralDisplay->setRhythmInterpreter(nullptr);
    }
    
    if (PresetManager::loadFactoryPreset(*network, "drum_pattern")) {
        std::cout << "✅ Loaded factory drum pattern preset" << std::endl;
        
        // Load sample files into AudioManager
        loadPresetSamplesIntoAudioManager();
        
        // Update AudioManager with new rhythm interpreter
        auto audioManager = network->getAudioManager();
        if (audioManager && network->getRhythmInterpreter()) {
            audioManager->setRhythmInterpreter(network->getRhythmInterpreter());
            std::cout << "🔄 AudioManager updated after preset load" << std::endl;
        }
        
        // Update spectral display with new rhythm interpreter
        if (spectralDisplay) {
            spectralDisplay->setRhythmInterpreter(network->getRhythmInterpreter());
        }
        
        visualizer->refreshLayout();  // Refresh visualizer layout
        refreshNeuronSliders();      // Refresh neurons first (consistent with manual adding)
        refreshConnectionSliders();
        refreshConnectionMatrix();
    } else {
        std::cout << "❌ Failed to load factory preset" << std::endl;
    }
}

void GUI::openInputFileDialog() {
    if (!gui) return;
    auto fileDialog = tgui::FileDialog::create("Select Audio File", "Open");
    // Allow common audio formats
    fileDialog->setFileTypeFilters({{tgui::String{"Audio (*.wav;*.flac;*.ogg)"}, {tgui::String{"*.wav"}, tgui::String{"*.flac"}, tgui::String{"*.ogg"}}}});
    fileDialog->onFileSelect([this](const std::vector<tgui::Filesystem::Path>& paths){
        if (!paths.empty() && audioManager) {
            std::string path = paths[0].asString().toStdString();
            if (audioManager->loadInputFile(path)) {
                audioManager->startInputPlayback();
                std::cout << "🎵 Input audio selected: " << path << std::endl;
            } else {
                std::cout << "❌ Failed to load input audio: " << path << std::endl;
            }
        }
    });
    gui->add(fileDialog);
}

void GUI::showPresetBrowser() {
    auto dialog = tgui::ChildWindow::create("Preset Browser");
    dialog->setSize(600, 500);
    dialog->setPosition("50%", "50%");
    dialog->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 40));
    dialog->getRenderer()->setBorderColor(tgui::Color::White);
    
    // Get preset information
    auto factoryInfos = PresetManager::getPresetInfos("presets/factory/");
    auto userInfos = PresetManager::getPresetInfos("presets/user/");
    
    // Create scrollable panel for preset details
    auto scrollPanel = tgui::ScrollablePanel::create();
    scrollPanel->setSize(550, 400);
    scrollPanel->setPosition(25, 40);
    dialog->add(scrollPanel);
    
    float yPos = 10;
    
    // Add factory presets
    if (!factoryInfos.empty()) {
        auto factoryHeader = tgui::Label::create("Factory Presets");
        factoryHeader->setPosition(10, yPos);
        factoryHeader->getRenderer()->setTextColor(tgui::Color::Yellow);
        factoryHeader->getRenderer()->setTextStyle(tgui::TextStyle::Bold);
        scrollPanel->add(factoryHeader);
        yPos += 30;
        
        for (const auto& info : factoryInfos) {
            auto nameLabel = tgui::Label::create(info.name);
            nameLabel->setPosition(20, yPos);
            nameLabel->getRenderer()->setTextColor(tgui::Color(80, 80, 80));
            nameLabel->getRenderer()->setTextStyle(tgui::TextStyle::Bold);
            scrollPanel->add(nameLabel);
            
            auto authorLabel = tgui::Label::create("Author: " + info.author);
            authorLabel->setPosition(20, yPos + 20);
            authorLabel->getRenderer()->setTextColor(tgui::Color(100, 100, 100));
            scrollPanel->add(authorLabel);
            
            auto descLabel = tgui::Label::create(info.description);
            descLabel->setPosition(20, yPos + 40);
            descLabel->setSize(500, 40);
            descLabel->getRenderer()->setTextColor(tgui::Color(120, 120, 120));
            scrollPanel->add(descLabel);
            
            yPos += 80;
        }
    }
    
    // Add user presets
    if (!userInfos.empty()) {
        auto userHeader = tgui::Label::create("User Presets");
        userHeader->setPosition(10, yPos);
        userHeader->getRenderer()->setTextColor(tgui::Color::Cyan);
        userHeader->getRenderer()->setTextStyle(tgui::TextStyle::Bold);
        scrollPanel->add(userHeader);
        yPos += 30;
        
        for (const auto& info : userInfos) {
            auto nameLabel = tgui::Label::create(info.name);
            nameLabel->setPosition(20, yPos);
            nameLabel->getRenderer()->setTextColor(tgui::Color(80, 80, 80));
            nameLabel->getRenderer()->setTextStyle(tgui::TextStyle::Bold);
            scrollPanel->add(nameLabel);
            
            auto authorLabel = tgui::Label::create("Author: " + info.author);
            authorLabel->setPosition(20, yPos + 20);
            authorLabel->getRenderer()->setTextColor(tgui::Color(100, 100, 100));
            scrollPanel->add(authorLabel);
            
            auto descLabel = tgui::Label::create(info.description);
            descLabel->setPosition(20, yPos + 40);
            descLabel->setSize(500, 40);
            descLabel->getRenderer()->setTextColor(tgui::Color(120, 120, 120));
            scrollPanel->add(descLabel);
            
            yPos += 80;
        }
    }
    
    // Close button
    auto closeButton = tgui::Button::create("Close");
    closeButton->setPosition(250, 460);
    closeButton->setSize(100, 30);
    closeButton->onPress([=]() {
        dialog->close();
    });
    dialog->add(closeButton);
    
    gui->add(dialog);
}

void GUI::loadPresetSamplesIntoAudioManager() {
    if (!network || !audioManager) {
        std::cout << "⚠️ Cannot load preset samples: network or audioManager is null" << std::endl;
        return;
    }
    
    const auto& neurons = network->getNeurons();
    std::cout << "🔄 Loading " << neurons.size() << " sample files into AudioManager..." << std::endl;
    
    // Map to track unique sample files and their assigned indices
    std::map<std::string, int> sampleFileToIndex;
    int nextAvailableIndex = 1; // Start from index 1
    
    for (size_t i = 0; i < neurons.size(); ++i) {
        const Neuron* neuron = neurons[i].get();
        std::string samplePath = neuron->getSampleFilePath();
        
        if (samplePath.empty()) {
            std::cout << "⚠️ Neuron " << i << " has no sample file path" << std::endl;
            continue;
        }
        
        int assignedIndex;
        
        // Check if we've already loaded this sample file
        auto it = sampleFileToIndex.find(samplePath);
        if (it != sampleFileToIndex.end()) {
            // Reuse existing index
            assignedIndex = it->second;
            std::cout << "🔄 Reusing sample " << assignedIndex << " for neuron " << i << ": " << samplePath << std::endl;
        } else {
            // Load new sample file
            assignedIndex = nextAvailableIndex++;
            
            if (audioManager->loadSampleFromPath(assignedIndex, samplePath)) {
                sampleFileToIndex[samplePath] = assignedIndex;
                std::cout << "✅ Loaded sample " << assignedIndex << " for neuron " << i << ": " << samplePath << std::endl;
            } else {
                std::cout << "❌ Failed to load sample for neuron " << i << ": " << samplePath << std::endl;
                continue; // Skip updating neuron's sample index if loading failed
            }
        }
        
        // Update the neuron's sample index to point to the loaded sample
        // We need to cast away const to modify the neuron
        Neuron* mutableNeuron = const_cast<Neuron*>(neuron);
        mutableNeuron->setSampleIndex(assignedIndex);
    }
    
    std::cout << "🎵 Preset sample loading complete! Loaded " << sampleFileToIndex.size() << " unique samples." << std::endl;
}
