# 🎛️ Todd Rhythmogram Filter Gain Controls - User Manual

## 🎯 Enhanced Feature: 5x Range Individual Filter Gain Control

The Todd (1994) Rhythmogram system includes **enhanced 0x-5x gain sliders** for each of the 8 logarithmic frequency bands, providing precise control over rhythmic hierarchy analysis sensitivity with scientific accuracy.

## 🔧 How It Works

### 📍 Location
- **GUI Panel**: Scrollable "Rhythmogram Mapping" panel (press M to toggle)
- **Position**: Horizontal sliders below Todd frequency band labels in scrollable interface
- **Enhanced Range**: 0.0x to 5.0x multiplicative gain with 0.1x step precision (1.0x = unity)

### 🎚️ Todd Rhythmogram Filter Gains (0x-5x Range)

Each Todd frequency band has independent gain control:

| Band | Todd Frequency | Rhythmic Category | 5x Gain Control Purpose |
|------|----------------|-------------------|-------------------------|
| 1 | 0.125 Hz | Ultra-slow modulation | Control 8-second macro-structure sensitivity |
| 2 | 0.25 Hz | Very slow rhythm | Adjust 4-second phrase pattern detection |
| 3 | 0.5 Hz | Slow rhythm | Fine-tune 2-second breathing/phrasing response |
| 4 | 1.0 Hz | Basic pulse | Control fundamental beat/pulse sensitivity |
| 5 | 2.0 Hz | Fast rhythm | Adjust subdivision and syncopation detection |
| 6 | 4.0 Hz | Sub-syllable rate | Control articulation and phoneme sensitivity |
| 7 | 8.0 Hz | Syllable rate | Adjust attack transient and consonant detection |
| 8 | 16.0 Hz | Formant rate | Control micro-timing and tremolo sensitivity |

## 🎵 Musical Applications

### 🎚️ Common Gain Settings

**Boost Macro-Structure Focus** (Ambient/Electronic):
- 0.125Hz-0.5Hz: 2.5x-4.0x (emphasize slow structural patterns)
- 1.0Hz-2.0Hz: 3.2x (strong fundamental pulse and rhythm)
- 4.0Hz: 1.0x (normal articulation sensitivity)
- 8.0Hz-16.0Hz: 0.5x-1.2x (reduce high-rate interference)

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
- **Real-time Control**: Changes apply immediately to Todd rhythmogram analysis
- **Enhanced Range**: Values span 0.0x to 5.0x with 0.1x step precision
- **Multiplicative Processing**: Applied to rhythmogram envelope/biquad outputs before neural scaling
- **Default State**: All Todd filters start at 1.0x (unity gain)
- **Scrollable Interface**: Accessible through TGUI ScrollablePanel with optimized spacing
- **Tempo-Relative Frequencies**: BPM slider (30-260) scales all frequencies proportionally (f = f₀ × bpm/120)
- **Adaptive Filtering**: Filter bank automatically updates frequencies, bandwidths when BPM changes

### Todd Rhythmogram Signal Flow
```
Audio Input → BPM-Scaled Frequencies → Todd Analysis → Envelope/Biquad → Individual Gain (0x-5x) → User Scale (0.0-20.0) → Matrix Toggle → Direct Neural Input
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
- **Too Much Macro-Structure**: Reduce 0.125Hz-0.5Hz gains (0.2x-0.6x)
- **Weak Pulse Response**: Boost 1.0Hz-2.0Hz gains (2.5x-4.5x)
- **Missing Micro-Detail**: Increase 8.0Hz-16.0Hz gains (2.0x-3.5x)
- **Overactive Neural Feedback**: Lower gains in Todd bands that trigger excessively

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

**The Todd (1994) Rhythmogram system now offers scientific-grade temporal hierarchy control with 5x precision gain range!** 🎛️🎵📊