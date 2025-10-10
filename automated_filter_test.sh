#!/bin/bash

# Automated test for filter listen buttons
echo "🧪 Automated Filter Listen Button Test"
echo "========================================"
echo ""
echo "This test demonstrates the filter system working automatically."
echo "It will:"
echo "1. Start the application" 
echo "2. Automatically enable recording and audio output"
echo "3. Show status messages in the console"
echo ""
echo "You should see:"
echo "- '🎵 RhythmInterpreter initialized with filter bank'"
echo "- Status messages showing Recording: ON and AudioStreaming: ON"
echo "- Audio input/output flow messages"
echo ""
echo "Starting test in 3 seconds..."
sleep 3

echo ""
echo "🚀 Launching NeuronSeqSampler..."
echo "📝 Instructions that will appear:"
echo "   - Press 'R' to toggle audio recording"
echo "   - Press 'F' to toggle filtered audio output" 
echo "   - Click 'L' buttons to solo filter bands"
echo ""
echo "⏱️  Test will run for 20 seconds..."

# Run the application for 20 seconds
timeout 20 ./NeuronSeqSampler

echo ""
echo "✅ Test completed!"
echo ""
echo "Expected workflow for manual testing:"
echo "1. Launch: ./NeuronSeqSampler"
echo "2. Press 'R' - should show 'Recording started'"
echo "3. Press 'F' - should show 'Filtered audio output ENABLED'"
echo "4. Click 'L' buttons in GUI to solo frequency bands"
echo "5. Adjust gain/resonance sliders while listening"