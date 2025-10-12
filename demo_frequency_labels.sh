#!/bin/bash
# Demo script to test dynamic frequency labels when BPM changes

echo "🎵 Testing Dynamic Frequency Labels in Connection Matrix"
echo "This demo will show how frequency labels update when BPM changes"
echo ""

# Start the application in background
echo "▶️  Starting NeuronSeqSampler..."
cd /home/jaakko/Asiakirjat/Koodii/neuronSeqSampler
./NeuronSeqSampler &
APP_PID=$!

echo "   Application started (PID: $APP_PID)"
echo ""

echo "📊 Instructions for testing:"
echo "   1. Open the Connection Matrix (press 'M' key)"
echo "   2. Look at the frequency labels on the left side"
echo "   3. Change the BPM slider and observe the labels update"
echo "   4. Try the Autodetect Tempo toggle and play samples"
echo ""

echo "🎛️  Expected behavior:"
echo "   • At 120 BPM (default): Labels show Quarter (1Hz), Half (0.5Hz), etc."  
echo "   • At 60 BPM: Labels show Quarter (0.5Hz), Half (0.25Hz), etc."
echo "   • At 240 BPM: Labels show Quarter (2Hz), Half (1Hz), etc."
echo "   • With autodetect on: Labels update automatically as tempo is detected"
echo ""

echo "⏱️  Testing for 30 seconds, then will terminate..."
sleep 30

# Clean up
echo ""
echo "🔄 Terminating application..."
kill $APP_PID 2>/dev/null
wait $APP_PID 2>/dev/null

echo "✅ Demo completed!"
echo ""
echo "💡 The frequency labels should now dynamically update to show the actual"
echo "   scaled frequencies based on the current BPM setting, making the"
echo "   connection matrix more informative and tempo-aware."