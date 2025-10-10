# 🎛️ Connection Matrix GUI User Guide

## Overview
The **Connection Matrix** provides visual, interactive control over how the 8-band rhythm interpreter filterbank connects to your neural network. Each toggle button represents a connection between a specific frequency band and a neuron, with gain sliders for precise control.

## Interface Layout

```
🎛️ Connection Matrix (8×N)
[Clear All] [Random]           <- Quick Actions

        N1    N2    N3         <- Neuron Columns
Sub     [○]   [○]   [●]——|     <- 60Hz (kick fundamentals)
Bass    [○]   [●]   [○]  |     <- 120Hz (kick harmonics)  
LMid    [●]   [○]   [○]  |     <- 250Hz (snare body)
Mid     [○]   [○]   [●]——|     <- 500Hz (snare snap)
UMid    [○]   [●]   [○]  |     <- 1kHz (hi-hat body)
Pres    [●]   [○]   [○]  |     <- 2kHz (hi-hat attack)
Bril    [○]   [○]   [●]——|     <- 4kHz (cymbal shimmer)
Air     [○]   [●]   [○]       <- 8kHz (high-frequency air)
```

## Controls

### Toggle Buttons
- **○ (Empty Circle)**: No connection - this frequency band doesn't affect this neuron
- **● (Filled Circle)**: Active connection - this frequency band influences this neuron
- **Click**: Toggle connection on/off
- **Hover**: Shows tooltip with frequency band description and target neuron
- **Color Intensity**: Brighter green indicates stronger connection weight

### Gain Sliders  
- **Visibility**: Only appear when a connection is active (● button)
- **Range**: 0-100% gain control
- **Real-time**: Changes immediately affect the rhythm interpreter
- **Position**: Located to the right of each toggle button

### Quick Actions
- **Clear All**: Disconnects all frequency bands from all neurons
- **Random**: Creates random connection pattern with varied weights

## Frequency Bands Explained

| **Band** | **Frequency** | **Musical Content** | **Best For** |
|----------|---------------|-------------------|--------------|
| **Sub** | 60Hz | Kick drum fundamentals, sub-bass | Triggering bass-heavy neurons |
| **Bass** | 120Hz | Kick harmonics, basslines | Rhythmic foundation neurons |
| **LMid** | 250Hz | Snare body, tom warmth | Percussive texture neurons |
| **Mid** | 500Hz | Snare attack, vocal clarity | Melodic/lead neurons |
| **UMid** | 1kHz | Hi-hat body, presence | Mid-frequency texture neurons |
| **Pres** | 2kHz | Hi-hat attack, definition | Sharp percussive neurons |  
| **Bril** | 4kHz | Cymbal shimmer, sparkle | High-frequency detail neurons |
| **Air** | 8kHz | Ambience, breath, space | Atmospheric/reverb neurons |

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
1. **Use Random**: Start with random connections to hear possibilities
2. **Enable Recording**: Press 'R' to activate rhythm analysis
3. **Trigger Network**: Use spacebar to generate audio for analysis
4. **Adjust Gradually**: Make small gain changes to hear subtle effects

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

**🎵 The Connection Matrix transforms your neural network into a living, breathing musical organism that responds intelligently to its own audio output!**