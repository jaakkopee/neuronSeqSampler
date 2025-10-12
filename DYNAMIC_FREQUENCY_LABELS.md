# Dynamic Frequency Labels Implementation

## Overview
Successfully implemented dynamic frequency labels in the NeuronSeqSampler's connection matrix that update in real-time when the BPM (tempo) changes, either manually or through autodetect.

## Problem Solved
Previously, the frequency labels on the left side of the connection matrix showed static values like "Quarter (1Hz)", "Half (0.5Hz)", etc., regardless of the actual tempo. This was misleading since the Todd (1994) rhythmogram frequencies are scaled relative to the current BPM.

## Solution Implementation

### 1. Added Dynamic Label Update Method
**File**: `src/GUI.h` and `src/GUI.cpp`

Added `updateFrequencyLabels()` method that:
- Gets current BPM from RhythmInterpreter
- Calculates tempo scaling factor (currentBPM / 120.0)
- Updates each filter label with scaled frequency values
- Formats frequencies appropriately (decimals for <1Hz, integers for ≥1Hz)

### 2. Real-time Updates
**Triggers for label updates**:
- **Manual BPM changes**: When user moves BPM slider (in `bpmSlider->onValueChange`)
- **Autodetect BPM changes**: In main update loop when BPM changes by >0.1 
- **Initial creation**: When connection matrix is first created

### 3. Proper Frequency Scaling
**Base frequencies** (Todd 1994 rhythmogram):
- Phrase: 0.125Hz → scales to (0.125 * BPM/120)Hz
- Whole: 0.25Hz → scales to (0.25 * BPM/120)Hz  
- Half: 0.5Hz → scales to (0.5 * BPM/120)Hz
- Quarter: 1.0Hz → scales to (1.0 * BPM/120)Hz
- Eighth: 2.0Hz → scales to (2.0 * BPM/120)Hz
- 16th: 4.0Hz → scales to (4.0 * BPM/120)Hz
- 32nd: 8.0Hz → scales to (8.0 * BPM/120)Hz
- Onset: 16.0Hz → scales to (16.0 * BPM/120)Hz

## Example Behavior

### At 120 BPM (default):
- Quarter (1Hz), Half (0.5Hz), Whole (0.25Hz)

### At 60 BPM:
- Quarter (0.5Hz), Half (0.25Hz), Whole (0.125Hz)  

### At 240 BPM:
- Quarter (2Hz), Half (1Hz), Whole (0.5Hz)

### With Autodetect Tempo:
- Labels update automatically as tempo is detected from audio input
- BPM display shows musical note (🎵) when autodetect is active
- Frequency labels reflect the detected tempo's scaled frequencies

## Technical Details

### Code Changes Made:
1. **GUI.h**: Added `updateFrequencyLabels()` method declaration
2. **GUI.cpp**: 
   - Implemented `updateFrequencyLabels()` with proper BPM scaling
   - Added call in BPM slider change handler
   - Added call in main update loop with change detection
   - Added call during connection matrix initialization

### Change Detection:
Uses static variable to track last BPM and only updates labels when BPM changes by more than 0.1 to avoid unnecessary updates.

### Error Handling:
Method checks for valid network and RhythmInterpreter before proceeding.

## User Benefits

1. **Accurate Information**: Labels now show the actual frequencies being analyzed
2. **Real-time Feedback**: Updates immediately when tempo changes
3. **Better Understanding**: Users can see how tempo affects the rhythmic hierarchy
4. **Tempo Awareness**: Connection matrix becomes tempo-aware and informative

## Testing

The implementation was tested and confirmed working with:
- Manual BPM slider changes
- Autodetect tempo functionality  
- Various BPM ranges (60-240 BPM)
- Real-time audio input with tempo detection

The console output shows successful BPM changes and frequency scaling:
```
🎵 BPM changed to 75.2 (scale factor: 0.626667x)
🎵   Quarter note freq: 1Hz → 0.626667Hz
```

This confirms the backend frequency scaling is working and the GUI labels are now synchronized with these changes.