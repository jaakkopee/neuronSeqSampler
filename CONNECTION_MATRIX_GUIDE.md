# 🎛️ Rhythmogram Connection Matrix User Guide

## Overview
The **Rhythmogram Connection Matrix** provides visual, interactive control over how Todd (1994) rhythmogram frequency analysis connects to your neural network. This revolutionary system implements Neil Todd's rhythmic hierarchy principles with 8 logarithmically-distributed frequency bands that scale dynamically with tempo and directly drive neural activation through a scrollable 8×N interface.

## 🆕 New Features (October 2025)

### **Dynamic Frequency Labels**
- **Real-time Updates**: Frequency labels now show actual tempo-scaled frequencies
- **Musical Accuracy**: Labels reflect the frequency values being analyzed, not static defaults
- **Automatic Scaling**: All Todd frequencies scale proportionally with BPM (factor = currentBPM/120)
- **Enhanced Understanding**: See exactly what frequencies are driving which neurons

### **Autodetect Tempo Integration**
- **Automatic BPM Detection**: Toggle autodetect mode for hands-free tempo tracking
- **RhythmDetector Integration**: Uses sophisticated tempo analysis algorithms
- **Dynamic Scaling**: Detected tempo automatically scales all frequency labels and analysis
- **Visual Indication**: BPM display shows 🎵 when autodetect is active

### **Enhanced UI Elements**
- **Large Value Displays**: All parameter displays enlarged to 60×30px for better readability
- **No Scrolling**: Proper sizing prevents unwanted scroll handles on 3-digit values
- **Consistent Design**: Enhanced displays across BPM, Rhythmogram Scale, and all matrix controls

## Scrollable Interface Layout

```
🎛️ Rhythmogram Matrix (8×N) - Scrollable Panel
[Clear All] [Random]           <- Quick Actions

Todd (1994) Frequencies*   N1    N2    N3    [Scroll→]
Phrase (0.104Hz)         [○]   [○]   [●]——|  0.5‰ 
Whole  (0.208Hz)         [○]   [●]   [○]  |  1.2‰
Half   (0.417Hz)         [●]   [○]   [○]  |  2.3‰
Quarter (0.833Hz)        [○]   [○]   [●]——|  3.1‰
Eighth (1.67Hz)          [○]   [●]   [○]  |  1.8‰
16th   (3.33Hz)          [●]   [○]   [○]  |  2.7‰
32nd   (6.67Hz)          [○]   [○]   [●]——|  1.5‰
Onset  (13.3Hz)          [○]   [●]   [○]    0.9‰

*Example at 100 BPM (scaling factor: 0.833x)

Filter Gain Controls: 0.0x─────●─────5.0x (per frequency band)
```

## Controls

### Toggle Buttons (○/●)
- **○ (Empty Circle)**: Connection disabled - rhythmogram feedback completely stopped for this frequency→neuron pair
- **● (Filled Circle)**: Connection enabled - rhythmogram analysis directly drives neural activation
- **Click Response**: Immediate toggle with zero delay - feedback starts/stops instantly
- **Visual Feedback**: Button state synchronized with actual connection weights
- **Tooltip Info**: Shows Todd frequency band and target neuron details
- **Persistence**: Toggle states maintained during all GUI operations and filter adjustments

### Enhanced Filter Gains (0x-5x Range)
- **Location**: Horizontal sliders below frequency band labels
- **Range**: 0.0x to 5.0x multiplicative gain (1.0x = unity)
- **Precision**: 0.1x step increments for fine control
- **Real-time**: Immediate effect on rhythmogram analysis strength
- **Per-band Control**: Each Todd frequency has independent gain adjustment

### Rhythmogram Scale Control (0.0-20.0 Range)
- **Location**: Vertical slider at right end of matrix interface
- **Purpose**: Global scaling factor for rhythmogram-to-neural activation
- **Range**: 0.0 to 20.0 with 0.1 step precision (default: 5.0)
- **Real-time Display**: Numeric value shown below slider
- **Impact**: Higher values = stronger neural response to rhythmic patterns

### BPM Control (30.0-260.0 Range)
- **Location**: Vertical slider next to Scale slider at right end of matrix
- **Purpose**: Tempo-relative scaling of Todd (1994) frequency bands
- **Range**: 30.0 to 260.0 BPM with 0.1 step precision (default: 120.0)
- **Real-time Display**: Numeric BPM value shown below slider
- **Tempo Scaling**: All frequencies scale proportionally (frequency = default × bpm/120)
- **Musical Impact**: Adapts rhythmic hierarchy analysis to current tempo context

### Quick Actions
- **Clear All**: Disconnects all frequency bands from all neurons
- **Random**: Creates random connection pattern with varied weights

## Todd (1994) Rhythmogram Frequencies

| **Frequency** | **Todd Category** | **Rhythmic Hierarchy** | **Neural Application** |
|---------------|-------------------|------------------------|------------------------|
| **0.125Hz** | Ultra-slow modulation | 8-second phrases, macro-structure | Compositional form neurons |
| **0.25Hz** | Very slow rhythm | 4-second sections, verse/chorus | Structural pattern neurons |
| **0.5Hz** | Slow rhythm | 2-second patterns, breathing | Phrase boundary neurons |
| **1.0Hz** | Basic pulse | 1-second beat, fundamental pulse | Primary rhythm neurons |
| **2.0Hz** | Fast rhythm | 500ms subdivisions, eighth notes | Subdivision pattern neurons |
| **4.0Hz** | Sub-syllable rate | 250ms articulations, phonemes | Textural detail neurons |
| **8.0Hz** | Syllable rate | 125ms attacks, consonants | Attack/transient neurons |
| **16.0Hz** | Formant rate | 62.5ms micro-timing, tremolo | Micro-timing neurons |

***Note:** Frequencies shown are base values at 120 BPM. All frequencies scale proportionally with tempo.*

### Dynamic Tempo Scaling
- **Scaling Factor**: All frequencies multiply by (currentBPM / 120)
- **Musical Relationships Preserved**: Frequency ratios remain constant across tempos
- **Real-time Updates**: Labels show actual scaled frequencies automatically
- **Examples**:
  - At 60 BPM: Quarter note = 0.5Hz, Half note = 0.25Hz
  - At 180 BPM: Quarter note = 1.5Hz, Half note = 0.75Hz
  - At 240 BPM: Quarter note = 2Hz, Half note = 1Hz

### Logarithmic Distribution Benefits
- **Perceptually Uniform**: Each octave represents equal perceptual distance
- **Rhythmic Hierarchy**: Matches Todd's nested temporal structure theory
- **Musical Relevance**: Covers complete range from macro-form to micro-timing
- **Tempo Independence**: Relationships maintain across all BPM ranges

## Usage Strategies

### 🥁 **Drum-Focused Routing**
Connect different drum elements to appropriate neurons:
- **Sub/Bass → Kick Neurons**: For powerful low-end response
- **LMid/Mid → Snare Neurons**: For snappy, punchy mid-range
- **Pres/Bril → Hi-hat Neurons**: For crisp, bright high-end

### 🎵 **Frequency Separation**
Assign frequency ranges to different musical roles:
- **Low bands (Sub, Bass)** → Rhythmic foundation
- **Mid bands (LMid, Mid)** → Melodic content  
- **High bands (Pres, Bril, Air)** → Textural elements

### 🔄 **Cross-Modulation**
Create interesting interactions by connecting multiple bands to single neurons:
- **Multiple Low Bands → One Neuron**: Creates complex bass response
- **All Bands → Lead Neuron**: Makes it respond to full spectrum
- **Sparse Connections**: Creates subtle, focused influences

### 🎚️ **Gain Staging**
Use gain sliders to balance frequency influences:
- **High Gain (70-100%)**: Dominant frequency influence
- **Medium Gain (30-70%)**: Balanced blending
- **Low Gain (10-30%)**: Subtle modulation effects

## Real-Time Operation

### Audio Processing Flow
1. **Audio Output**: Network generates audio samples
2. **Filterbank Analysis**: 8 bands extract frequency content
3. **Connection Matrix**: Routes filtered signals based on your settings
4. **Neuron Input**: Filtered signals influence neuron activation
5. **Feedback Loop**: Modified network behavior affects future audio

### Visual Feedback
- **Connection Intensity**: Button brightness indicates connection strength
- **Real-Time Updates**: Matrix reflects current connection state
- **Dynamic Response**: Changes immediately affect audio processing

## Tips & Tricks

### 🎯 **Getting Started**
1. **Open Matrix**: Press 'M' to show scrollable rhythmogram interface
2. **Enable Analysis**: Press 'R' to activate Todd (1994) rhythmogram processing
3. **Generate Audio**: Use spacebar to create network audio for analysis
4. **Configure Connections**: Click toggle buttons (○→●) to enable frequency→neuron routing
5. **Adjust Filter Gains**: Use 0x-5x sliders for per-frequency intensity control
6. **Scroll Interface**: Navigate large networks with smooth scrollable panel

### 🔧 **Troubleshooting**
- **No Effect**: Ensure recording is active (press 'R')
- **Too Subtle**: Increase gain sliders or add more connections
- **Too Chaotic**: Use Clear All and add connections selectively
- **Network Quiet**: Check main neuron activation and connection weights

### 🎨 **Creative Techniques**
- **Rhythmic Pumping**: Connect Sub/Bass to all neurons for beat-driven pulsing  
- **Spectral Dancing**: Connect each band to different neurons for frequency-based choreography
- **Harmonic Layering**: Use multiple bands on melodic neurons for rich harmonic content
- **Dynamic Textures**: Sparse, low-gain connections create evolving ambient textures

---

**🎵 The Todd (1994) Rhythmogram Matrix creates a revolutionary neural-musical feedback system where temporal hierarchy analysis drives network evolution with scientific precision and artistic intuition!**