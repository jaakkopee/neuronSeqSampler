# Synchronized Tempo-Based Frequency Scaling Implementation

## Overview
Implemented synchronized frequency label updates across both UI locations when the BPM slider changes, ensuring that filter frequencies scale proportionally relative to the 120 BPM baseline.

## Implementation Details

### 1. GUI Connection Matrix Labels (Right Side)
**File:** `src/GUI.cpp`
**Method:** `updateFrequencyLabels()`

**Changes made:**
- Modified to read BPM from `bpmSlider->getValue()` instead of hardcoded 120.0f
- Calculates `tempoScale = currentBPM / 120.0f`
- Updates RhythmInterpreter filter frequencies via `setBandFrequency(i, scaledFrequency)`
- Added call to `spectralDisplay->setManualBPM(currentBPM)` for synchronization

### 2. Spectral Display Labels (Left Side) 
**Files:** `src/SimpleSpectralDisplay.h`, `src/SimpleSpectralDisplay.cpp`

**Changes made:**
- Added `setManualBPM(float bmp)` method to public interface
- Implemented `setManualBPM()` to update `lastKnownBPM` and trigger `updateFrequencyBands()`
- Modified `updateFrequencyBands()` to use stored `lastKnownBPM` instead of hardcoded 120.0f
- Removed BPM change detection since it's now externally driven

### 3. Synchronization Logic
**Location:** `GUI::updateFrequencyLabels()`

The BPM slider callback triggers `updateFrequencyLabels()` which now:
1. Gets current BPM from slider
2. Updates connection matrix filter labels 
3. Updates RhythmInterpreter filter frequencies
4. Calls `spectralDisplay->setManualBPM()` to update spectral display labels
5. Both label sets update simultaneously with identical scaling

## Frequency Scaling Formula
```cpp
float tempoScale = currentBPM / 120.0f;
float scaledFrequency = baseFrequency * tempoScale;
```

## Base Frequencies (Todd 1994 Rhythmogram)
```cpp
const std::vector<float> baseFrequencies = {
    0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f  // Hz at 120 BPM
};
```

## Filter Names
```cpp
const std::vector<std::string> filterNames = {
    "Phrase", "Whole", "Half", "Quarter", "Eighth", "16th", "32nd", "Onset"
};
```

## Expected Behavior Examples

| BPM Setting | Tempo Scale | Quarter Note Freq | Result |
|-------------|-------------|-------------------|---------|
| 60 BPM      | 0.5x        | 0.5 Hz           | Half frequency |
| 120 BPM     | 1.0x        | 1.0 Hz           | Base frequency |
| 180 BPM     | 1.5x        | 1.5 Hz           | 1.5x frequency |
| 240 BPM     | 2.0x        | 2.0 Hz           | Double frequency |

## Testing
1. Launch application: `./test_build`
2. Observe default frequencies at 120 BPM in both locations
3. Move BPM slider and verify both label sets update simultaneously
4. Check scaling accuracy at various BPM values (60, 90, 180, 240)
5. Verify RhythmInterpreter filter frequencies also update via `setBandFrequency()`

## Integration Points
- **Auto-tempo system**: Continues to work independently via RhythmInterpreter 
- **Manual BPM control**: Now affects both visual labels and actual filter processing
- **Frequency display consistency**: Both UI locations show identical scaled frequencies
- **Real-time updates**: Changes are immediate when BPM slider moves

This implementation completes the comprehensive tempo-following system with unified frequency scaling across all UI elements and actual audio processing.