#!/bin/bash

# Test script to check high-frequency filter responsiveness
echo "🔍 Testing High-Frequency Filter Responsiveness"

echo "Starting NeuronSeqSampler..."
cd /home/jaakko/Asiakirjat/Koodii/neuronSeqSampler

# Start application in background and capture output
timeout 30s ./NeuronSeqSampler 2>&1 | tee /tmp/neuronseq_test.log &
APP_PID=$!

sleep 5

echo "Application started (PID: $APP_PID)"
echo "Instructions for manual testing:"
echo "1. Click on the neuron to trigger samples"
echo "2. Press '1' key to play samples"
echo "3. Press 'M' to toggle rhythmogram matrix visibility"
echo "4. Check if high-frequency filters (bands 5-7) show activity"
echo "5. Look for any 'HF Filter' debug messages in the output"
echo ""
echo "Expected behavior:"
echo "- High-frequency filters should show dynamic values, not constants"
echo "- Filter outputs should respond to audio events"
echo "- Debug messages should show filter activity when samples play"

# Wait for the application to finish or timeout
wait $APP_PID

echo ""
echo "Test completed. Checking logs for high-frequency filter activity..."
if grep -q "HF Filter" /tmp/neuronseq_test.log; then
    echo "✅ High-frequency filter debug messages found:"
    grep "HF Filter" /tmp/neuronseq_test.log | tail -10
else
    echo "❌ No high-frequency filter debug messages found"
fi

echo ""
echo "Full log saved to: /tmp/neuronseq_test.log"