#!/bin/bash

echo "🎵 Practical Swing Test for NeuronSeqSampler"
echo "============================================="
echo ""
echo "This script tests swing functionality in the actual application."
echo ""
echo "📋 Test Instructions:"
echo "1. Build and run the neuronSeqSampler"
echo "2. Press 'Q' to enable quantization"
echo "3. Set grid to 1/8 notes using the dropdown"
echo "4. Set quantization amount to 100%"
echo "5. Adjust swing slider from 0% to 100%"
echo "6. Play a steady beat and listen for the timing changes"
echo ""
echo "🎯 Expected Results:"
echo "• 0% swing = Straight, evenly spaced triggering"
echo "• 50% swing = Subtle groove, off-beats slightly delayed"
echo "• 100% swing = Triplet feel, off-beats at 2/3 position"
echo ""
echo "🔨 Building the application now..."

# Build the project
make clean && make

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Build successful!"
    echo ""
    echo "🚀 You can now run: ./neuronSeqSampler"
    echo ""
    echo "📝 Swing Test Checklist:"
    echo "□ Press 'Q' to toggle quantization ON"
    echo "□ Select '1/8 Note' from grid dropdown"
    echo "□ Set quantization amount to 100%"
    echo "□ Adjust swing from 0% to 100%"
    echo "□ Listen for timing changes in the rhythm"
    echo ""
    echo "🎵 The swing parameter should now transform straight timing"
    echo "   into triplet rhythm at 100% swing!"
else
    echo ""
    echo "❌ Build failed! Please check for compilation errors."
    exit 1
fi