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
class AudioManager;

class GUI {
private:
    tgui::Gui* gui;
    sf::RenderWindow* window;
    NeuronNetwork* network;
    Visualizer* visualizer;
    Recorder* recorder;
    AudioManager* audioManager;
    float* activationInterval; // Pointer to the main app's activation interval
    
    // GUI panels
    tgui::Panel::Ptr controlPanel;
    tgui::ScrollablePanel::Ptr slidersPanel;
    tgui::Label::Ptr statusLabel;
    tgui::MenuBar::Ptr menuBar;
    
    // Control elements
    std::vector<tgui::Slider::Ptr> connectionSliders;
    std::vector<tgui::Label::Ptr> connectionLabels;
    std::vector<tgui::Slider::Ptr> neuronSliders;
    std::vector<tgui::Label::Ptr> neuronLabels;
    std::vector<tgui::ComboBox::Ptr> activationFunctionCombos;
    tgui::Slider::Ptr activationIntervalSlider;
    tgui::Label::Ptr activationIntervalLabel;
    tgui::ComboBox::Ptr viewModeComboBox;
    tgui::Label::Ptr viewModeLabel;
    
    // Rhythm interpreter controls
    tgui::Panel::Ptr rhythmPanel;
    tgui::Button::Ptr rhythmEnableButton;
    tgui::Slider::Ptr rhythmGainSlider;
    tgui::Label::Ptr rhythmStatusLabel;
    tgui::Label::Ptr rhythmTempoLabel;
    std::vector<tgui::Slider::Ptr> filterFreqSliders;
    std::vector<tgui::Slider::Ptr> filterBandwidthSliders;
    std::vector<tgui::Slider::Ptr> filterGainSliders;
    std::vector<tgui::Button::Ptr> filterListenButtons; // Solo/listen buttons for each filter band
    std::vector<tgui::Label::Ptr> filterOutputDisplays; // Real-time output level displays for each filter
    std::vector<tgui::Label::Ptr> filterGainDisplays; // Display labels showing filter gain values
    // Frequency response visualization temporarily removed due to TGUI limitations
    
    // Connection matrix GUI (8 filters × N neurons)
    tgui::ScrollablePanel::Ptr connectionMatrixPanel;
    tgui::Label::Ptr matrixTitleLabel;
    std::vector<std::vector<tgui::Button::Ptr>> matrixToggleButtons; // [filter][neuron]
    std::vector<std::vector<tgui::Slider::Ptr>> matrixGainSliders;   // [filter][neuron] 
    std::vector<std::vector<tgui::Label::Ptr>> matrixGainDisplays;   // [filter][neuron] - Connection gain value displays
    std::vector<tgui::Label::Ptr> filterLabels; // Labels for filter bands
    std::vector<tgui::Label::Ptr> neuronColumnLabels; // Labels for neuron columns
    tgui::Slider::Ptr rhythmogramScaleSlider; // Vertical slider for rhythmogram scale control
    tgui::Label::Ptr rhythmogramScaleLabel; // Label showing current rhythmogram scale value
    tgui::Slider::Ptr bpmSlider; // Vertical slider for BPM control (tempo-relative frequencies)
    tgui::Label::Ptr bpmLabel; // Label showing current BPM value
    tgui::Button::Ptr autodetectTempoToggle; // Toggle for automatic tempo detection
    bool matrixVisible = true; // Connection matrix visibility state
    bool isUpdatingMatrix = false; // Flag to prevent recursive updates
    int matrixUpdateCounter = 0; // Counter to reduce update frequency
    int toggleBlockCounter = 0; // Counter to block updates after toggle interactions
    bool allowMatrixUpdates = true; // Flag to control when matrix updates are allowed
    
    // Layout
    float controlPanelTopOffset = 0.0f;
    
    void createMenuBar();
    void createControlPanel();
    void createConnectionSliders();
    void createNeuronSliders();
    void createRhythmInterpreterPanel();
    void createConnectionMatrixPanel();
    void updateStatusDisplay();
    void updateRhythmStatus();
    void updateConnectionMatrix();
    // void drawFilterResponse(size_t filterIndex); // Temporarily disabled
    
    // Menu actions
    void addNeuron();
    void removeNeuron();
    void showConnectionDialog();
    void showRemoveConnectionDialog();
    void resetNetwork();
    void showAddNeuronDialog();
    
    // Recording actions
    void startRecording();
    void startInternalRecording();
    void startExternalRecording();
    void stopRecording();
    void showRecordingDialog();
    void showInternalRecordingDialog();
    void showExternalRecordingDialog();
    
    // Sample file management
    std::vector<std::string> getSampleFiles(const std::string& directory);
    std::vector<std::string> getAllSampleDirectories();
    
    // Event handlers
    void onSliderChanged(size_t connectionIndex, float value);
    void onNeuronSliderChanged(size_t neuronIndex, float value);
    void onActivationFunctionChanged(size_t neuronIndex, const std::string& functionName);

public:
    GUI(tgui::Gui* tguiGui, sf::RenderWindow* renderWindow, NeuronNetwork* neuronNetwork, 
        Visualizer* visualizerPtr, Recorder* recorderPtr, AudioManager* audioMgr, float* activationIntervalPtr);
    
    void initialize();
    void update();
    void render();
    
    // Layout management
    void setGUIArea(float x, float y, float width, float height);
    
    // Control methods
    void refreshConnectionSliders();
    void refreshNeuronSliders();
    void refreshConnectionMatrix();
    void setSliderValue(size_t connectionIndex, float value);
    void forceMatrixUpdate(); // Allow one-time matrix update
    
    // Matrix visibility control
    void toggleMatrixVisibility();
    
    // Update frequency labels with current BPM scaling
    void updateFrequencyLabels();
};