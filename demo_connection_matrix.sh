#!/bin/bash

# 🎛️ Connection Matrix Demo Script
# Comprehensive demonstration of the rhythm interpreter connection matrix GUI

echo "🎛️ NeuronSeqSampler - Connection Matrix Demo"
echo "============================================"
echo ""

echo "📊 Connection Matrix Features:"
echo "  ✅ 8×N visual grid interface (frequency bands × neurons)"
echo "  ✅ Toggle buttons for connection enable/disable"  
echo "  ✅ Gain sliders for precise signal control"
echo "  ✅ Real-time audio analysis and feedback"
echo "  ✅ Frequency-specific tooltips and descriptions"
echo "  ✅ Quick action buttons (Clear All / Random)"
echo ""

echo "🎵 Frequency Band Coverage:"
echo "  🔹 Sub Bass (60Hz)     - Kick drum fundamentals"
echo "  🔹 Bass (120Hz)        - Kick harmonics & basslines"  
echo "  🔹 Low Mids (250Hz)    - Snare body & tom warmth"
echo "  🔹 Mids (500Hz)        - Snare attack & vocal clarity"
echo "  🔹 Upper Mids (1kHz)   - Hi-hat body & presence"
echo "  🔹 Presence (2kHz)     - Hi-hat attack & definition"
echo "  🔹 Brilliance (4kHz)   - Cymbal shimmer & sparkle"
echo "  🔹 Air (8kHz)          - High-frequency ambience"
echo ""

echo "🎚️ How to Use the Connection Matrix:"
echo "  1. Launch application in testing mode (automatic 3-neuron setup)"
echo "  2. Look for 'Connection Matrix' panel on the right side"
echo "  3. Press 'R' to start recording (enables rhythm analysis)"
echo "  4. Press 'Spacebar' to generate audio for the rhythm interpreter"
echo "  5. Click toggle buttons (○/●) to connect frequency bands to neurons"
echo "  6. Adjust gain sliders for connected routes (0-100%)"
echo "  7. Use 'Random' button to generate interesting connection patterns"
echo "  8. Use 'Clear All' to reset all connections"
echo ""

echo "🎯 Recommended Experiments:"
echo "  • Connect Sub/Bass bands to Neuron 1 (kick-focused routing)"
echo "  • Connect Mid bands to Neuron 2 (snare-focused routing)"
echo "  • Connect Presence/Brilliance to Neuron 3 (hi-hat routing)"
echo "  • Try 'Random' connections for unexpected musical interactions"
echo "  • Adjust gains from 10% (subtle) to 100% (dominant influence)"
echo ""

echo "⚡ Performance Tips:"
echo "  • Recording must be active for rhythm analysis to work"
echo "  • Visual feedback shows connection strength through button brightness"
echo "  • Hover over toggle buttons for detailed frequency descriptions"  
echo "  • Changes take effect immediately during audio processing"
echo ""

echo "🚀 Launching NeuronSeqSampler with Connection Matrix..."
echo "   Look for the '🎛️ Connection Matrix (8×3)' panel on the right!"
echo ""

# Launch with testing network for immediate experimentation
./NeuronSeqSampler --testing