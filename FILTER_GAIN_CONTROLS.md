# 🎛️ Filter Gain Controls - User Manual

## 🎯 New Feature: Individual Filter Gain Control

The Rhythm Interpreter now includes **user-controlled gain sliders** for each of the 8 frequency bands, allowing precise control over the sensitivity of each filter.

## 🔧 How It Works

### 📍 Location
- **GUI Panel**: Right side of screen in "Rhythm Interpreter connection matrix" panel
- **Position**: Small horizontal sliders directly below each frequency band label
- **Range**: 0.0x to 2.0x gain (1.0x = unity/normal gain)

### 🎚️ Filter Gain Sliders

Each frequency band now has its own gain control:

| Band | Frequency | Label | Gain Control Purpose |
|------|-----------|--------|---------------------|
| 1 | 1.0 Hz | Ultra (1Hz) | Control subsonic pattern sensitivity |
| 2 | 3.4 Hz | VLow (3Hz) | Adjust very low rhythm detection |
| 3 | 11.3 Hz | Low (11Hz) | Fine-tune slow bass rhythm response |
| 4 | 38.0 Hz | Sub (38Hz) | Control kick drum fundamental sensitivity |
| 5 | 127.8 Hz | Bass (128Hz) | Adjust kick harmonics and bassline |
| 6 | 429.7 Hz | Mid (430Hz) | Control snare attack and vocal sensitivity |
| 7 | 1445.7 Hz | Pres (1.4kHz) | Adjust hi-hat attack and clarity |
| 8 | 8000.0 Hz | Air (8kHz) | Control high-frequency and cymbal sensitivity |

## 🎵 Musical Applications

### 🎚️ Common Gain Settings

**Boost Low-End Focus** (Electronic Music):
- Ultra/VLow/Low: 1.5x-2.0x (emphasize deep bass patterns)
- Sub/Bass: 1.8x (strong kick and bass response)
- Mid: 1.0x (normal snare sensitivity)
- Pres/Air: 0.8x (reduce high-frequency interference)

**Enhance Percussion Clarity** (Acoustic Drums):
- Ultra/VLow: 0.5x (reduce subsonic noise)
- Low/Sub: 1.2x (moderate bass emphasis) 
- Bass/Mid: 1.5x (strong drum body and attack)
- Pres/Air: 1.3x (crisp hi-hats and cymbals)

**Vocal-Focused Mix**:
- Ultra/VLow/Low: 0.3x (minimize low-frequency interference)
- Sub/Bass: 0.8x (reduced bass to avoid vocal masking)
- Mid: 2.0x (maximum vocal clarity and snare presence)
- Pres: 1.6x (enhanced vocal articulation)
- Air: 1.2x (maintain spatial detail)

## 🔧 Technical Details

### Implementation
- **Real-time Control**: Changes apply immediately to audio analysis
- **Range Clamping**: Values automatically limited between 0.0x and 2.0x
- **Multiplicative Gain**: Applied after global gain but before connection matrix
- **Default State**: All filters start at 1.0x (unity gain)

### Signal Flow
```
Audio Input → Filterbank → Individual Gain × Global Gain → Connection Matrix → Neuron Activation
```

### Memory
- **Storage**: Each filter's gain value stored in `std::vector<float> filterGains`
- **Persistence**: Gain settings maintained until manual adjustment
- **Thread Safety**: Real-time safe for audio thread operation

## 🎛️ Usage Tips

### 🎯 Strategic Gain Adjustment
1. **Start with Unity**: Begin with all gains at 1.0x
2. **Identify Problem Frequencies**: Use audio analysis to find overpowering or weak bands
3. **Gradual Adjustment**: Make small changes (±0.2x) and listen to results
4. **Monitor Connections**: Watch connection matrix activity as you adjust gains
5. **Save Presets**: Note successful gain combinations for different music styles

### 🔍 Troubleshooting
- **Too Much Bass**: Reduce Ultra/VLow/Low/Sub gains (0.5x-0.8x)
- **Weak Kick Response**: Boost Sub/Bass gains (1.3x-1.8x)
- **Missing Hi-hat Detail**: Increase Pres/Air gains (1.2x-1.6x)
- **Overactive Connections**: Lower gains in frequency bands that trigger too frequently

## ✨ Benefits

### 🎵 Musical Control
- **Genre Adaptation**: Tailor frequency response to different music styles
- **Mix Balance**: Compensate for poorly balanced input audio
- **Creative Expression**: Emphasize specific frequency ranges for artistic effect
- **Problem Solving**: Reduce unwanted frequency response in specific bands

### 🔧 Technical Advantages
- **Individual Control**: Each filter can be optimized independently
- **Wide Range**: 0.0x to 2.0x covers silence to significant boost
- **Real-time**: Immediate response for live performance and experimentation
- **Preserved Ratios**: Maintains relative balance between connected frequencies

**The Rhythm Interpreter now offers professional-level frequency shaping control!** 🎛️🎵