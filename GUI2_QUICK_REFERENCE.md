# GUI2 Quick Reference

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| **M** | Toggle module list panel |
| **G** | Toggle grid display |
| **Enter** | Open parameter window (module must be selected) |
| **Delete** / **Backspace** | Delete selected module |
| **Mouse Wheel** | Zoom in/out |

## Mouse Controls

| Action | Effect |
|--------|--------|
| **Click module list item** | Add module to canvas |
| **Click module** | Select module |
| **Click + drag module** | Move module |
| **Click port + drag** | Start connection |
| **Release on compatible port** | Complete connection |
| **Click empty space** | Deselect all |

## Module Colors

| Module | Color | Purpose |
|--------|-------|---------|
| **AudioManager** | Purple | Audio playback |
| **Recorder** | Red | Audio recording |
| **NeuronNetwork** | Green | Neural network |
| **BeatTracker** | Orange | Beat tracking |
| **Rhythmogram** | Light Green | Rhythm analysis |
| **Quantizer** | Blue | Timing quantization |

## Port Colors

- **Green circles**: Input ports (left side of module)
- **Red circles**: Output ports (right side of module)

## Typical Signal Chains

### Audio Recording Chain
```
AudioManager → Recorder → AudioManager
```

### Beat Analysis Chain
```
AudioManager → Rhythmogram → BeatTracker → NeuronNetwork
```

### Quantized Playback Chain
```
BeatTracker → Quantizer → NeuronNetwork → AudioManager
```

### Full Processing Chain
```
Recorder → Rhythmogram → BeatTracker → Quantizer
         ↓                    ↓            ↓
    AudioManager ← NeuronNetwork ←────────┘
```

## Tips

1. **Organization**: Use the grid (G key) to align modules neatly
2. **Parameter Tuning**: Select a module and press Enter to access all parameters
3. **Workflow**: Start with AudioManager or Recorder, then add analysis modules
4. **Connections**: Drag from outputs (right, red) to inputs (left, green)
5. **Cleanup**: Delete unused modules to keep canvas organized

## Common Workflows

### Live Audio Analysis
1. Add **Recorder** (for live input)
2. Add **Rhythmogram** (for rhythm analysis)
3. Connect Recorder → Rhythmogram
4. Add **BeatTracker** for tempo
5. Connect Rhythmogram → BeatTracker

### Pattern Learning
1. Set up audio chain (AudioManager or Recorder)
2. Add **Rhythmogram**
3. Add **NeuronNetwork**
4. Connect Rhythmogram rhythm output → NeuronNetwork rhythm input
5. Optionally add **BeatTracker** for phase information
6. Configure NeuronNetwork parameters (learning rate, etc.)

### Quantized Performance
1. Add **BeatTracker** for tempo reference
2. Add **Quantizer**
3. Add **NeuronNetwork**
4. Connect BeatTracker → Quantizer
5. Connect Quantizer → NeuronNetwork
6. Set desired grid resolution in Quantizer

## Port Type Compatibility

| Source Port | Can Connect To |
|-------------|----------------|
| AUDIO_OUT | AUDIO_IN |
| CONTROL_OUT | CONTROL_IN |
| RHYTHM_OUT | RHYTHM_IN |
| ONSET_OUT | ONSET_IN |
| NEURON_OUT | NEURON_IN |

**Note**: Output ports connect to input ports only. Same-type connections only.
