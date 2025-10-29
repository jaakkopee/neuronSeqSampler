#!/bin/bash

echo "🔍 Testing Spectral View Restoration After Reset"
echo "==============================================="

echo "📋 This test verifies that the spectral view resumes working after network reset"
echo ""

echo "🏗️  Building project..."
make > /dev/null 2>&1

if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo "✅ Build successful"
echo ""

echo "🧪 Testing scenario:"
echo "1. App startup with rhythm interpreter"
echo "2. Spectral display should show activity (if audio is playing)"
echo "3. Network reset should clear spectral display"
echo "4. Spectral display should resume working after reset"
echo ""

echo "🔧 Key fixes applied:"
echo "✅ AudioManager rhythm interpreter reference updated on reset"
echo "✅ AudioManager rhythm interpreter reference updated on preset load"
echo "✅ Spectral display properly reinitializes with new rhythm interpreter"
echo "✅ Enhanced logging for debugging rhythm interpreter connections"
echo ""

echo "📝 Manual test steps:"
echo ""
echo "1. 🚀 Run: ./test_build"
echo ""
echo "2. 🎵 Play some audio samples (press number keys 1-9)"
echo "   → Spectral view should show frequency activity"
echo ""
echo "3. 🔄 Reset network via menu: Network → Reset Network"
echo "   → Look for these console messages:"
echo "     '🔄 Spectral display cleared'"
echo "     '🔄 AudioManager reconnected to new rhythm interpreter'"
echo "     '🔄 Spectral display reconnected to new rhythm interpreter'"
echo ""
echo "4. 🎵 Play audio samples again (number keys 1-9)"
echo "   → Spectral view should resume showing activity"
echo ""
echo "5. 📂 Test preset loading: Press 'L' or use Presets menu"
echo "   → Look for: '🔄 AudioManager updated after preset load'"
echo "   → Spectral view should continue working"
echo ""
echo "✅ Expected Results:"
echo "   - No crashes during reset operations"
echo "   - Spectral view clears properly during reset"
echo "   - Spectral view resumes after reset/preset load"
echo "   - Audio processing feeds rhythm interpreter correctly"
echo ""

echo "🔧 Debug information to watch for:"
echo "   - '🎵 Minimal RhythmInterpreter initialized'"
echo "   - '🔄 SimpleSpectralDisplay: New rhythm interpreter connected'"
echo "   - AudioManager and SpectralDisplay reconnection messages"
echo ""

echo "❌ If spectral view still doesn't work, check:"
echo "   - Is audio actually being captured/processed?"
echo "   - Are samples playing (generating audio for analysis)?"
echo "   - Check console for any rhythm interpreter errors"