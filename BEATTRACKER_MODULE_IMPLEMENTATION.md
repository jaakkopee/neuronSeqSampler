# BeatTracker Module Implementation

## Summary

The BeatTrackerModule has been updated to expose all GUI-useful classes and functions from the BeatTracker system, providing comprehensive control over beat tracking and phase-based learning modulation.

## Module Overview

**BeatTrackerModule** wraps the BeatTracker class and exposes:
- Real-time beat phase tracking
- Tempo detection (40-320 BPM)
- Phase-based learning gain modulation
- Agent-based beat hypothesis testing
- Pattern detection and analysis

## Ports

### Input Ports
1. **Network Firings** (CONTROL_IN) - Vector of neuron firing indicators
2. **Input Onsets** (AUDIO_IN) - Vector of onset strengths per frequency band

### Output Ports
1. **Beat Phase** (CONTROL_OUT) - Current position in beat cycle (0.0-1.0)
2. **Tempo** (CONTROL_OUT) - Detected tempo in BPM
3. **Learning Gain** (CONTROL_OUT) - Phase-based learning gain multiplier
4. **Confidence** (CONTROL_OUT) - Confidence in phase estimate (0.0-1.0)

## Parameters

| Parameter | Range | Unit | Description |
|-----------|-------|------|-------------|
| **Enabled** | 0.0 - 1.0 | - | Enable/disable beat tracking |
| **Global Tempo** | 40.0 - 320.0 | BPM | Set global tempo reference |
| **Beat Boost** | 1.0 - 20.0 | x | Learning gain multiplier at downbeat |
| **Phase Window** | 0.01 - 0.5 | - | Width of high-gain window around downbeat |
| **Boost Target** | 0.0 - 2.0 | - | Where to apply boost (0=Learning, 1=Activation, 2=Weights) |

## GUI-Accessible Methods

### Real-time State
```cpp
float getCurrentPhase() const;           // Current beat phase (0.0-1.0)
float getDetectedTempo() const;          // Detected tempo in BPM
float getPhaseBasedLearningGain() const; // Current learning gain multiplier
float getPhaseConfidence() const;        // Confidence in phase estimate
```

### Configuration
```cpp
float getBeatBoost() const;              // Current beat boost value
float getPhaseWindow() const;            // Current phase window width
BoostTarget getBoostTarget() const;      // Current boost target
size_t getAgentCount() const;            // Number of active tracking agents
```

### Pattern Analysis
```cpp
Pattern getStrongestPattern() const;     // Strongest detected rhythmic pattern
```

### Control
```cpp
void resetTracker();                     // Reset all tracking state
```

## Implementation Details

### Beat Tracking Pipeline

1. **Input Processing**
   - Receives network firing vectors and input onset vectors
   - Combines multiple signals into unified activity streams

2. **Cross-Correlation Analysis**
   - Compares network activity with input onsets
   - Detects beat period and phase alignment
   - Uses bipolar preprocessing (peaks and valleys)

3. **Agent-Based Tracking**
   - Multiple agents maintain competing tempo/phase hypotheses
   - Agents score themselves based on onset alignment
   - Best agent determines global tempo and phase
   - Weak agents are pruned, new agents spawned when confidence is high

4. **Pattern Detection**
   - PatternFinder identifies recurring rhythmic structures
   - Patterns boost agent confidence when they match
   - Strongest pattern available for analysis

5. **Learning Modulation**
   - Phase-based gain calculated using Gaussian envelope
   - Boost applied at downbeat (phase = 0.0)
   - Width controlled by Phase Window parameter
   - Can target: Learning Rate, Neuron Activation, or Connection Weights

### Parameter Update Flow

```cpp
process(deltaTime) {
    // 1. Update enabled state
    setEnabled(enabledParam->value > 0.5f)
    
    // 2. Update tempo reference
    setGlobalTempo(tempoParam->value)
    
    // 3. Update modulation parameters
    setBeatBoost(boostParam->value)
    setPhaseWindow(windowParam->value)
    
    // 4. Update boost target
    setBoostTarget(targetParam->value)
    
    // 5. Process inputs
    if (have network firings && input onsets) {
        beatTracker->update(networkFirings, inputOnsets)
    }
}
```

## Boost Target Options

### 0: Learning (BoostTarget::Learning)
- Modulates learning rate of neurons
- Neurons learn faster at downbeat
- Good for rhythm-synchronized plasticity

### 1: Activation (BoostTarget::Activation)
- Modulates neuron firing thresholds
- Neurons more sensitive at downbeat
- Good for phase-locked firing patterns

### 2: Weights (BoostTarget::ConnectionWeights)
- Modulates connection weight updates
- Connections strengthen more at downbeat
- Good for temporal structure learning

## Agent System

The BeatTracker uses multiple concurrent agents to track beats:

- **Agent Spawning**: New agents created when confidence exceeds threshold (0.6)
- **Agent Scoring**: Each agent scores based on onset alignment and pattern match
- **Agent Pruning**: Weak agents (confidence < 0.1) are removed
- **Best Agent**: Highest confidence agent determines global state
- **Max Agents**: Default 5 agents, prevents memory/CPU overhead

## Pattern Structure

```cpp
struct Pattern {
    std::vector<float> onsetPositions;  // Strong onset positions (0.0-1.0 in phase)
    float period;                        // Pattern period in samples
    float strength;                      // Pattern recurrence score
    int occurrences;                     // Number of detections
};
```

## Usage Example

```cpp
// Create module
BeatTrackerModule beatTracker(44100, 512);
beatTracker.initialize();

// Configure
auto* enabledParam = beatTracker.getParameter("Enabled");
enabledParam->value = 1.0f;

auto* tempoParam = beatTracker.getParameter("Global Tempo");
tempoParam->value = 120.0f;  // Set tempo hint

auto* boostParam = beatTracker.getParameter("Beat Boost");
boostParam->value = 10.0f;  // Strong boost at downbeat

// Process
std::vector<float> networkFirings = getNeuronFirings();
std::vector<float> inputOnsets = getOnsetData();

beatTracker.receiveInput(0, networkFirings);
beatTracker.receiveInput(1, inputOnsets);
beatTracker.process(deltaTime);

// Read outputs
float phase = beatTracker.getCurrentPhase();
float tempo = beatTracker.getDetectedTempo();
float gain = beatTracker.getPhaseBasedLearningGain();
float confidence = beatTracker.getPhaseConfidence();
```

## Integration with GUI2

The BeatTracker module integrates seamlessly with the GUI2 modular system:

1. **Visual Representation**: Module appears as colored rectangle with input/output ports
2. **Parameter Window**: Click module + Enter to adjust parameters
3. **Connections**: Connect to NeuronNetwork and Rhythmogram modules
4. **Real-time Feedback**: Parameters update live during processing

## Related Classes

- **Agent**: Individual beat tracking hypothesis with tempo, phase, confidence
- **PatternFinder**: Detects recurring rhythmic patterns in onset data
- **BeatTracker**: Main tracker coordinating agents and patterns

## Benefits

✅ **Temporal Locking**: Network can synchronize with musical beat structure  
✅ **Flexible Modulation**: Choose what to boost (learning, activation, weights)  
✅ **Robust Tracking**: Agent-based system handles tempo changes gracefully  
✅ **Pattern Awareness**: Detected patterns inform tracking confidence  
✅ **GUI Integration**: All parameters accessible via modular interface  

## Next Steps

- Connect BeatTracker to NeuronNetwork learning system
- Visualize beat phase and agents in real-time
- Add pattern visualization to parameter window
- Create preset configurations for different musical styles

---
**Status**: ✅ Complete - BeatTrackerModule fully implements BeatTracker API  
**Build**: ✅ Compiles successfully with SFML 3.0.2  
**GUI Ready**: ✅ All parameters exposed and functional
