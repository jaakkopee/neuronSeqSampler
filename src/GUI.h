#pragma once
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/Widgets/Panel.hpp>
#include <TGUI/Widgets/ScrollablePanel.hpp>
#include <TGUI/Widgets/Label.hpp>
#include <TGUI/Widgets/Slider.hpp>
#include <TGUI/Widgets/MenuBar.hpp>
#include <TGUI/Widgets/ChildWindow.hpp>
#include <TGUI/Widgets/Button.hpp>
#include <TGUI/Widgets/ComboBox.hpp>
#include <TGUI/Widgets/SpinControl.hpp>
#include <TGUI/Widgets/EditBox.hpp>
#include <TGUI/Widgets/ListBox.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include <filesystem>

// Forward declarations
class NeuronNetwork;
class Visualizer;
class Recorder;

class GUI {
private:
    tgui::Gui* gui;
    sf::RenderWindow* window;
    NeuronNetwork* network;
    Visualizer* visualizer;
    Recorder* recorder;
    
    // GUI panels
    tgui::Panel::Ptr controlPanel;
    tgui::ScrollablePanel::Ptr slidersPanel;
    tgui::Label::Ptr statusLabel;
    tgui::MenuBar::Ptr menuBar;
    
    // Control elements
    std::vector<tgui::Slider::Ptr> connectionSliders;
    std::vector<tgui::Label::Ptr> connectionLabels;
    
    // Layout
    float controlPanelTopOffset = 0.0f;
    
    void createMenuBar();
    void createControlPanel();
    void createConnectionSliders();
    void updateStatusDisplay();
    
    // Menu actions
    void addNeuron();
    void removeNeuron();
    void showConnectionDialog();
    void showRemoveConnectionDialog();
    void resetNetwork();
    void showAddNeuronDialog();
    
    // Recording actions
    void startRecording();
    void stopRecording();
    void showRecordingDialog();
    
    // Sample file management
    std::vector<std::string> getSampleFiles(const std::string& directory);
    std::vector<std::string> getAllSampleDirectories();
    
    // Event handlers
    void onSliderChanged(size_t connectionIndex, float value);

public:
    GUI(tgui::Gui* tguiGui, sf::RenderWindow* renderWindow, NeuronNetwork* neuronNetwork, Visualizer* visualizerPtr = nullptr, Recorder* recorderPtr = nullptr);
    
    void initialize();
    void update();
    void render();
    
    // Layout management
    void setGUIArea(float x, float y, float width, float height);
    
    // Control methods
    void refreshConnectionSliders();
    void setSliderValue(size_t connectionIndex, float value);
};