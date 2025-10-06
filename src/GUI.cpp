#include "GUI.h"
#include "NeuronNetwork.h"
#include "Connection.h"
#include "AudioManager.h"
#include "Visualizer.h"
#include "Recorder.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

GUI::GUI(tgui::Gui* tguiGui, sf::RenderWindow* renderWindow, NeuronNetwork* neuronNetwork, Visualizer* visualizerPtr, Recorder* recorderPtr, AudioManager* audioMgr, float* activationIntervalPtr)
    : gui(tguiGui)
    , window(renderWindow)
    , network(neuronNetwork)
    , visualizer(visualizerPtr)
    , recorder(recorderPtr)
    , audioManager(audioMgr)
    , activationInterval(activationIntervalPtr)
{
}

void GUI::initialize() {
    createMenuBar();
    createControlPanel();
    createConnectionSliders();
    updateStatusDisplay();  // Initialize status display with current network state
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
    
    // Connect menu actions
    menuBar->connectMenuItem("Network", "Add Neuron", [this]() { this->addNeuron(); });
    menuBar->connectMenuItem("Network", "Remove Neuron", [this]() { this->removeNeuron(); });
    menuBar->connectMenuItem("Network", "Add Connection", [this]() { this->showConnectionDialog(); });
    menuBar->connectMenuItem("Network", "Remove Connection", [this]() { this->showRemoveConnectionDialog(); });
    menuBar->connectMenuItem("Network", "Reset Network", [this]() { this->resetNetwork(); });
    menuBar->connectMenuItem("Recording", "Record NeuronSeq Output", [this]() { this->startInternalRecording(); });
    menuBar->connectMenuItem("Recording", "Record External Microphone", [this]() { this->startExternalRecording(); });
    menuBar->connectMenuItem("Recording", "Stop Recording", [this]() { this->stopRecording(); });
    
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
    
    // Create scrollable panel for sliders - adjusted position for new control
    slidersPanel = tgui::ScrollablePanel::create({"95%", "75%"}); // Full width usage
    slidersPanel->setPosition("2.5%", "22%"); // Centered position
    slidersPanel->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 40, 200));
    controlPanel->add(slidersPanel, "SlidersPanel");
}

void GUI::createConnectionSliders() {
    if (!network) return;
    
    connectionSliders.clear();
    connectionLabels.clear();
    slidersPanel->removeAllWidgets();
    
    const auto& connections = network->getConnections();
    float yPos = 5.0f;
    
    for (size_t i = 0; i < connections.size(); ++i) {
        const Connection* conn = connections[i].get();
        
        // Create label
        auto label = tgui::Label::create("C" + std::to_string(i + 1) + ":");
        label->setPosition(5, yPos);
        label->setTextSize(10);
        label->getRenderer()->setTextColor(tgui::Color::White);
        slidersPanel->add(label);
        connectionLabels.push_back(label);
        
        // Create slider - optimized for wider panel
        auto slider = tgui::Slider::create();
        slider->setPosition(40, yPos);
        slider->setSize(200, 16);
        slider->setMinimum(-1.2f);
        slider->setMaximum(1.2f);
        slider->setStep(0.01f);  // 240 steps: 2.4 range / 0.01 step = 240 steps
        slider->setValue(conn->getWeight());
        
        // Connect slider to callback
        slider->onValueChange([this, i](float value) {
            this->onSliderChanged(i, value);
        });
        
        slidersPanel->add(slider);
        connectionSliders.push_back(slider);
        
        // Value label
        auto valueLabel = tgui::Label::create(std::to_string(conn->getWeight()));
        valueLabel->setPosition(250, yPos);
        valueLabel->setTextSize(9);
        valueLabel->getRenderer()->setTextColor(tgui::Color::Yellow);
        slidersPanel->add(valueLabel);
        
        yPos += 22.0f;  // Further reduced spacing
    }
    
    std::cout << "Created " << connectionSliders.size() << " connection sliders" << std::endl;
}

void GUI::onSliderChanged(size_t connectionIndex, float value) {
    if (!network || connectionIndex >= network->getConnectionCount()) return;
    
    Connection* conn = network->getConnection(connectionIndex);
    if (conn) {
        conn->setWeight(value);
        std::cout << "Updated connection " << connectionIndex << " weight to " << value << std::endl;
        
        // Update the value label
        if (connectionIndex < connectionLabels.size()) {
            auto children = slidersPanel->getWidgets();
            // Find the value label (it's the 3rd widget for each connection: label, slider, value)
            size_t valueLabelIndex = connectionIndex * 3 + 2;
            if (valueLabelIndex < children.size()) {
                auto valueLabel = std::dynamic_pointer_cast<tgui::Label>(children[valueLabelIndex]);
                if (valueLabel) {
                    valueLabel->setText(std::to_string(value));
                }
            }
        }
    }
}

void GUI::update() {
    updateStatusDisplay();
}

void GUI::updateStatusDisplay() {
    if (!network || !statusLabel) return;
    
    std::string status = "Neurons: " + std::to_string(network->getNeuronCount()) + 
                        " | Connections: " + std::to_string(network->getConnectionCount());
    statusLabel->setText(status);
}

void GUI::render() {
    // TGUI rendering is handled automatically by the gui
}

void GUI::setGUIArea(float x, float y, float width, float height) {
    if (controlPanel) {
        // Use percentage positioning for better scaling
        controlPanel->setPosition("80%", "4%");
        controlPanel->setSize("20%", "96%");
    }
}

void GUI::refreshConnectionSliders() {
    createConnectionSliders();
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
    dialog->getRenderer()->setTitleColor(tgui::Color::White);
    
    auto label = tgui::Label::create("Select neuron to remove:");
    label->setPosition("10", "30");
    dialog->add(label);
    
    auto comboBox = tgui::ComboBox::create();
    comboBox->setPosition("10", "60");
    comboBox->setSize("200", "25");
    
    // Populate with neuron indices
    for (size_t i = 0; i < neuronCount; ++i) {
        comboBox->addItem("Neuron " + std::to_string(i + 1));
    }
    dialog->add(comboBox);
    
    auto removeButton = tgui::Button::create("Remove");
    removeButton->setPosition("10", "100");
    removeButton->setSize("80", "30");
    removeButton->onClick([this, dialog, comboBox]() {
        int selectedIndex = comboBox->getSelectedItemIndex();
        if (selectedIndex >= 0) {
            std::cout << "Removing neuron " << (selectedIndex + 1) << std::endl;
            
            if (network->removeNeuron(selectedIndex)) {
                std::cout << "Successfully removed neuron. Network now has " 
                         << network->getNeuronCount() << " neurons and " 
                         << network->getConnectionCount() << " connections" << std::endl;
                
                // Refresh GUI and visualizer
                refreshConnectionSliders();
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
    if (neuronCount < 2) {
        // Show message that we need at least 2 neurons
        auto messageDialog = tgui::ChildWindow::create("Cannot Add Connection");
        messageDialog->setSize(300, 150);
        messageDialog->setPosition("50%", "50%");
        
        auto message = tgui::Label::create("You need at least 2 neurons\nto create a connection.\n\nAdd more neurons first.");
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
    
    auto dialog = tgui::ChildWindow::create("Add Connection");
    dialog->setSize("350", "200");
    dialog->setPosition("50% - 175", "50% - 100");
    dialog->getRenderer()->setTitleColor(tgui::Color::White);
    
    auto fromLabel = tgui::Label::create("From Neuron:");
    fromLabel->setPosition("10", "30");
    dialog->add(fromLabel);
    
    auto fromCombo = tgui::ComboBox::create();
    fromCombo->setPosition("100", "30");
    fromCombo->setSize("120", "25");
    
    auto toLabel = tgui::Label::create("To Neuron:");
    toLabel->setPosition("10", "70");
    dialog->add(toLabel);
    
    auto toCombo = tgui::ComboBox::create();
    toCombo->setPosition("100", "70");
    toCombo->setSize("120", "25");
    
    auto weightLabel = tgui::Label::create("Weight:");
    weightLabel->setPosition("10", "110");
    dialog->add(weightLabel);
    
    auto weightSpin = tgui::SpinControl::create();
    weightSpin->setPosition("100", "110");
    weightSpin->setSize("120", "25");
    weightSpin->setMinimum(-1.2f);
    weightSpin->setMaximum(1.2f);
    weightSpin->setStep(0.1f);
    weightSpin->setValue(0.0f);
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
    addButton->setPosition("10", "150");
    addButton->setSize("80", "30");
    addButton->onClick([this, dialog, fromCombo, toCombo, weightSpin]() {
        int fromIndex = fromCombo->getSelectedItemIndex();
        int toIndex = toCombo->getSelectedItemIndex();
        float weight = weightSpin->getValue();
        
        if (fromIndex >= 0 && toIndex >= 0 && fromIndex != toIndex) {
            // Get the neurons
            Neuron* sourceNeuron = network->getNeuron(fromIndex);
            Neuron* targetNeuron = network->getNeuron(toIndex);
            
            if (sourceNeuron && targetNeuron) {
                // Create the connection
                auto connection = network->connect(sourceNeuron, targetNeuron, weight);
                
                std::cout << "Added connection from neuron " << (fromIndex + 1) 
                         << " to neuron " << (toIndex + 1) << " with weight " << weight << std::endl;
                std::cout << "Network now has " << network->getConnectionCount() << " connections" << std::endl;
                
                // Refresh GUI and visualizer
                refreshConnectionSliders();
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
    cancelButton->setPosition("100", "150");
    cancelButton->setSize("80", "30");
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
        
        // Clear the entire network
        network->clearNetwork();
        
        std::cout << "Network reset complete. Neurons: " << network->getNeuronCount() 
                 << ", Connections: " << network->getConnectionCount() << std::endl;
        
        // Refresh GUI and visualizer
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
                
                // Add neuron to network
                auto neuron = network->addNeuron(sampleIndex, 0.0f, threshold, 1.0f, ActivationFunction::Linear);
                
                std::cout << "Added neuron with sample: " << fullPath << std::endl;
                std::cout << "Sample index: " << sampleIndex << ", Threshold: " << threshold << std::endl;
                std::cout << "Network now has " << network->getNeuronCount() << " neurons" << std::endl;
                
                // Refresh GUI completely
                refreshConnectionSliders();
                updateStatusDisplay();
                
                // Update visualizer layout for new neuron
                if (visualizer) {
                    visualizer->refreshLayout();
                }
                
                // Show success message
                auto successDialog = tgui::ChildWindow::create("Success");
                successDialog->setSize(300, 150);
                successDialog->setPosition("50%", "50%");
                
                auto message = tgui::Label::create("Neuron added successfully!\nSample: " + selectedFile.toStdString() + "\nThreshold: " + std::to_string(threshold));
                message->setPosition(10, 40);
                message->setSize(280, 60);
                message->getRenderer()->setTextColor(tgui::Color::White);
                successDialog->add(message);
                
                auto okButton = tgui::Button::create("OK");
                okButton->setPosition(100, 110);
                okButton->setSize(100, 30);
                okButton->onPress([=]() {
                    successDialog->close();
                });
                successDialog->add(okButton);
                
                gui->add(successDialog);
                
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