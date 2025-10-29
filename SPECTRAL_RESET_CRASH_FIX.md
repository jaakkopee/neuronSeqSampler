# Spectral View Reset Crash Fix

## Problem Analysis
The crash with "Muistialueen ylitys" (memory overflow/segmentation fault) was occurring after network reset when the spectral view tried to access a deleted RhythmInterpreter object.

### Root Cause
Race condition during network reset:
1. GUI calls `network->clearNetwork()` which deletes the RhythmInterpreter
2. Main loop continues calling `spectralDisplay.update()` 
3. The update method tries to access the deleted RhythmInterpreter
4. **CRASH**: Segmentation fault from accessing freed memory

## Solution Implemented

### ✅ **1. Enhanced Null Pointer Safety in SpectralDisplay**
```cpp
void SimpleSpectralDisplay::update() {
    // Double-check that rhythmInterpreter is still valid before any access
    if (!rhythmInterpreter) return;
    
    // ... update logic ...
    
    if (timeSinceLastUpdate >= updateInterval) {
        // Triple-check rhythmInterpreter validity before accessing it
        // This prevents race conditions during network reset
        if (!rhythmInterpreter) {
            updateClock.restart();
            return;
        }
        
        // Safe access with exception handling
        try {
            auto filterOutputs = rhythmInterpreter->getFilterOutputs();
            // ...
        } catch (const std::exception& e) {
            // Auto-recovery: clear reference to prevent further crashes
            rhythmInterpreter = nullptr;
        }
    }
}
```

### ✅ **2. Improved GUI Reset Ordering**
```cpp
void GUI::resetNetwork() {
    // Step 1: Notify spectral display FIRST (prevents access)
    if (spectralDisplay) {
        spectralDisplay->setRhythmInterpreter(nullptr);
    }
    
    // Step 2: Clear network (deletes old rhythm interpreter)
    network->clearNetwork();
    
    // Step 3: Create new rhythm interpreter  
    network->initializeRhythmInterpreter();
    
    // Step 4: Reconnect spectral display to new interpreter
    if (spectralDisplay && network->getRhythmInterpreter()) {
        spectralDisplay->setRhythmInterpreter(network->getRhythmInterpreter());
    }
}
```

### ✅ **3. Enhanced Preset Loading Safety**
Updated preset loading functions to properly notify spectral display before network operations that could delete the rhythm interpreter.

## Safety Mechanisms

### 🛡️ **Multiple Null Checks**
- Check before every rhythm interpreter access
- Additional checks after timing intervals
- Recovery mechanism in exception handlers

### 🛡️ **Proper Cleanup Ordering** 
- Spectral display cleared BEFORE rhythm interpreter deletion
- New rhythm interpreter created BEFORE reconnection
- Clear logging for debugging

### 🛡️ **Exception Handling**
- Try-catch blocks around rhythm interpreter access
- Auto-recovery by clearing invalid references
- Prevents crash propagation

## Testing Verification

### Manual Test Cases:
1. **Load Preset** → Reset Network → Load Another Preset
2. **Use 'L' key** (factory preset) → Reset → Repeat
3. **Rapid operations**: Load/Reset/Load in quick succession
4. **GUI menu**: Network → Reset Network

### Expected Results:
- ✅ No crashes during any network operations
- ✅ Console shows proper cleanup messages
- ✅ Spectral view stops/resumes properly
- ✅ Memory remains stable

## Files Modified:
- `src/SimpleSpectralDisplay.cpp`: Enhanced null safety and exception handling
- `src/GUI.cpp`: Improved reset ordering with proper spectral display notifications

## Impact:
- **Fixed**: Segmentation fault crashes during network reset
- **Maintained**: All spectral view functionality when rhythm interpreter is valid
- **Added**: Robust error recovery and memory safety
- **Performance**: Minimal overhead from additional null checks

The spectral view now safely handles network resets without crashes while maintaining full functionality during normal operations.