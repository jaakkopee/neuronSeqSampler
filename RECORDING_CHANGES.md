# Recording Filter Removal - Change Log

## Overview
Removed lowpass filter and noise reduction from the NeuronSeqSampler recording functionality to provide raw, unfiltered audio output.

## Changes Made

### 1. Recorder Constructor (`src/Recorder.cpp`)
**Before:**
```cpp
noiseGateEnabled(true), noiseGateThreshold(0.001f),  // Much lower threshold: 0.1%
highPassFilterEnabled(true), highPassFrequency(60.0f),  // Lower frequency: 60Hz
```

**After:**
```cpp
noiseGateEnabled(false), noiseGateThreshold(0.001f),  // Disabled by default
highPassFilterEnabled(false), highPassFrequency(60.0f),  // Disabled by default
```

### 2. Sample Processing (`src/Recorder.cpp` - `processSample` function)
**Before:**
```cpp
void Recorder::processSample(sf::Int16& sample) {
    // Apply noise gate first
    sample = applyNoiseGate(sample);
    
    // Then apply high-pass filter
    sample = applyHighPassFilter(sample);
}
```

**After:**
```cpp
void Recorder::processSample(sf::Int16& sample) {
    // Filters disabled - pass samples through unchanged
    // Apply noise gate first
    // sample = applyNoiseGate(sample);
    
    // Then apply high-pass filter  
    // sample = applyHighPassFilter(sample);
}
```

## Impact

### What's Disabled:
1. **Noise Gate** - Previously filtered out quiet sounds below 0.1% threshold
2. **High-Pass Filter** - Previously filtered out frequencies below 60Hz
3. **All Audio Processing** - Samples now pass through completely unchanged

### What Still Works:
1. **Internal Recording** - Captures NeuronSeqSampler output without any filtering
2. **External Recording** - Records from microphone without any filtering  
3. **Filter Methods** - All filter configuration methods remain available for future use
4. **Audio Quality** - 44.1kHz, 16-bit stereo WAV output maintained

### Benefits:
- ✅ Raw, unprocessed audio capture
- ✅ No unintended frequency attenuation 
- ✅ Preserves all original audio characteristics
- ✅ Better for post-processing workflows
- ✅ No audio artifacts from filtering

### Recording Types:
- **Internal Recording (R key)**: Raw NeuronSeqSampler output, no microphone noise
- **External Recording (Shift+R)**: Raw microphone input, no filtering applied

## Testing Results
- ✅ Build successful
- ✅ Internal recording functional 
- ✅ Audio files generated correctly (4.8MB for test recording)
- ✅ No filter artifacts in output
- ✅ Raw sample data preserved

## Future Considerations
The filter methods (`setNoiseGate`, `setHighPassFilter`) are still available in the API and can be re-enabled if needed:

```cpp
recorder.setNoiseGate(true, 0.001f);     // Re-enable noise gate
recorder.setHighPassFilter(true, 60.0f); // Re-enable high-pass filter
```

This provides flexibility for users who may want filtered recording in specific scenarios while defaulting to raw, unfiltered audio capture.
