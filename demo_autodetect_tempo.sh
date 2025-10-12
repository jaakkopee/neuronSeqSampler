#!/bin/bash

# NeuronSeqSampler - Autodetect Tempo Feature Demo
# This script demonstrates the new automatic tempo detection toggle

echo "🎵 NeuronSeqSampler - Autodetect Tempo Demo"
echo "=========================================="
echo ""
echo "NEW FEATURE: Autodetect Tempo Toggle"
echo "• Located below the BPM slider in the connection matrix panel"
echo "• When ON: BPM automatically tracks detected tempo from audio input"
echo "• When OFF: BPM can be manually controlled via slider"
echo ""
echo "USAGE INSTRUCTIONS:"
echo "1. Open the connection matrix (press 'M' if not visible)"
echo "2. Look for the 'Autodetect Tempo' button below the BPM slider"
echo "3. Click the button to toggle autodetect ON/OFF"
echo "4. When ON:"
echo "   - Button appears bright green"
echo "   - BPM slider shows detected tempo but is grayed out (read-only)"
echo "   - BPM display shows musical note (🎵) indicator"
echo "5. When OFF:"
echo "   - Button appears gray"
echo "   - BPM slider is active for manual control"
echo "   - BPM display shows numeric value only"
echo ""
echo "TECHNICAL DETAILS:"
echo "• Uses RhythmDetector's analyzeTempo method for tempo detection"
echo "• Valid BPM range: 30-260 BPM (same as manual slider)"
echo "• Tempo updates occur when detected change > 0.5 BPM (reduces jitter)"
echo "• Filter frequencies automatically scale to detected tempo"
echo "• Manual BPM changes ignored when autodetect is enabled"
echo ""
echo "TESTING TIPS:"
echo "• Play rhythmic audio with clear beat structure"
echo "• Try different tempos to see automatic detection"
echo "• Toggle between manual and auto modes during playback"
echo "• Check that filter frequencies adapt to tempo changes"
echo ""

# Start the application
echo "Starting NeuronSeqSampler..."
echo "Press 'M' to show/hide the connection matrix if needed"
echo ""

./NeuronSeqSampler