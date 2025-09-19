# Activation Interval Slider - Feature Documentation

## Overview
Added a real-time GUI slider to control the neural network activation update interval, allowing dynamic adjustment of how frequently the network processes neuron activations and connections.

## Feature Location
**GUI Panel**: Right-side control panel
**Position**: Below the status label, above connection sliders
**Label**: "Update Rate: XXXms"

## Functionality

### Slider Range:
- **Minimum**: 1ms (extremely fast updates - 1000 Hz)
- **Maximum**: 1000ms (slow updates - 1 Hz)  
- **Step Size**: 1ms increments
- **Default**: 100ms (10 Hz)

### Real-Time Control:
- **Live Updates**: Changes take effect immediately
- **Visual Feedback**: Label shows current interval in milliseconds
- **No Restart Required**: Adjustment happens during runtime

## Technical Implementation

### Code Changes:

#### 1. GUI Header (`src/GUI.h`)
```cpp
// Added to private members:
float* activationInterval; // Pointer to main app's activation interval

// Added control elements:
tgui::Slider::Ptr activationIntervalSlider;
tgui::Label::Ptr activationIntervalLabel;

// Updated constructor:
GUI(..., float* activationIntervalPtr);
```

#### 2. GUI Implementation (`src/GUI.cpp`)
```cpp
// Constructor updated to accept activation interval pointer
GUI::GUI(..., float* activationIntervalPtr)
    : ..., activationInterval(activationIntervalPtr) {}

```cpp
// Added to createControlPanel():
activationIntervalSlider = tgui::Slider::create();
activationIntervalSlider->setMinimum(1.0f);    // 1ms minimum
activationIntervalSlider->setMaximum(1000.0f); // 1000ms maximum
activationIntervalSlider->setStep(1.0f);       // 1ms increments

// Real-time callback:
activationIntervalSlider->onValueChange([this](float value) {
    if (activationInterval) {
        *activationInterval = value;
        activationIntervalLabel->setText("Update Rate: " + std::to_string(static_cast<int>(value)) + "ms");
    }
});
```

#### 3. Main Application (`src/main.cpp`)
```cpp
// Updated GUI instantiation to pass activation interval reference:
, guiManager(&gui, &window, &network, &visualizer, &recorder, &audioManager, &activationInterval)
```

## Usage Instructions

### Basic Usage:
1. **Start Application**: Launch NeuronSeqSampler
2. **Locate Slider**: Find "Update Rate" slider in right control panel
3. **Adjust Speed**: Drag slider left (faster) or right (slower)
4. **Observe Changes**: Watch neural network update frequency change in real-time

### Effect on Network Behavior:

#### Ultra-Fast Updates (1-10ms):
- **Behavior**: Extremely rapid, high-frequency network activation
- **Use Case**: Real-time audio synthesis, granular control
- **Audio**: Very fast rhythms, tremolo effects
- **Warning**: High CPU usage, may cause audio artifacts

#### Fast Updates (10-50ms):
- **Behavior**: Rapid, responsive network activation
- **Use Case**: Real-time performance, quick reactions
- **Audio**: Fast-paced rhythms, immediate triggering

#### Medium Updates (50-200ms):
- **Behavior**: Balanced responsiveness and stability  
- **Use Case**: Musical timing, rhythmic patterns
- **Audio**: Natural tempo, musical phrases

#### Slow Updates (200-1000ms):
- **Behavior**: Deliberate, measured activation
- **Use Case**: Ambient sounds, slow evolution
- **Audio**: Sparse triggers, meditative timing

## Visual Feedback
- **Label Updates**: Shows current interval (e.g., "Update Rate: 150ms")
- **Real-Time**: No delay between slider movement and effect
- **Precise Control**: 1ms precision for ultra-fine tuning

## Performance Considerations

### ⚠️ Ultra-Fast Settings Warning (1-10ms):
**Use with caution!** Settings below 10ms can:
- **Overwhelm CPU**: May consume 100% CPU resources
- **Audio Artifacts**: Can cause clicks, pops, and dropouts
- **System Instability**: May freeze or crash the application
- **Recommended Only For**: Experimental sound design, powerful systems

### CPU Usage:
- **Ultra-Fast (1-5ms)**: Very high CPU usage, may overwhelm system
- **Fast (5-50ms)**: Higher CPU usage due to frequent network processing
- **Slower Updates**: Lower CPU usage, more efficient
- **Optimal Range**: 10-200ms for balanced performance

### Audio Quality:
- **Ultra-Fast (1-5ms)**: May cause severe audio artifacts and system instability
- **Very Fast (5-20ms)**: May cause audio artifacts if CPU can't keep up
- **Very Slow**: May feel unresponsive for real-time interaction
- **Recommended**: 10-200ms for most musical applications

## Musical Applications

### Rhythm Control:
- **100ms**: Standard drum machine timing (10 BPM × 60)
- **150ms**: Moderate musical tempo
- **200ms**: Slower, more deliberate rhythms

### Performance Modes:
- **Live Performance**: 50-100ms for responsiveness
- **Composition**: 100-200ms for musical timing
- **Ambient**: 300-500ms for slow evolution

### Pattern Development:
- **Start Slow**: Begin with longer intervals to observe network behavior
- **Speed Up**: Gradually reduce interval to increase complexity
- **Find Sweet Spot**: Adjust to match desired musical timing

## Integration with Other Features

### Works With:
- ✅ **Manual Triggers**: Spacebar and mouse clicks unaffected
- ✅ **Recording**: Both internal and external recording capture timing changes
- ✅ **Connection Weights**: Network topology changes interact with timing
- ✅ **All Neuron Types**: Works with any activation function

### Complements:
- **Connection Weight Sliders**: Adjust both timing and network strength
- **Neuron Addition/Removal**: Change network complexity and timing
- **Recording Features**: Capture different temporal behaviors

## Troubleshooting

### If Slider Doesn't Respond:
- Check that the GUI is properly initialized
- Ensure the application has focus
- Verify the network has neurons to activate

### If Updates Seem Too Fast/Slow:
- **Too Fast**: Increase interval to reduce CPU load
- **Too Slow**: Decrease interval for more responsiveness
- **Unstable**: Try intervals in 50-200ms range

### Performance Issues:
- **High CPU**: Increase activation interval
- **Audio Dropouts**: Use longer intervals (200ms+)
- **Unresponsive**: Decrease interval, check system resources

## Future Enhancements
- **BPM Mode**: Convert interval to beats-per-minute display
- **Sync Mode**: Synchronize with external MIDI clock
- **Automation**: Record interval changes over time
- **Presets**: Save/load common interval settings

This activation interval slider provides real-time control over the temporal characteristics of the neural network, enabling dynamic musical expression and performance flexibility.