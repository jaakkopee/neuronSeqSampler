#!/bin/bash

echo "Testing multiple connection matrix toggle functionality..."
echo "Instructions:"
echo "1. The application will start with a test network"
echo "2. Try activating MULTIPLE toggles in the connection matrix"
echo "3. Verify that multiple toggles can be active simultaneously"
echo "4. Test filter gain sliders to ensure they don't reset toggles"
echo "5. Press Ctrl+C when finished testing"
echo ""
echo "Starting application..."
sleep 2

cd /home/jaakko/Asiakirjat/Koodii/neuronSeqSampler
./NeuronSeqSampler --testing