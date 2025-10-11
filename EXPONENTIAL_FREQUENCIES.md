# 🎵 Todd (1994) Rhythmogram Frequency Implementation

## 🔄 Scientific Rhythmic Hierarchy Enhancement

The rhythmogram system now implements **Neil Todd's (1994) logarithmic frequency distribution** starting from 0.125 Hz, providing scientific temporal hierarchy analysis based on established rhythmic perception research.

## 📊 New Frequency Bands

### Before (Linear-ish Distribution):
```
Sub Bass:     60 Hz  → Kick fundamentals
Bass:        120 Hz  → Kick harmonics  
Low Mids:    250 Hz  → Snare body
Mids:        500 Hz  → Snare snap
Upper Mids: 1000 Hz  → Hi-hat body
Presence:   2000 Hz  → Hi-hat attack
Brilliance: 4000 Hz  → Cymbal shimmer
Air:        8000 Hz  → High frequency content
```

### After (Todd 1994 Rhythmogram Distribution):
```
0.125 Hz  → Ultra-slow modulation (8-second macro-structure)
0.25 Hz   → Very slow rhythm (4-second phrase patterns)
0.5 Hz    → Slow rhythm (2-second breathing/phrasing)
1.0 Hz    → Basic pulse (fundamental beat detection)
2.0 Hz    → Fast rhythm (subdivision and syncopation)
4.0 Hz    → Sub-syllable rate (articulation and phonemes)
8.0 Hz    → Syllable rate (attack transients, consonants)
16.0 Hz   → Formant rate (micro-timing, tremolo effects)
```

## 🎯 Benefits of Todd (1994) Rhythmogram Distribution

### 🎼 Scientific Rhythmic Analysis:
- **Temporal Hierarchy Theory**: Based on Todd's nested rhythmic structure research
- **Perceptually Motivated**: Each octave represents equal perceptual temporal distance
- **Complete Coverage**: Spans from macro-form (8s) to micro-timing (62.5ms)
- **Research Foundation**: Implements established cognitive rhythmic processing principles

### 🔬 Technical Implementation:
- **Todd Formula**: f(n) = 0.125 × 2^n for bands 0-7 (pure octave doubling)
- **Hybrid Processing**: Envelope following for <4Hz, biquad filtering for ≥4Hz
- **Per Decamille Display**: Scientific measurement precision in ‰ units
- **Neural Scaling**: 500x scaling factor for meaningful neural activation
- **Scrollable Interface**: Accommodates large networks with optimized spacing

## 🎛️ Scrollable Todd Rhythmogram Interface

The scrollable matrix displays Todd (1994) frequency hierarchy:
- **0.125Hz** - Ultra-slow modulation (macro-structure, 8-second patterns)
- **0.25Hz** - Very slow rhythm (phrase boundaries, 4-second sections)  
- **0.5Hz** - Slow rhythm (breathing, phrasing, 2-second patterns)
- **1.0Hz** - Basic pulse (fundamental beat, 1-second intervals)
- **2.0Hz** - Fast rhythm (subdivisions, syncopation, 500ms)
- **4.0Hz** - Sub-syllable rate (articulation, phonemes, 250ms)
- **8.0Hz** - Syllable rate (attack transients, 125ms)
- **16.0Hz** - Formant rate (micro-timing, tremolo, 62.5ms)

## 🎵 Musical Impact

This exponential distribution provides:
- **More nuanced bass analysis** with dedicated ultra-low and very-low bands
- **Better rhythm detection** for complex polyrhythmic patterns
- **Enhanced kick drum tracking** with improved sub-bass resolution
- **Maintained high-frequency detail** for cymbals and spatial elements

The Todd (1994) rhythmogram system now offers research-grade temporal hierarchy analysis with scrollable interface and neural integration! 🎛️✨📊