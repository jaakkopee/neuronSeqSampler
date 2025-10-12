#!/bin/bash

# Demo script for testing the new rhythmogram scale slider feature

echo "🎛️ Rhythmogram Scale Slider Demo"
echo "================================="
echo ""
echo "New features added:"
echo "• Vertical rhythmogram scale slider (0.0-20.0, default 5.0)"
echo "• Real-time scale value display"
echo "• Positioned at the right end of the connection matrix"
echo "• 0.1 step precision for fine control"
echo ""
echo "How to test:"
echo "1. Launch the application with: ./NeuronSeqSampler --testing"
echo "2. Press 'M' to open the rhythmogram connection matrix"
echo "3. Press 'R' to enable rhythmogram analysis"
echo "4. Press spacebar to generate audio for analysis"
echo "5. Look for the vertical 'Scale' slider at the right edge"
echo "6. Adjust the slider to see values from 0.0 to 20.0"
echo "7. Watch the numeric display update in real-time"
echo ""
echo "Technical details:"
echo "• Default scale: 5.0 (down from previous 500.0 hardcoded)"  
echo "• Range: 0.0 - 20.0 with 0.1 step increments"
echo "• Controls rhythmogram-to-neural activation scaling"
echo "• Higher values = stronger neural response to rhythmic patterns"
echo "• Lower values = more subtle rhythmic influence"
echo ""

# Launch the application for testing
echo "Launching NeuronSeqSampler in testing mode..."
echo "Look for the new Scale slider at the right end of the matrix!"
echo ""
./NeuronSeqSampler --testing