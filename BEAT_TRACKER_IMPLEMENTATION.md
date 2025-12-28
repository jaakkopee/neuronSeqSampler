# Beat Phase Recognition System - Implementation Summary

## Overview
Implemented a complete beat phase recognition system using cross-correlation analysis to detect downbeat phase and modulate learning at musically important moments.

## Architecture

### Core Algorithm (BeatTracker class)
- **Cross-correlation**: Compares network firing patterns with audio input onset patterns
- **Tempo Detection**: Searches 40-200 BPM range to find best periodic alignment
- **Phase Tracking**: Maintains current phase (0.0-1.0, where 0.0 = downbeat)
- **Learning Modulation**: Gaussian-envelope phase-based gain multiplier

### Key Components

#### src/BeatTracker.h/cpp
- **History Buffers**: 8 seconds of network activity and input onsets
- **Analysis Frequency**: Every 50 frames (~0.5-1 sec)
- **Phase Correction**: Gentle alignment (10% per update) to strongest onset
- **Gain Formula**: `phaseGain = 1 + beatBoost × exp(-(distFromDownbeat/phaseWindow)²)`

#### Integration Points
1. **NeuronNetwork::processAudioForRhythm()**
   - Collects network firing activity: `neuron->getHasFired()` → 1.0 or 0.0
   - Gets input onset activity: `rhythmInterpreter->getFilterOutputs()`
   - Updates beat tracker: `beatTracker->update(networkFirings, inputOnsets)`

2. **NeuronNetwork::learnFromRhythm()**
   - Gets phase gain: `beatTracker->getPhaseBasedLearningGain()`
   - Multiplies with onset boost: `effectiveOnsetBoost *= phaseGain`
   - Provides learning "jolts" at downbeats

#### GUI Controls (src/GUI.h/cpp)
Located at Y=720-890 in connection matrix panel:
- **Toggle Button**: TRACK OFF/ON (gray ↔ green)
- **Status Label**: Shows "Phase: X.XX | Tempo: XXX.X | Conf: X.XX"
- **Beat Boost Slider**: Range 1.0-20.0, default 5.0 (controls learning multiplier strength)
- **Phase Window Slider**: Range 0.01-0.5, default 0.15 (controls temporal tightness)

## Parameters

### Adjustable (via GUI)
- **beatBoost**: Learning strength multiplier (1.0-20.0)
- **phaseWindow**: Temporal window around downbeat (0.01-0.5)
- **enabled**: Toggle beat tracking on/off

### Fixed (compile-time)
- **sampleRate**: 44100 Hz
- **frameSize**: 512 samples
- **historyLength**: 8 seconds
- **analysisInterval**: 50 frames
- **minBPM**: 40
- **maxBPM**: 200
- **phaseCorrection**: 0.1 (10% per update)
- **tempoSmoothing**: 0.95

## Signal Processing Details

### Network Activity Combination
```cpp
float combineNetworkActivity(networkFirings):
    return average of all neuron firing states (1.0 or 0.0)
```

### Input Activity Combination
```cpp
float combineInputActivity(inputOnsets):
    weighted sum favoring low-frequency bands
    weight[i] = exp(-0.3 × i)
    return normalized weighted sum
```

### Cross-Correlation
```cpp
float calculateCorrelation(lag):
    normalized Pearson correlation coefficient
    between networkHistory and time-shifted inputHistory
    returns value in [-1, 1]
```

### Phase Gain Calculation
```cpp
float calculatePhaseGain():
    dist = min(phase, 1.0 - phase)  // wraparound distance
    gaussian = exp(-(dist / phaseWindow)²)
    return 1.0 + beatBoost × gaussian
```

## Files Modified/Created

### New Files
- `src/BeatTracker.h` - Class declaration
- `src/BeatTracker.cpp` - Implementation
- `test_beat_tracker.sh` - Test script

### Modified Files
- `src/NeuronNetwork.h` - Added beatTracker member and accessor
- `src/NeuronNetwork.cpp` - Integration in processAudioForRhythm() and learnFromRhythm()
- `src/GUI.h` - Added beat tracker widget pointers
- `src/GUI.cpp` - Added controls, status updates, cleanup
- `CMakeLists.txt` - Added BeatTracker.cpp to build

## Testing

Run the test script:
```bash
./test_beat_tracker.sh
```

### Manual Test Steps
1. Launch application
2. Open rhythmogram panel
3. Enable beat tracker (TRACK ON button should turn green)
4. Load audio file with clear beat
5. Observe status display updating with phase/tempo/confidence
6. Adjust beat boost slider to control learning strength
7. Adjust phase window to control temporal precision
8. Verify learning modulation by observing connection weight changes

## Expected Behavior

### Phase Tracking
- Phase should advance from 0.0 to 1.0 cyclically
- Phase 0.0 should align with strongest onsets (downbeats)
- Confidence should increase as pattern becomes clearer

### Tempo Detection
- Should stabilize on the dominant periodic pattern
- Range: 40-200 BPM
- Smoothed with factor 0.95 to avoid jitter

### Learning Modulation
- Maximum gain at phase 0.0 (downbeat)
- Gain falls off according to Gaussian envelope
- Width controlled by phaseWindow parameter
- Strength controlled by beatBoost parameter

## Future Enhancements

1. **Parameter Tuning**: Optimize correlation thresholds and phase correction speed
2. **Preset Integration**: Save/load beat tracker settings with presets
3. **Visualization**: Graphical display of phase position
4. **Multi-scale Tracking**: Hierarchical beat/measure/phrase detection
5. **Adaptive Windows**: Automatically adjust phaseWindow based on tempo stability

## Build Status
✅ Successfully compiles with no errors
✅ All dependencies linked correctly
✅ Ready for testing and parameter tuning
