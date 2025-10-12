# NeuronSeqSampler - Recent Updates & Features

## Latest Release - October 2025

### 🎵 Dynamic Frequency Labels
**Revolutionary tempo-aware interface enhancement**

- **Dynamic Label Updates**: Connection matrix frequency labels now update in real-time to show actual scaled frequencies
- **Tempo Scaling**: All Todd (1994) frequencies scale proportionally with BPM (scaling factor = currentBPM/120)
- **Automatic Updates**: Labels refresh when:
  - Manual BPM slider changes
  - Autodetect tempo changes the BPM (>0.1 BPM threshold)
  - Connection matrix is initially created
- **Accurate Information**: No more confusion between static "Quarter (1Hz)" labels and actual tempo-scaled frequencies
- **Musical Examples**: 
  - At 60 BPM: Quarter shows "0.5Hz", Half shows "0.25Hz"
  - At 240 BPM: Quarter shows "2Hz", Half shows "1Hz"
  - At 120 BPM: Default Todd frequencies (Quarter "1Hz", etc.)

### 🤖 Autodetect Tempo Integration
**Hands-free tempo tracking with RhythmDetector**

- **Toggle Button**: "Autodetect Tempo" button below BPM controls
- **RhythmDetector Integration**: Uses `analyzeTempo()` method for intelligent BPM detection
- **Visual Feedback**: BPM display shows musical note (🎵) when autodetect is active
- **Automatic Scaling**: Detected tempo automatically scales all Todd frequencies
- **Manual Override Prevention**: BPM slider disabled during autodetect mode
- **Real-time Updates**: Frequency labels update automatically as tempo is detected

### 📊 Enhanced Value Displays
**Large, readable parameter displays for improved usability**

- **Enlarged Displays**: All slider value displays increased to 60×30 pixels
- **No Scrolling**: Proper sizing prevents unwanted scroll handles even with 3-digit BPM values
- **13pt Text**: Optimized text size for clear readability without overflow
- **Consistent Design**: Applied to BPM slider, Rhythmogram scale slider, and all connection matrix controls
- **Better UX**: No more squinting at tiny parameter values during performance

### 🎛️ Connection Matrix Improvements
**Enhanced interface for Todd frequency routing**

- **Improved Tooltips**: More detailed frequency band descriptions
- **Visual Consistency**: Better button spacing and alignment
- **State Persistence**: Connection states properly maintained across interface operations
- **Performance Optimization**: Reduced update overhead with change detection

---

## Technical Implementation Details

### Dynamic Frequency Labels System
```cpp
// Real-time frequency calculation
float tempoScale = currentBPM / 120.0f;
float scaledFrequency = baseFrequencies[i] * tempoScale;

// Smart formatting for display
if (scaledFrequency < 1.0f) {
    // Show decimals for sub-1Hz frequencies
    freqText = removeTrailingZeros(std::to_string(scaledFrequency));
} else {
    // Show integers for ≥1Hz frequencies  
    freqText = std::to_string(static_cast<int>(scaledFrequency + 0.5f));
}

// Update label: "Quarter (0.63Hz)" at 75.6 BPM
labelText = filterNames[i] + " (" + freqText + "Hz)";
```

### Autodetect Tempo Architecture
```cpp
// In RhythmInterpreter::processAudioFrame()
if (autodetectTempo && rhythmDetector) {
    float detectedBPM = rhythmDetector->analyzeTempo();
    if (detectedBPM > 0) {
        setBPM(detectedBPM);  // Triggers frequency scaling
        updateFilterBankForBPM();  // Updates all Todd filters
        // GUI automatically updates frequency labels via change detection
    }
}
```

### Enhanced Display Sizing
```cpp
// Optimized for 3-digit BPM values without scrolling
bpmLabel->setSize(60, 30);  // Was 40×20, caused scrolling at >99 BPM
bpmLabel->setTextSize(13);  // Was 15, caused overflow
rhythmogramScaleLabel->setSize(60, 30);  // Consistent sizing across all displays
```

---

## Migration Guide

### For Existing Users
- **No Breaking Changes**: All existing functionality preserved
- **Automatic Updates**: Frequency labels will show scaled frequencies immediately
- **Enhanced Experience**: Larger displays improve usability without changing workflow
- **New Features**: Autodetect tempo is optional - manual BPM control still available

### For Developers
- **New Methods**: 
  - `GUI::updateFrequencyLabels()` - Call when BPM changes
  - `RhythmInterpreter::setAutodetectTempo(bool)` - Enable/disable autodetect
  - `RhythmInterpreter::getAutodetectTempo()` - Check autodetect state
- **Dependencies**: RhythmDetector integration for autodetect functionality
- **GUI Updates**: Enhanced TGUI widget sizing for better display rendering

---

## User Benefits

### 🎵 **Musicians & Performers**
- **Accurate Tempo Information**: See exactly what frequencies are being analyzed
- **Hands-free Tempo**: Autodetect mode for live performance without manual BPM adjustment
- **Better Readability**: Large displays prevent parameter reading errors during performance
- **Musical Intelligence**: Interface reflects actual musical relationships, not just defaults

### 🔬 **Researchers & Developers** 
- **Scientific Accuracy**: Interface matches Todd (1994) theoretical framework implementation
- **Real-time Feedback**: Immediate visual confirmation of tempo scaling effects
- **Enhanced Debugging**: Clear parameter displays aid in system analysis
- **Educational Value**: Students can see how tempo affects rhythmic hierarchy

### 🎛️ **Sound Designers**
- **Precise Control**: Know exactly what frequencies are driving which neural pathways
- **Creative Flexibility**: Autodetect for responsive patches, manual for precise control
- **Visual Clarity**: Enhanced displays reduce setup time and increase creative flow
- **Professional Workflow**: Interface scales from experimental to performance-ready

---

## Demo Scripts

Try the new features with guided demonstrations:

```bash
# Test dynamic frequency labels
./demo_frequency_labels.sh

# Experience autodetect tempo
./demo_autodetect_tempo.sh  

# See enhanced value displays
./demo_large_sliders.sh
```

Each demo provides specific instructions and expected behaviors for the new features.

---

*These enhancements maintain NeuronSeqSampler's scientific foundation while significantly improving user experience and musical intelligence.*