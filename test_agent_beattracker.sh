#!/bin/bash

# Simple unit test for Agent-based BeatTracker components
# This test verifies basic compilation and class instantiation

echo "=== Agent-Based BeatTracker Test ==="
echo ""

# Create a simple test file
cat > /tmp/test_agent_beattracker.cpp << 'EOF'
#include "../src/BeatTracker.h"
#include <iostream>
#include <vector>
#include <deque>

int main() {
    std::cout << "Testing Agent class..." << std::endl;
    
    // Test Agent creation
    Agent agent(120.0f, 0.0f, 44100);
    std::cout << "  Agent created with tempo: " << agent.getTempo() 
              << " BPM, phase: " << agent.getPhase() << std::endl;
    
    // Test phase advancement
    agent.advancePhase(512);
    std::cout << "  After advancing 512 samples, phase: " << agent.getPhase() << std::endl;
    
    // Test scoring with dummy data
    std::deque<float> dummyOnsets(1000, 0.1f);
    std::vector<Pattern> dummyPatterns;
    float score = agent.scoreHypothesis(dummyOnsets, dummyPatterns);
    std::cout << "  Agent confidence score: " << score << std::endl;
    
    std::cout << "\nTesting PatternFinder class..." << std::endl;
    
    // Test PatternFinder creation
    PatternFinder finder(44100);
    std::cout << "  PatternFinder created" << std::endl;
    
    // Test pattern finding with dummy data
    std::deque<float> onsets(5000, 0.0f);
    // Add some peaks
    for (size_t i = 0; i < onsets.size(); i += 500) {
        onsets[i] = 0.8f;
    }
    
    auto patterns = finder.findPatterns(onsets, 0.0f, 1000.0f);
    std::cout << "  Found " << patterns.size() << " pattern(s)" << std::endl;
    
    std::cout << "\nTesting BeatTracker with agent system..." << std::endl;
    
    // Test BeatTracker creation
    BeatTracker tracker(44100, 512);
    tracker.setEnabled(true);
    std::cout << "  BeatTracker created and enabled" << std::endl;
    
    // Simulate some updates
    std::vector<float> networkFirings(16, 0.0f);
    std::vector<float> inputOnsets(16, 0.0f);
    
    for (int i = 0; i < 10; i++) {
        // Add some activity
        if (i % 2 == 0) {
            networkFirings[0] = 1.0f;
            inputOnsets[0] = 0.7f;
        } else {
            networkFirings[0] = 0.0f;
            inputOnsets[0] = 0.1f;
        }
        
        tracker.update(networkFirings, inputOnsets);
    }
    
    std::cout << "  Current phase: " << tracker.getCurrentPhase() << std::endl;
    std::cout << "  Detected tempo: " << tracker.getDetectedTempo() << " BPM" << std::endl;
    std::cout << "  Phase confidence: " << tracker.getPhaseConfidence() << std::endl;
    
    // Test reset
    tracker.reset();
    std::cout << "  After reset - phase: " << tracker.getCurrentPhase() 
              << ", tempo: " << tracker.getDetectedTempo() << std::endl;
    
    std::cout << "\n✓ All basic tests passed!" << std::endl;
    return 0;
}
EOF

# Compile the test
echo "Compiling test..."
g++ -std=c++17 -o /tmp/test_agent_beattracker /tmp/test_agent_beattracker.cpp src/BeatTracker.cpp -Isrc

if [ $? -eq 0 ]; then
    echo "Compilation successful!"
    echo ""
    echo "Running test..."
    /tmp/test_agent_beattracker
    TEST_RESULT=$?
    
    echo ""
    if [ $TEST_RESULT -eq 0 ]; then
        echo "✓ Test completed successfully!"
    else
        echo "✗ Test failed with exit code $TEST_RESULT"
        exit 1
    fi
else
    echo "✗ Compilation failed!"
    exit 1
fi

# Cleanup
rm -f /tmp/test_agent_beattracker /tmp/test_agent_beattracker.cpp

echo ""
echo "Test complete."
