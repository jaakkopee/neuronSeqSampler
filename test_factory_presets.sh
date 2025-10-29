#!/bin/bash

echo "🏭 Testing Factory Presets Generation"
echo "===================================="

echo "📁 Current factory preset directory status:"
ls -la presets/factory/

echo ""
echo "🔧 Triggering factory preset creation..."

# Create a simple test that calls the factory preset function
cat > test_factory_creation.cpp << 'EOF'
#include "src/PresetManager.h"
#include <iostream>

int main() {
    std::cout << "Testing factory preset creation..." << std::endl;
    
    bool success = PresetManager::createFactoryPresets();
    
    if (success) {
        std::cout << "✅ Factory preset creation completed successfully" << std::endl;
    } else {
        std::cout << "❌ Factory preset creation failed" << std::endl;
    }
    
    return success ? 0 : 1;
}
EOF

echo "📚 Compiling factory test..."
g++ -std=c++17 -I. test_factory_creation.cpp src/PresetManager.cpp -o test_factory_creation 2>/dev/null

if [ $? -eq 0 ]; then
    echo "▶️  Running factory preset creation test..."
    ./test_factory_creation
    
    echo ""
    echo "📋 Checking created factory presets..."
    if [ -d "presets/factory" ]; then
        echo "Factory preset files:"
        for file in presets/factory/*.json; do
            if [ -f "$file" ]; then
                filename=$(basename "$file" .json)
                size=$(wc -c < "$file")
                echo "  ✅ $filename - ${size} bytes"
                
                # Check preset info
                if command -v jq >/dev/null 2>&1; then
                    name=$(jq -r '.preset_info.name' "$file" 2>/dev/null)
                    description=$(jq -r '.preset_info.description' "$file" 2>/dev/null)
                    echo "     📝 Name: $name"
                    echo "     📄 Description: $description"
                    echo ""
                fi
            fi
        done
    else
        echo "❌ Factory preset directory not found"
    fi
else
    echo "❌ Failed to compile factory test"
    
    # Try alternative: Use the main app to trigger creation
    echo ""
    echo "🔄 Alternative: Testing via loadFactoryPreset call..."
    
    cat > test_load_factory.cpp << 'EOF'
#include "src/PresetManager.h"
#include "src/NeuronNetwork.h"
#include <iostream>

int main() {
    std::cout << "Testing factory preset loading (which triggers creation)..." << std::endl;
    
    NeuronNetwork network(nullptr); // Pass nullptr for AudioManager
    
    // Try to load a preset - this should trigger creation if needed
    bool success = PresetManager::loadFactoryPreset(network, "drum_pattern");
    
    if (success) {
        std::cout << "✅ Factory preset loaded successfully" << std::endl;
    } else {
        std::cout << "❌ Factory preset loading failed" << std::endl;
    }
    
    return success ? 0 : 1;
}
EOF

    g++ -std=c++17 -I. test_load_factory.cpp src/PresetManager.cpp src/NeuronNetwork.cpp src/Neuron.cpp -o test_load_factory 2>/dev/null
    
    if [ $? -eq 0 ]; then
        ./test_load_factory
        
        echo ""
        echo "📋 Checking created factory presets..."
        ls -la presets/factory/
    else
        echo "❌ Could not compile load test either"
    fi
fi

# Clean up test files
rm -f test_factory_creation.cpp test_factory_creation test_load_factory.cpp test_load_factory

echo ""
echo "🎯 Factory Presets Summary:"
echo "1. 🥁 Basic Drum Pattern - Simple 3-neuron beginner-friendly setup"
echo "2. 🕸️  Complex Network - 5-neuron interconnected with mixed activations"  
echo "3. 📈 Sigmoid Cascade - 4-neuron chain with smooth sigmoid dynamics"
echo "4. ⚡ High Energy - Fast-paced with low thresholds and rapid activations"
echo "5. 🌊 Ambient Flow - Slow, organic rhythms with Tanh activations"
echo "6. 🧪 Experimental - Unusual mixed setup for creative exploration"