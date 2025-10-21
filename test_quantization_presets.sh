#!/bin/bash

echo "🎵 Testing Quantization Preset System"
echo "====================================="
echo ""

# Create a simple test program to test preset saving/loading with quantization
cat > test_quantization_presets.cpp << 'EOF'
#include "src/NeuronNetwork.h"
#include "src/Quantizer.h"
#include "src/PresetManager.h"
#include <iostream>
#include <filesystem>

int main() {
    std::cout << "🎵 Testing Quantization Preset System" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    // Create a network with quantizer
    NeuronNetwork network;
    
    // Create and configure quantizer
    auto quantizer = std::make_unique<Quantizer>(140.0f, 44100);
    quantizer->setEnabled(true);
    quantizer->setGridResolution(Quantizer::GridResolution::EIGHTH_NOTE);
    quantizer->setQuantizationAmount(0.75f);
    quantizer->setSwingFactor(0.5f);
    
    // Connect quantizer to network
    network.setQuantizer(quantizer.get());
    
    // Add some neurons for completeness
    network.addNeuron(1, 0.0f, 1.0f, 0.5f, 0.0f);
    network.addNeuron(2, 0.0f, 1.0f, 0.5f, 0.1f);
    
    std::cout << "\n🔧 Original quantizer settings:" << std::endl;
    std::cout << "   Enabled: " << quantizer->isQuantizationEnabled() << std::endl;
    std::cout << "   Grid: " << static_cast<int>(quantizer->getGridResolution()) << std::endl;
    std::cout << "   Amount: " << quantizer->getQuantizationAmount() << std::endl;
    std::cout << "   Swing: " << quantizer->getSwingFactor() << std::endl;
    std::cout << "   BPM: " << quantizer->getBPM() << std::endl;
    
    // Save preset
    PresetManager::PresetInfo info;
    info.name = "Quantization Test";
    info.description = "Test preset with quantization settings";
    info.author = "Test Script";
    
    std::string testFile = "test_quantization_preset.json";
    std::cout << "\n💾 Saving preset..." << std::endl;
    if (PresetManager::savePreset(network, testFile, info)) {
        std::cout << "✅ Preset saved successfully" << std::endl;
    } else {
        std::cout << "❌ Failed to save preset" << std::endl;
        return 1;
    }
    
    // Create new network and quantizer for loading
    NeuronNetwork loadNetwork;
    auto loadQuantizer = std::make_unique<Quantizer>(120.0f, 44100); // Different defaults
    loadQuantizer->setEnabled(false);
    loadQuantizer->setGridResolution(Quantizer::GridResolution::QUARTER_NOTE);
    loadQuantizer->setQuantizationAmount(0.0f);
    loadQuantizer->setSwingFactor(0.0f);
    loadNetwork.setQuantizer(loadQuantizer.get());
    
    std::cout << "\n🔧 Before loading - quantizer settings:" << std::endl;
    std::cout << "   Enabled: " << loadQuantizer->isQuantizationEnabled() << std::endl;
    std::cout << "   Grid: " << static_cast<int>(loadQuantizer->getGridResolution()) << std::endl;
    std::cout << "   Amount: " << loadQuantizer->getQuantizationAmount() << std::endl;
    std::cout << "   Swing: " << loadQuantizer->getSwingFactor() << std::endl;
    std::cout << "   BPM: " << loadQuantizer->getBPM() << std::endl;
    
    // Load preset
    std::cout << "\n📂 Loading preset..." << std::endl;
    if (PresetManager::loadPreset(loadNetwork, testFile)) {
        std::cout << "✅ Preset loaded successfully" << std::endl;
        
        std::cout << "\n🔧 After loading - quantizer settings:" << std::endl;
        std::cout << "   Enabled: " << loadQuantizer->isQuantizationEnabled() << std::endl;
        std::cout << "   Grid: " << static_cast<int>(loadQuantizer->getGridResolution()) << std::endl;
        std::cout << "   Amount: " << loadQuantizer->getQuantizationAmount() << std::endl;
        std::cout << "   Swing: " << loadQuantizer->getSwingFactor() << std::endl;
        std::cout << "   BPM: " << loadQuantizer->getBPM() << std::endl;
        
        // Verify settings match
        bool success = true;
        if (!loadQuantizer->isQuantizationEnabled()) {
            std::cout << "❌ Enabled state not restored" << std::endl;
            success = false;
        }
        if (static_cast<int>(loadQuantizer->getGridResolution()) != 2) { // EIGHTH_NOTE = 2
            std::cout << "❌ Grid resolution not restored" << std::endl;
            success = false;
        }
        if (std::abs(loadQuantizer->getQuantizationAmount() - 0.75f) > 0.01f) {
            std::cout << "❌ Quantization amount not restored" << std::endl;
            success = false;
        }
        if (std::abs(loadQuantizer->getSwingFactor() - 0.5f) > 0.01f) {
            std::cout << "❌ Swing factor not restored" << std::endl;
            success = false;
        }
        if (std::abs(loadQuantizer->getBPM() - 140.0f) > 0.1f) {
            std::cout << "❌ BPM not restored" << std::endl;
            success = false;
        }
        
        if (success) {
            std::cout << "\n🎉 All quantization settings restored correctly!" << std::endl;
        } else {
            std::cout << "\n❌ Some quantization settings were not restored correctly" << std::endl;
        }
        
    } else {
        std::cout << "❌ Failed to load preset" << std::endl;
        return 1;
    }
    
    // Clean up test file
    std::filesystem::remove(testFile);
    std::cout << "\n🧹 Test file cleaned up" << std::endl;
    
    return 0;
}
EOF

# Compile the test
echo "🔨 Compiling quantization preset test..."
g++ -I. test_quantization_presets.cpp src/NeuronNetwork.cpp src/Neuron.cpp src/Connection.cpp src/Quantizer.cpp src/PresetManager.cpp src/RhythmInterpreter.cpp -o test_quantization_presets -std=c++17 -lsfml-audio

if [ $? -eq 0 ]; then
    echo "✅ Compilation successful!"
    echo ""
    echo "🎵 Running quantization preset test..."
    ./test_quantization_presets
    
    # Clean up
    rm -f test_quantization_presets.cpp test_quantization_presets
    echo ""
    echo "🧹 Test files cleaned up"
else
    echo "❌ Compilation failed!"
    exit 1
fi