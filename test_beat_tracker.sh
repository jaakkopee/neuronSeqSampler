#!/bin/bash

echo "=== Beat Tracker Test ==="
echo ""
echo "This test verifies the beat phase recognition system:"
echo "1. Enable beat tracker toggle button"
echo "2. Load an audio file with a clear beat"
echo "3. Observe phase, tempo, and confidence display"
echo "4. Adjust beat boost slider (1-20) to control learning strength"
echo "5. Adjust phase window slider (0.01-0.5) to control temporal tightness"
echo "6. Verify learning modulation at downbeats"
echo ""
echo "Starting test_build..."
./test_build
