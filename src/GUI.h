#pragma once
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/Widgets/Panel.hpp>
#include <TGUI/Widgets/ScrollablePanel.hpp>
#include <TGUI/Widgets/Label.hpp>
#include <TGUI/Widgets/Slider.hpp>
#include <SFML/Graphics.hpp>
#include <memory>

// Forward declarations
class NeuronNetwork;

class GUI {
private:
    tgui::Gui* gui;
    sf::RenderWindow* window;
    NeuronNetwork* network;
    
    // GUI panels
    tgui::Panel::Ptr controlPanel;
    tgui::ScrollablePanel::Ptr slidersPanel;
    tgui::Label::Ptr statusLabel;
    
    // Control elements
    std::vector<tgui::Slider::Ptr> connectionSliders;
    std::vector<tgui::Label::Ptr> connectionLabels;
    
    void createControlPanel();
    void createConnectionSliders();
    void updateStatusDisplay();
    
    // Event handlers
    void onSliderChanged(size_t connectionIndex, float value);

public:
    GUI(tgui::Gui* tguiGui, sf::RenderWindow* renderWindow, NeuronNetwork* neuronNetwork);
    
    void initialize();
    void update();
    void render();
    
    // Layout management
    void setGUIArea(float x, float y, float width, float height);
    
    // Control methods
    void refreshConnectionSliders();
    void setSliderValue(size_t connectionIndex, float value);
};