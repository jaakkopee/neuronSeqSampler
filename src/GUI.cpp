#include "GUI.h"
#include "NeuronNetwork.h"
#include "Connection.h"
#include <iostream>

GUI::GUI(tgui::Gui* tguiGui, sf::RenderWindow* renderWindow, NeuronNetwork* neuronNetwork)
    : gui(tguiGui)
    , window(renderWindow)
    , network(neuronNetwork)
{
}

void GUI::initialize() {
    createControlPanel();
    createConnectionSliders();
}

void GUI::createControlPanel() {
    // Create main control panel - made even smaller
    controlPanel = tgui::Panel::create({"25%", "100%"});
    controlPanel->setPosition("75%", "0%");
    controlPanel->getRenderer()->setBackgroundColor(tgui::Color(50, 50, 50, 180));
    gui->add(controlPanel, "ControlPanel");
    
    // Status label
    statusLabel = tgui::Label::create("Neuron Sequence Sampler");
    statusLabel->setPosition("5%", "2%");
    statusLabel->setTextSize(14);
    statusLabel->getRenderer()->setTextColor(tgui::Color::White);
    controlPanel->add(statusLabel, "StatusLabel");
    
    // Create scrollable panel for sliders - adjusted for smaller control panel
    slidersPanel = tgui::ScrollablePanel::create({"90%", "85%"});
    slidersPanel->setPosition("5%", "10%");
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
        slider->setPosition(25, yPos);
        slider->setSize(120, 16);
        slider->setMinimum(-1.2f);
        slider->setMaximum(1.2f);
        slider->setStep(0.1f);  // 24 steps: 2.4 range / 0.1 step = 24 steps
        slider->setValue(conn->getWeight());
        
        // Connect slider to callback
        slider->onValueChange([this, i](float value) {
            this->onSliderChanged(i, value);
        });
        
        slidersPanel->add(slider);
        connectionSliders.push_back(slider);
        
        // Value label
        auto valueLabel = tgui::Label::create(std::to_string(conn->getWeight()));
        valueLabel->setPosition(150, yPos);
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