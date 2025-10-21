#include "src/NeuronNetwork.h"
#include "src/PresetManager.h"
#include <iostream>

int main() {
    std::cout << "🛠️  Testing Preset Loading Safety" << std::endl;
    std::cout << "=================================" << std::endl;
    
    try {
        // Create a network
        NeuronNetwork network;
        
        // Try to load drum pattern preset (should work even without quantization section)
        std::cout << "\n📂 Testing drum pattern preset loading..." << std::endl;
        if (PresetManager::loadPreset(network, "presets/factory/drum_pattern.json")) {
            std::cout << "✅ Drum pattern loaded successfully!" << std::endl;
            std::cout << "   Neurons: " << network.getNeuronCount() << std::endl;
            std::cout << "   Connections: " << network.getConnectionCount() << std::endl;
        } else {
            std::cout << "❌ Failed to load drum pattern preset" << std::endl;
            return 1;
        }
        
        // Clear network and try another preset
        network.clearNetwork();
        std::cout << "\n📂 Testing user test preset loading..." << std::endl;
        if (PresetManager::loadPreset(network, "presets/user/Test.json")) {
            std::cout << "✅ Test preset loaded successfully!" << std::endl;
            std::cout << "   Neurons: " << network.getNeuronCount() << std::endl;
            std::cout << "   Connections: " << network.getConnectionCount() << std::endl;
        } else {
            std::cout << "❌ Failed to load test preset" << std::endl;
            return 1;
        }
        
        std::cout << "\n🎉 All preset loading tests passed!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Exception during test: " << e.what() << std::endl;
        return 1;
    }
}
