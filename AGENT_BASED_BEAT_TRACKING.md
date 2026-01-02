# Agent-Based Beat Tracking and Pattern Finder - Implementation Summary

## Overview
Enhanced the BeatTracker system with an agent-based architecture and pattern detection capabilities. Multiple competing agents maintain different tempo/phase hypotheses and are scored based on onset alignment and recurring rhythmic patterns.

## Architecture

### Agent Class
Represents a beat tracking hypothesis with independent tempo and phase estimates.

**Key Features:**
- **Tempo Hypothesis**: BPM estimate maintained by the agent
- **Phase Hypothesis**: Current beat phase (0.0-1.0, where 0.0 = downbeat)
- **Confidence Score**: Dynamic score (0.0-1.0) based on onset alignment and pattern matching
- **Adaptation**: Gradual refinement of tempo/phase based on new evidence
- **Lifecycle**: Spawned when strong patterns detected, removed when confidence drops below threshold

**Scoring Mechanism:**
```cpp
confidence = 0.6 × onsetAlignmentScore + 0.4 × patternMatchScore
```

- **Onset Alignment Score**: Measures how well strong onsets align with predicted beat positions
- **Pattern Match Score**: Evaluates harmonic relationships between detected patterns and agent's tempo

### PatternFinder Class
Detects recurring rhythmic patterns in onset data to enhance beat tracking accuracy.

**Key Features:**
- **Peak Detection**: Identifies strong onset peaks in the input data
- **Pattern Extraction**: Converts peaks to phase-relative positions
- **Recurrence Analysis**: Scores patterns based on periodic alignment
- **Temporal Analysis**: Examines onset sequences relative to downbeat hypothesis

**Pattern Structure:**
```cpp
struct Pattern {
    vector<float> onsetPositions;  // Positions in phase units (0.0-1.0)
    float period;                   // Pattern period in samples
    float strength;                 // Recurrence score (0.0-1.0)
    int occurrences;                // Number of repetitions detected
};
```

**Pattern Detection Process:**
1. Find peaks in onset data (threshold: 0.2)
2. Extract temporal positions relative to downbeat
3. Calculate recurrence by checking alignment with periodic grid points (0.0, 0.25, 0.5, 0.75)
4. Score pattern strength based on alignment consistency

### BeatTracker Enhancements

**Agent Management:**
- **Maximum Agents**: 5 concurrent hypotheses (configurable via `maxAgents`)
- **Spawn Threshold**: 0.6 confidence required to create new agent
- **Removal Threshold**: 0.1 minimum confidence to survive
- **Deduplication**: Prevents spawning similar agents (within 5 BPM and 0.1 phase)

**Integration Flow:**
1. Every frame:
   - Advance phase for all agents
   - Score agents against onset data and detected patterns
   - Remove weak agents (confidence < 0.1)
   - Update global tempo/phase from best agent

2. Every 200 frames (~2-4 seconds):
   - Run cross-correlation analysis
   - Spawn new agents if strong correlations found (> 0.6)
   - Preprocess signals for pattern detection

**Global State Update:**
The agent with highest confidence influences global parameters:
- Tempo smoothed with factor 0.95
- Phase corrected by 10% toward agent's phase
- Confidence propagated to `phaseConfidence`

## API Reference

### Agent Class

#### Constructor
```cpp
Agent(float tempo, float phase, size_t sampleRate)
```
Creates agent with initial tempo (BPM) and phase (0.0-1.0) hypothesis.

#### Methods
```cpp
float scoreHypothesis(const deque<float>& onsets, const vector<Pattern>& patterns)
```
Updates confidence score based on onset alignment and pattern matching.

```cpp
void adapt(float targetPhase, float targetTempo, float adaptationRate = 0.1f)
```
Gradually adjusts phase and tempo toward targets.

```cpp
void advancePhase(size_t frameSamples)
```
Advances phase based on current tempo and elapsed samples.

#### Accessors
```cpp
float getTempo() const
float getPhase() const
float getConfidence() const
void setConfidence(float conf)
```

### PatternFinder Class

#### Constructor
```cpp
PatternFinder(size_t sampleRate)
```

#### Methods
```cpp
vector<Pattern> findPatterns(const deque<float>& onsets, 
                             float downbeatPhase, 
                             float beatPeriod)
```
Detects patterns in onset data relative to downbeat hypothesis.

```cpp
Pattern getStrongestPattern() const
```
Returns the pattern with highest strength score.

```cpp
void reset()
```
Clears pattern history.

### BeatTracker Extensions

The existing BeatTracker API remains unchanged. Agent management happens internally.

**New Private Members:**
```cpp
vector<unique_ptr<Agent>> agents;        // Active agents
unique_ptr<PatternFinder> patternFinder; // Pattern detection
size_t maxAgents;                        // Max concurrent agents (5)
float agentSpawnThreshold;               // Spawn threshold (0.6)
float agentRemovalThreshold;             // Removal threshold (0.1)
```

**New Private Methods:**
```cpp
void updateAgents()              // Manage agent lifecycle
void spawnAgent(float, float)    // Create new agent
void pruneWeakAgents()           // Remove weak agents
Agent* getBestAgent()            // Get highest confidence agent
void updateFromBestAgent()       // Update global state from best
```

## Algorithm Details

### Agent Onset Alignment Scoring
```
For each sample in recent history (last 4 beats):
  1. Calculate expected phase at that time point
  2. Find distance to nearest downbeat (0.0 or 1.0)
  3. Calculate alignment weight: exp(-20 × distance²)
  4. Score += onset_strength × alignment_weight
Return: normalized score
```

### Agent Pattern Matching Scoring
```
For each detected pattern:
  1. Calculate period ratio: pattern.period / agent.beatPeriod
  2. Check harmonic relationship (0.5×, 1×, 2×, 4×)
  3. Score harmonic match (higher if within 10% of harmonic)
  4. Weight by pattern strength
Return: best harmonic match score
```

### Pattern Recurrence Calculation
```
For each peak position:
  1. Calculate phase within pattern period
  2. Check alignment with grid points (0, 0.25, 0.5, 0.75)
  3. Count aligned peaks (within 10% of grid point)
Return: aligned_peaks / total_peaks
```

### Global State Update
```
best_agent = agent with highest confidence
if best_agent.confidence > 0.3:
  1. Smooth tempo: 0.95 × old + 0.05 × agent.tempo
  2. Correct phase by 10% toward agent.phase
  3. Update phaseConfidence = agent.confidence
  4. Recalculate beatPeriod and phaseVelocity
```

## Performance Considerations

### Real-Time Processing
- **Agent Updates**: O(n) per frame where n = number of agents (max 5)
- **Pattern Detection**: O(m) where m = onset peaks, performed periodically
- **Memory Footprint**: ~100 bytes per agent + pattern storage
- **Frame Budget**: All operations designed for <1ms per frame at 44.1kHz

### Optimizations
1. **Downsampling**: Correlation uses every 4th sample
2. **Limited Lookback**: Agent scoring uses last 4 beats only
3. **Coarse-to-Fine**: Cross-correlation searches with step 50, then refines with step 2
4. **Agent Cap**: Maximum 5 agents prevents unbounded growth
5. **Deduplication**: Prevents redundant similar agents

## Integration with Existing System

### Backward Compatibility
The enhanced BeatTracker maintains full API compatibility:
- All public methods unchanged
- Existing GUI controls work without modification
- BoostTarget enum extended for future use

### Learning Modulation
Pattern emphasis can be directed via BoostTarget:
- **Learning**: Apply boost to learning rate (default)
- **Activation**: Apply boost to neuron activations
- **ConnectionWeights**: Apply boost to connection weights

Pattern-detected beats receive the same phase-based gain as before, now with improved accuracy from agent-based tracking.

## Testing

### Manual Testing
```bash
./test_beat_tracker.sh
```

**Test Scenarios:**
1. **Single Stable Tempo**: Agent count should stabilize at 1-2
2. **Tempo Changes**: New agents spawn, old agents pruned
3. **Polyrhythmic Content**: Multiple agents with harmonic tempos
4. **Weak/Noisy Input**: Agents should maintain stability, confidence may drop

### Expected Behavior

**Agent Lifecycle:**
- Initial agent spawned at 120 BPM, phase 0.0
- Strong correlations (>0.6) trigger new agent spawns
- Weak agents (<0.1 confidence) automatically removed
- Similar agents prevented by deduplication

**Pattern Detection:**
- Patterns require ≥2 occurrences
- Strong onsets (>0.2 threshold) form patterns
- Recurrence scored by alignment with rhythmic grid
- Strongest pattern influences agent confidence

**Global Tracking:**
- Best agent's hypothesis becomes global tempo/phase
- Smooth transitions prevent jitter
- Confidence reflects agent system stability

## Future Enhancements

### Potential Improvements
1. **Hierarchical Tracking**: Separate agents for beat, measure, and phrase levels
2. **Adaptive Thresholds**: Automatically adjust spawn/removal thresholds based on signal characteristics
3. **Pattern Library**: Build database of common rhythmic patterns for genre recognition
4. **Visualization**: Display active agents and their confidence in GUI
5. **Multi-scale Patterns**: Detect patterns at multiple time scales simultaneously
6. **Agent Communication**: Allow agents to share information about pattern discoveries
7. **Persistence**: Save/load agent states with presets

### Configuration Options (Future)
```cpp
void setMaxAgents(size_t max)              // Adjust agent capacity
void setSpawnThreshold(float threshold)    // Control agent creation
void setRemovalThreshold(float threshold)  // Control agent deletion
void setPatternMinOccurrences(int min)     // Pattern detection sensitivity
```

## Files Modified

### Core Implementation
- `src/BeatTracker.h` - Added Agent, Pattern, and PatternFinder classes
- `src/BeatTracker.cpp` - Implemented agent management and pattern detection

### Build System
- `.gitignore` - Added `*.o` to ignore compiled object files

### Documentation
- `AGENT_BASED_BEAT_TRACKING.md` - This file

## Technical Notes

### Memory Management
- All agents managed via `std::unique_ptr` for automatic cleanup
- Pattern vectors cleared periodically to prevent unbounded growth
- Onset history maintains fixed size (3 seconds)

### Thread Safety
Current implementation is single-threaded. For multi-threaded use:
- Protect agent vector with mutex during updates
- Consider lock-free data structures for pattern storage
- Ensure atomic updates to global tempo/phase

### Numerical Stability
- All phase values wrapped to [0, 1) range
- Tempo clamped to [40, 200] BPM
- Confidence clamped to [0, 1]
- Wraparound distance calculations prevent phase discontinuities

## Troubleshooting

### Common Issues

**No agents spawning:**
- Check `agentSpawnThreshold` (default 0.6)
- Verify onset data has sufficient strength (>0.2)
- Ensure correlation analysis is running (every 200 frames)

**Too many agents:**
- Lower `maxAgents` (default 5)
- Increase `agentRemovalThreshold` (default 0.1)
- Check for very noisy input causing spurious correlations

**Unstable tempo tracking:**
- Patterns may be too weak - check `Pattern.strength`
- Agent hypotheses too diverse - increase deduplication tolerance
- Input signal may lack clear periodic structure

**Performance issues:**
- Reduce `maxAgents` to lower computational load
- Increase cross-correlation interval (frameCounter threshold)
- Disable pattern detection if not needed

## Conclusion

The agent-based architecture improves beat tracking robustness by:
1. Maintaining multiple competing hypotheses
2. Leveraging recurring patterns for confidence scoring
3. Adapting to tempo changes more responsively
4. Providing graceful degradation with noisy input

The system maintains real-time performance while offering significantly improved accuracy for complex rhythmic content.
