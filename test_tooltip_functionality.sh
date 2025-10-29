#!/bin/bash
echo "🎵 Testing Neuron Sample Tooltips"
echo "================================="
echo ""
echo "Instructions:"
echo "1. Application will start"
echo "2. Press 'L' to load the factory drum pattern preset"
echo "3. Hover your mouse over the neurons to see sample file tooltips"
echo "4. Close the application when done"
echo ""
echo "Starting application..."
echo ""

# Run the application for a limited time
timeout 30s ./test_build || echo "Application closed or timeout reached"

echo ""
echo "Test completed! Did you see the sample file tooltips when hovering?"