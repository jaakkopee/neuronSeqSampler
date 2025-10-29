#include "PresetManager.h"
#include "NeuronNetwork.h"
#include <iostream>

int main() {
    std::cout << "🏭 Creating factory presets directory..." << std::endl;
    PresetManager::createPresetDirectory();
    
    NeuronNetwork network;
    
    std::cout << "📄 Attempting to load drum_pattern preset..." << std::endl;
    if (PresetManager::loadFactoryPreset(network, "drum_pattern")) {
        std::cout << "✅ Successfully loaded drum_pattern preset!" << std::endl;
    } else {
        std::cout << "❌ Failed to load drum_pattern preset!" << std::endl;
    }
    
    std::cout << "📄 Attempting to load complex_network preset..." << std::endl;
    if (PresetManager::loadFactoryPreset(network, "complex_network")) {
        std::cout << "✅ Successfully loaded complex_network preset!" << std::endl;
    } else {
        std::cout << "❌ Failed to load complex_network preset!" << std::endl;
    }
    
    std::cout << "📄 Attempting to load ambient_flow preset..." << std::endl;
    if (PresetManager::loadFactoryPreset(network, "ambient_flow")) {
        std::cout << "✅ Successfully loaded ambient_flow preset!" << std::endl;
    } else {
        std::cout << "❌ Failed to load ambient_flow preset!" << std::endl;
    }
    
    return 0;
}