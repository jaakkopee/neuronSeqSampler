#!/bin/bash

echo "🎛️ MATRIX VISIBILITY TOGGLE TEST"
echo "==============================="
echo ""
echo "🔍 Testing M Key Functionality:"
echo "  • Launch NeuronSeqSampler in testing mode"
echo "  • Connection matrix should be visible by default"
echo "  • Press 'M' key to hide the matrix"
echo "  • Press 'M' key again to show the matrix"
echo "  • Look for console messages confirming toggle actions"
echo ""
echo "✅ Expected Console Output:"
echo "  '🎛️ Rhythm Interpreter connection matrix hidden (press M to toggle)'"
echo "  '🎛️ Rhythm Interpreter connection matrix shown (press M to toggle)'"
echo ""

echo "🚀 Launching test (will auto-exit after 8 seconds)..."
echo "   Press 'M' key multiple times to test the toggle!"
echo ""

timeout 8 ./NeuronSeqSampler --testing