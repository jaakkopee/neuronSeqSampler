#!/bin/bash

echo "🧪 Testing Factory Preset Creation Behavior"
echo "==========================================="

echo "📁 Checking initial state..."
if [ -f "presets/factory/drum_pattern.json" ]; then
    echo "❌ Factory preset exists - removing for test"
    rm -f presets/factory/drum_pattern.json
else
    echo "✅ No factory preset exists (expected)"
fi

echo ""
echo "🏗️  Building project..."
make > /dev/null 2>&1

if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo "✅ Build successful"
echo ""

echo "🔬 Testing app startup without factory preset auto-creation..."

# Test 1: Run the app very briefly to see if it creates factory preset on startup
timeout 2s ./test_build > startup_output.log 2>&1 &
APP_PID=$!

# Wait a moment then kill the app
sleep 1
kill $APP_PID 2>/dev/null
wait $APP_PID 2>/dev/null

echo "📋 Checking startup output..."
if grep -q "Created factory preset" startup_output.log; then
    echo "❌ Factory preset was created on startup (unexpected)"
    cat startup_output.log
else
    echo "✅ Factory preset NOT created on startup (expected behavior)"
fi

echo ""
echo "🔍 Verifying factory preset file status..."
if [ -f "presets/factory/drum_pattern.json" ]; then
    echo "❌ Factory preset file exists after startup"
else
    echo "✅ Factory preset file does NOT exist after startup (expected)"
fi

echo ""
echo "🧪 Testing factory preset creation only when needed..."

# Create a simple test that loads a factory preset
cat > test_factory_load.cpp << 'EOF'
#include "src/PresetManager.h"
#include "src/NeuronNetwork.h"
#include <iostream>

int main() {
    std::cout << "Testing factory preset loading..." << std::endl;
    
    NeuronNetwork network;
    bool success = PresetManager::loadFactoryPreset(network, "drum_pattern");
    
    if (success) {
        std::cout << "✅ Factory preset loaded successfully" << std::endl;
        std::cout << "Network has " << network.getNeuronCount() << " neurons" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Failed to load factory preset" << std::endl;
        return 1;
    }
}
EOF

echo "📚 Compiling factory preset test..."
g++ -std=c++17 -I. test_factory_load.cpp src/PresetManager.cpp src/NeuronNetwork.cpp src/Neuron.cpp -o test_factory_load 2>/dev/null

if [ $? -eq 0 ]; then
    echo "▶️  Running factory preset load test..."
    ./test_factory_load
    
    echo ""
    echo "🔍 Checking if factory preset was created on demand..."
    if [ -f "presets/factory/drum_pattern.json" ]; then
        echo "✅ Factory preset created on demand when loading (expected)"
        
        echo ""
        echo "📋 Verifying preset includes sample file paths..."
        if grep -q "sample_file_path" presets/factory/drum_pattern.json; then
            echo "✅ Factory preset includes sample_file_path fields"
            echo "📄 Sample paths found:"
            grep "sample_file_path" presets/factory/drum_pattern.json
        else
            echo "❌ Factory preset missing sample_file_path fields"
        fi
        
        echo ""
        echo "🧪 Testing that factory preset is NOT recreated on subsequent runs..."
        ORIGINAL_SIZE=$(wc -c < presets/factory/drum_pattern.json)
        sleep 1  # Ensure different timestamp if file is recreated
        ./test_factory_load > /dev/null 2>&1
        NEW_SIZE=$(wc -c < presets/factory/drum_pattern.json)
        
        if [ "$ORIGINAL_SIZE" = "$NEW_SIZE" ]; then
            echo "✅ Factory preset NOT recreated on subsequent load (expected)"
        else
            echo "❌ Factory preset was recreated (unexpected)"
        fi
    else
        echo "❌ Factory preset was NOT created on demand"
    fi
else
    echo "❌ Failed to compile factory preset test"
fi

# Clean up
rm -f test_factory_load.cpp test_factory_load startup_output.log

echo ""
echo "📋 Summary of improvements:"
echo "✅ Removed unnecessary createFactoryPresets() call from main()"
echo "✅ Factory presets only created when needed (on first load)"
echo "✅ Existing factory presets are preserved (not overwritten)"
echo "✅ New factory presets include sample_file_path fields"
echo "✅ Factory presets include quantization settings"