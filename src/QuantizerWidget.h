#pragma once
#include <TGUI/TGUI.hpp>
#include "Quantizer.h"

/**
 * @brief GUI widget for quantization controls
 * 
 * The QuantizerWidget provides a user interface for controlling all aspects
 * of musical quantization including grid resolution, amount, and swing.
 */
class QuantizerWidget {
public:
    /**
     * @brief Construct a new Quantizer Widget
     * @param quantizer Reference to the quantizer to control
     */
    QuantizerWidget(Quantizer& quantizer);
    
    /**
     * @brief Initialize the widget and add it to a parent container
     * @param parent Parent GUI container
     */
    void initialize(tgui::Container::Ptr parent);
    
    /**
     * @brief Update widget visibility
     * @param visible Whether the widget should be visible
     */
    void setVisible(bool visible);
    
    /**
     * @brief Check if widget is currently visible
     * @return True if widget is visible
     */
    bool isVisible() const { return widgetVisible; }
    
    /**
     * @brief Toggle widget visibility
     */
    void toggleVisibility() { setVisible(!widgetVisible); }
    
    /**
     * @brief Update widget position and size
     * @param x X position
     * @param y Y position  
     * @param width Widget width
     * @param height Widget height
     */
    void setPosition(float x, float y, float width, float height);
    
    /**
     * @brief Update all widget controls to reflect current quantizer state
     */
    void updateFromQuantizer();
    
    /**
     * @brief Clean up widget resources
     */
    void cleanup();

private:
    // ========================= CORE REFERENCES =========================
    Quantizer& quantizer;           // Reference to the quantizer being controlled
    tgui::Container::Ptr parentContainer; // Parent GUI container
    
    // ========================= WIDGET STATE =========================
    bool widgetVisible;             // Current visibility state
    bool isInitialized;             // Whether widget has been initialized
    bool updatingFromQuantizer;     // Flag to prevent callback loops during updates
    
    // ========================= GUI COMPONENTS =========================
    tgui::Panel::Ptr mainPanel;        // Main container panel
    tgui::Label::Ptr titleLabel;       // Widget title
    
    // Enable/Disable controls
    tgui::CheckBox::Ptr enableCheckbox;
    tgui::Label::Ptr enableLabel;
    
    // Grid resolution controls
    tgui::ComboBox::Ptr gridComboBox;
    tgui::Label::Ptr gridLabel;
    
    // Quantization amount controls
    tgui::Slider::Ptr amountSlider;
    tgui::Label::Ptr amountLabel;
    tgui::Label::Ptr amountValueLabel;
    
    // Swing controls
    tgui::Slider::Ptr swingSlider;
    tgui::Label::Ptr swingLabel;
    tgui::Label::Ptr swingValueLabel;
    
    // BPM display (read-only, synced with main BPM)
    tgui::Label::Ptr bpmLabel;
    tgui::Label::Ptr bpmValueLabel;
    
    // ========================= WIDGET CREATION =========================
    
    /**
     * @brief Create the main panel container
     */
    void createMainPanel();
    
    /**
     * @brief Create enable/disable controls
     */
    void createEnableControls();
    
    /**
     * @brief Create grid resolution selection
     */
    void createGridControls();
    
    /**
     * @brief Create quantization amount controls
     */
    void createAmountControls();
    
    /**
     * @brief Create swing factor controls
     */
    void createSwingControls();
    
    /**
     * @brief Create BPM display
     */
    void createBPMDisplay();
    
    /**
     * @brief Setup all event callbacks
     */
    void setupCallbacks();
    
    // ========================= EVENT HANDLERS =========================
    
    /**
     * @brief Handle enable/disable checkbox changes
     * @param enabled New enable state
     */
    void onEnableChanged(bool enabled);
    
    /**
     * @brief Handle grid resolution selection changes
     * @param selectedItem Selected grid resolution text
     */
    void onGridResolutionChanged(const tgui::String& selectedItem);
    
    /**
     * @brief Handle quantization amount slider changes
     * @param value New amount value (0.0-1.0)
     */
    void onAmountChanged(float value);
    
    /**
     * @brief Handle swing factor slider changes
     * @param value New swing value (-1.0 to 1.0)
     */
    void onSwingChanged(float value);
    
    // ========================= UTILITY METHODS =========================
    
    /**
     * @brief Convert grid resolution enum to display string
     * @param resolution Grid resolution enum value
     * @return Human-readable string
     */
    std::string gridResolutionToString(Quantizer::GridResolution resolution) const;
    
    /**
     * @brief Convert display string to grid resolution enum
     * @param str Display string
     * @return Grid resolution enum value
     */
    Quantizer::GridResolution stringToGridResolution(const std::string& str) const;
    
    /**
     * @brief Update value label for amount slider
     */
    void updateAmountLabel();
    
    /**
     * @brief Update value label for swing slider
     */
    void updateSwingLabel();
    
    /**
     * @brief Update BPM display from quantizer
     */
    void updateBPMDisplay();
};