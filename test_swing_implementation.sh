#!/bin/bash

# Test script for swing implementation
echo "🎵 Testing Swing Implementation"
echo "================================="

# Create a simple test program to verify swing works
cat > test_swing.cpp << 'EOF'
#include "src/Quantizer.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "🎵 Testing Swing Implementation" << std::endl;
    std::cout << "===============================" << std::endl;
    
    // Create quantizer at 120 BPM
    Quantizer quantizer(120.0f, 44100);
    quantizer.setGridResolution(Quantizer::GridResolution::EIGHTH_NOTE);
    quantizer.setQuantizationAmount(1.0f);
    quantizer.setEnabled(true);
    
    // Test different swing values
    float swingValues[] = {0.0f, 0.5f, 1.0f};
    const char* swingNames[] = {"Straight (0%)", "Medium (50%)", "Full Triplet (100%)"};
    
    for (int i = 0; i < 3; i++) {
        std::cout << "\n📊 Testing " << swingNames[i] << " swing:" << std::endl;
        quantizer.setSwingFactor(swingValues[i]);
        
        // Test some off-beat times (these should be affected by swing)
        double testTimes[] = {0.25, 0.75, 1.25, 1.75}; // Off-beats in a 120 BPM context
        
        for (double time : testTimes) {
            double nearest = quantizer.getNearestGridPoint(time);
            std::cout << "  Time " << std::fixed << std::setprecision(3) << time 
                      << "s → " << nearest << "s (offset: " << (nearest - time) << "s)" << std::endl;
        }
    }
    
    std::cout << "\n✅ Swing test completed!" << std::endl;
    return 0;
}
EOF

# Compile the test
echo "🔨 Compiling swing test..."
g++ -I. test_swing.cpp src/Quantizer.cpp -o test_swing -std=c++17

if [ $? -eq 0 ]; then
    echo "✅ Compilation successful!"
    echo ""
    echo "🎵 Running swing test..."
    ./test_swing
    
    # Clean up
    rm -f test_swing.cpp test_swing
    echo ""
    echo "🧹 Test files cleaned up"
else
    echo "❌ Compilation failed!"
    exit 1
fi