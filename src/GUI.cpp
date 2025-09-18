#include "GUI.h"
#include "NeuronNetwork.h"
#include "Connection.h"
#include "AudioManager.h"
#include "Visualizer.h"
#include <iostream>
#include <filesystem>

GUI::GUI(tgui::Gui* tguiGui, sf::RenderWindow* renderWindow, NeuronNetwork* neuronNetwork, Visualizer* visualizerPtr)
    : gui(tguiGui)
    , window(renderWindow)
    , network(neuronNetwork)
    , visualizer(visualizerPtr)
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
    
    // Connect menu actions
    menuBar->connectMenuItem("Network", "Add Neuron", [this]() { this->addNeuron(); });
    menuBar->connectMenuItem("Network", "Remove Neuron", [this]() { this->removeNeuron(); });
    menuBar->connectMenuItem("Network", "Add Connection", [this]() { this->showConnectionDialog(); });
    menuBar->connectMenuItem("Network", "Remove Connection", [this]() { this->showRemoveConnectionDialog(); });
    menuBar->connectMenuItem("Network", "Reset Network", [this]() { this->resetNetwork(); });
    
    // Adjust control panel position to account for menu bar
    controlPanelTopOffset = 4.0f; // 4% for menu bar
}

void GUI::createControlPanel() {
    // Create main control panel
    controlPanel = tgui::Panel::create({"5%", "96%"}); // Reduced height for menu bar
    controlPanel->setPosition("96%", "4%"); // Moved down for menu bar
    controlPanel->setSize({"5%", "96%"});
    controlPanel->getRenderer()->setBackgroundColor(tgui::Color(50, 50, 50, 180));
    gui->add(controlPanel, "ControlPanel");
    
    // Status label
    statusLabel = tgui::Label::create("Neuron Sequence Sampler");
    statusLabel->setPosition("5%", "2%");
    statusLabel->setTextSize(14);
    statusLabel->getRenderer()->setTextColor(tgui::Color::White);
    controlPanel->add(statusLabel, "StatusLabel");
    
    // Create scrollable panel for sliders - adjusted for smaller control panel
    slidersPanel = tgui::ScrollablePanel::create({"50%", "85%"});
    slidersPanel->setPosition("10%", "10%");
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
        
        // Create slider - made longer
        auto slider = tgui::Slider::create();
        slider->setPosition(30, yPos);
        slider->setSize(240, 16);
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
        valueLabel->setPosition(115, yPos);
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
        controlPanel->setPosition(x, y);
        controlPanel->setSize(width, height);
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