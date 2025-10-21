#include "QuantizerWidget.h"
#include <sstream>
#include <iomanip>
#include <iostream>

QuantizerWidget::QuantizerWidget(Quantizer& quantizer)
    : quantizer(quantizer), widgetVisible(false), isInitialized(false), updatingFromQuantizer(false) {
}

void QuantizerWidget::initialize(tgui::Container::Ptr parent) {
    if (isInitialized) {
        return;
    }
    
    parentContainer = parent;
    
    createMainPanel();
    createEnableControls();
    createGridControls();
    createAmountControls();
    createSwingControls();
    createBPMDisplay();
    setupCallbacks();
    
    // Add to parent container
    parentContainer->add(mainPanel);
    
    // Start hidden
    setVisible(false);
    
    // Update controls to match quantizer state
    updateFromQuantizer();
    
    isInitialized = true;
}

void QuantizerWidget::createMainPanel() {
    mainPanel = tgui::Panel::create();
    mainPanel->setPosition("10%", "15%");
    mainPanel->setSize("30%", "65%");
    mainPanel->getRenderer()->setBackgroundColor(tgui::Color(25, 25, 25, 240));
    mainPanel->getRenderer()->setBorderColor(tgui::Color(100, 100, 100));
    mainPanel->getRenderer()->setBorders(2);
    
    // Title
    titleLabel = tgui::Label::create("Quantization");
    titleLabel->setPosition(10, 10);
    titleLabel->setTextSize(18);
    titleLabel->getRenderer()->setTextColor(tgui::Color(220, 220, 220));
    mainPanel->add(titleLabel);
}

void QuantizerWidget::createEnableControls() {
    // Enable checkbox
    enableCheckbox = tgui::CheckBox::create();
    enableCheckbox->setPosition(20, 50);
    enableCheckbox->setSize(20, 20);
    enableCheckbox->setChecked(quantizer.isQuantizationEnabled());
    mainPanel->add(enableCheckbox);
    
    // Enable label
    enableLabel = tgui::Label::create("Enable Quantization");
    enableLabel->setPosition(50, 50);
    enableLabel->setTextSize(14);
    enableLabel->getRenderer()->setTextColor(tgui::Color(200, 200, 200));
    mainPanel->add(enableLabel);
}

void QuantizerWidget::createGridControls() {
    // Grid resolution label
    gridLabel = tgui::Label::create("Grid Resolution:");
    gridLabel->setPosition(20, 90);
    gridLabel->setTextSize(14);
    gridLabel->getRenderer()->setTextColor(tgui::Color(200, 200, 200));
    mainPanel->add(gridLabel);
    
    // Grid resolution combo box
    gridComboBox = tgui::ComboBox::create();
    gridComboBox->setPosition(20, 115);
    gridComboBox->setSize(200, 25);
    
    // Add all grid resolution options
    gridComboBox->addItem("1/2 Note");
    gridComboBox->addItem("1/4 Note");
    gridComboBox->addItem("1/8 Note");
    gridComboBox->addItem("1/16 Note");
    gridComboBox->addItem("1/32 Note");
    gridComboBox->addItem("1/64 Note");
    
    gridComboBox->setSelectedItem(gridResolutionToString(quantizer.getGridResolution()));
    mainPanel->add(gridComboBox);
    
    // Setup callback AFTER adding to panel
    gridComboBox->onItemSelect([this](const tgui::String& item) {
        onGridResolutionChanged(item);
    });
}

void QuantizerWidget::createAmountControls() {
    // Amount label
    amountLabel = tgui::Label::create("Quantization Amount:");
    amountLabel->setPosition(20, 160);
    amountLabel->setTextSize(14);
    amountLabel->getRenderer()->setTextColor(tgui::Color(200, 200, 200));
    mainPanel->add(amountLabel);
    
    // Amount slider
    amountSlider = tgui::Slider::create(0.0f, 1.0f);
    amountSlider->setPosition(20, 185);
    amountSlider->setSize(180, 20);
    amountSlider->setValue(quantizer.getQuantizationAmount());
    amountSlider->setStep(0.01f);
    amountSlider->getRenderer()->setTrackColor(tgui::Color(60, 60, 60));
    amountSlider->getRenderer()->setThumbColor(tgui::Color(120, 180, 120));
    mainPanel->add(amountSlider);
    
    // Amount value label
    amountValueLabel = tgui::Label::create("100%");
    amountValueLabel->setPosition(210, 185);
    amountValueLabel->setTextSize(12);
    amountValueLabel->getRenderer()->setTextColor(tgui::Color(180, 180, 180));
    mainPanel->add(amountValueLabel);
}

void QuantizerWidget::createSwingControls() {
    // Swing label
    swingLabel = tgui::Label::create("Swing Factor:");
    swingLabel->setPosition(20, 230);
    swingLabel->setTextSize(14);
    swingLabel->getRenderer()->setTextColor(tgui::Color(200, 200, 200));
    mainPanel->add(swingLabel);
    
    // Swing slider
    swingSlider = tgui::Slider::create(-1.0f, 1.0f);
    swingSlider->setPosition(20, 255);
    swingSlider->setSize(180, 20);
    swingSlider->setValue(quantizer.getSwingFactor());
    swingSlider->setStep(0.01f);
    swingSlider->getRenderer()->setTrackColor(tgui::Color(60, 60, 60));
    swingSlider->getRenderer()->setThumbColor(tgui::Color(180, 120, 120));
    mainPanel->add(swingSlider);
    
    // Swing value label
    swingValueLabel = tgui::Label::create("0%");
    swingValueLabel->setPosition(210, 255);
    swingValueLabel->setTextSize(12);
    swingValueLabel->getRenderer()->setTextColor(tgui::Color(180, 180, 180));
    mainPanel->add(swingValueLabel);
}

void QuantizerWidget::createBPMDisplay() {
    // BPM label
    bpmLabel = tgui::Label::create("Tempo (BPM):");
    bpmLabel->setPosition(20, 300);
    bpmLabel->setTextSize(14);
    bpmLabel->getRenderer()->setTextColor(tgui::Color(200, 200, 200));
    mainPanel->add(bpmLabel);
    
    // BPM value label
    bpmValueLabel = tgui::Label::create("120");
    bpmValueLabel->setPosition(20, 325);
    bpmValueLabel->setTextSize(16);
    bpmValueLabel->getRenderer()->setTextColor(tgui::Color(150, 200, 150));
    mainPanel->add(bpmValueLabel);
}

void QuantizerWidget::setupCallbacks() {
    // Enable checkbox callback
    enableCheckbox->onCheck([this]() { onEnableChanged(true); });
    enableCheckbox->onUncheck([this]() { onEnableChanged(false); });
    
    // Grid resolution callback is now set up in createGridControls()
    
    // Amount slider callback
    amountSlider->onValueChange([this](float value) {
        onAmountChanged(value);
    });
    
    // Swing slider callback
    swingSlider->onValueChange([this](float value) {
        onSwingChanged(value);
    });
}

void QuantizerWidget::setVisible(bool visible) {
    widgetVisible = visible;
    if (mainPanel) {
        mainPanel->setVisible(visible);
    }
}

void QuantizerWidget::setPosition(float x, float y, float width, float height) {
    if (mainPanel) {
        mainPanel->setPosition(x, y);
        mainPanel->setSize(width, height);
    }
}

void QuantizerWidget::updateFromQuantizer() {
    if (!isInitialized) {
        return;
    }
    
    // Set flag to prevent callback loops
    updatingFromQuantizer = true;
    
    // Update enable checkbox
    enableCheckbox->setChecked(quantizer.isQuantizationEnabled());
    
    // Update grid resolution
    gridComboBox->setSelectedItem(gridResolutionToString(quantizer.getGridResolution()));
    
    // Update amount slider and label
    amountSlider->setValue(quantizer.getQuantizationAmount());
    updateAmountLabel();
    
    // Update swing slider and label
    swingSlider->setValue(quantizer.getSwingFactor());
    updateSwingLabel();
    
    // Clear flag after updates are complete
    updatingFromQuantizer = false;
    updateSwingLabel();
    
    // Update BPM display
    updateBPMDisplay();
}

void QuantizerWidget::cleanup() {
    if (parentContainer && mainPanel) {
        parentContainer->remove(mainPanel);
    }
    isInitialized = false;
}

void QuantizerWidget::onEnableChanged(bool enabled) {
    quantizer.setEnabled(enabled);
}

void QuantizerWidget::onGridResolutionChanged(const tgui::String& selectedItem) {
    // Prevent callback loops during programmatic updates
    if (updatingFromQuantizer) {
        return;
    }
    
    Quantizer::GridResolution resolution = stringToGridResolution(selectedItem.toStdString());
    quantizer.setGridResolution(resolution);
}

void QuantizerWidget::onAmountChanged(float value) {
    quantizer.setQuantizationAmount(value);
    updateAmountLabel();
}

void QuantizerWidget::onSwingChanged(float value) {
    quantizer.setSwingFactor(value);
    updateSwingLabel();
}

std::string QuantizerWidget::gridResolutionToString(Quantizer::GridResolution resolution) const {
    switch (resolution) {
        case Quantizer::GridResolution::HALF_NOTE: return "1/2 Note";
        case Quantizer::GridResolution::QUARTER_NOTE: return "1/4 Note";
        case Quantizer::GridResolution::EIGHTH_NOTE: return "1/8 Note";
        case Quantizer::GridResolution::SIXTEENTH_NOTE: return "1/16 Note";
        case Quantizer::GridResolution::THIRTY_SECOND_NOTE: return "1/32 Note";
        case Quantizer::GridResolution::SIXTY_FOURTH_NOTE: return "1/64 Note";
        default: return "1/16 Note";
    }
}

Quantizer::GridResolution QuantizerWidget::stringToGridResolution(const std::string& str) const {
    std::cout << "🎵 QuantizerWidget: stringToGridResolution converting: '" << str << "'" << std::endl;
    
    if (str == "1/2 Note") {
        std::cout << "🎵 QuantizerWidget: Matched 1/2 Note" << std::endl;
        return Quantizer::GridResolution::HALF_NOTE;
    }
    if (str == "1/4 Note") {
        std::cout << "🎵 QuantizerWidget: Matched 1/4 Note" << std::endl;
        return Quantizer::GridResolution::QUARTER_NOTE;
    }
    if (str == "1/8 Note") {
        std::cout << "🎵 QuantizerWidget: Matched 1/8 Note" << std::endl;
        return Quantizer::GridResolution::EIGHTH_NOTE;
    }
    if (str == "1/16 Note") {
        std::cout << "🎵 QuantizerWidget: Matched 1/16 Note" << std::endl;
        return Quantizer::GridResolution::SIXTEENTH_NOTE;
    }
    if (str == "1/32 Note") {
        std::cout << "🎵 QuantizerWidget: Matched 1/32 Note" << std::endl;
        return Quantizer::GridResolution::THIRTY_SECOND_NOTE;
    }
    if (str == "1/64 Note") {
        std::cout << "🎵 QuantizerWidget: Matched 1/64 Note" << std::endl;
        return Quantizer::GridResolution::SIXTY_FOURTH_NOTE;
    }
    
    std::cout << "🎵 QuantizerWidget: No match found, using default 1/16 Note" << std::endl;
    return Quantizer::GridResolution::SIXTEENTH_NOTE; // Default
}

void QuantizerWidget::updateAmountLabel() {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(0) << (quantizer.getQuantizationAmount() * 100) << "%";
    amountValueLabel->setText(stream.str());
}

void QuantizerWidget::updateSwingLabel() {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(0) << (quantizer.getSwingFactor() * 100) << "%";
    swingValueLabel->setText(stream.str());
}

void QuantizerWidget::updateBPMDisplay() {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1) << quantizer.getBPM();
    bpmValueLabel->setText(stream.str());
}