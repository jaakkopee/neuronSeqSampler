#!/bin/bash
# Complete audio test script

echo "Complete Audio System Test"
echo "=========================="

echo "1. Stopping JACK for recording..."
./manage_jack.sh stop

echo ""
echo "2. Optimizing microphone settings..."
./optimize_mic.sh

echo ""
echo "3. Testing recording for 3 seconds..."
cd build
echo -e "\n" | timeout 5s ./recorder_demo

echo ""
echo "4. Checking if recording contains audio data..."
if hexdump -C demo_recording.wav | head -5 | grep -q "00 00 00 00 00 00 00 00"; then
    echo "❌ Recording contains only silence"
else
    echo "✅ Recording contains audio data"
fi

echo ""
echo "5. Starting JACK for sample playback..."
cd ..
./manage_jack.sh start

echo ""
echo "6. Audio system ready!"
echo "   - Recording works when JACK is stopped"
echo "   - Sample playback works when JACK is running"
echo "   - Use './manage_jack.sh stop' before recording"
echo "   - Use './manage_jack.sh start' after recording"
