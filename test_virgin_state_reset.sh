#!/bin/bash

echo "Testing Virgin State Reset for Matrix Toggles"
echo "============================================="
echo ""
echo "Instructions for testing:"
echo "1. The application will start with a test network (3 neurons)"
echo "2. Try the following sequence:"
echo "   a) Click a matrix toggle to activate it (should show ● and gain slider)"
echo "   b) Adjust the gain slider to a different value" 
echo "   c) Click the same toggle again to deactivate it"
echo "   d) Click it once more to reactivate it"
echo "   e) Verify the gain slider returns to default 30% value"
echo ""
echo "Expected behavior:"
echo "- Deactivation should reset the gain to default (30%)"
echo "- Reactivation should show default gain, not the previous value"
echo "- This ensures a true 'virgin state' reset"
echo ""
echo "Press Ctrl+C to exit when finished testing..."
echo ""
sleep 3

cd /home/jaakko/Asiakirjat/Koodii/neuronSeqSampler
./NeuronSeqSampler --testing