# GUI2 Modular System Documentation

## Overview

The GUI2 modular system is a complete reimplementation of the neuronSeqSampler interface using a modular, node-based architecture. It wraps existing components (AudioManager, Recorder, NeuronNetwork, BeatTracker, RhythmInterpreter/Rhythmogram, and Quantizer) into modular components that can be visually connected and configured.

## Architecture

### Core Components

1. **ModularComponent** (base class)
   - Abstract base class for all modules
   - Defines ports, parameters, and common interface
   - Handles visual representation and interaction

2. **GUI2** (main GUI class)
   - Manages the module canvas
   - Handles user interaction (drag-drop, connections)
   - Renders modules, connections, and UI panels
   - Maintains module lifecycle

3. **Module Wrappers**
   - AudioManagerModule
   - RecorderModule
   - NeuronNetworkModule
   - BeatTrackerModule
   - RhythmogramModule
   - QuantizerModule

## File Structure

```
src/
├── ModularComponent.h          # Base class for all modules
├── AudioManagerModule.h        # AudioManager wrapper
├── RecorderModule.h           # Recorder wrapper
├── NeuronNetworkModule.h      # NeuronNetwork wrapper
├── BeatTrackerModule.h        # BeatTracker wrapper
├── RhythmogramModule.h        # RhythmInterpreter wrapper
├── QuantizerModule.h          # Quantizer wrapper
├── GUI2.h                     # Main GUI interface
├── GUI2.cpp                   # GUI implementation
GUI2Demo.cpp                   # Demo application
CMakeLists_GUI2.txt           # Build configuration
```

## Features

### Module Canvas

- **Infinite canvas** with pan and zoom support
- **Grid display** for alignment (toggle with 'G')
- **Visual feedback** for selection and interaction
- **Module rectangles** with color-coded types
- **Port visualization**:
  - Green circles: Input ports (left side)
  - Red circles: Output ports (right side)

### Module List Panel

- **Toggleable panel** (press 'M')
- Lists all available module types
- Click to add modules to canvas
- Hover feedback

### Connection System

- **Visual connections** between modules
- **Drag-and-drop** connection creation:
  1. Click on a port (input or output)
  2. Drag to another compatible port
  3. Release to create connection
- **Validation**:
  - Inputs can only connect to outputs
  - No self-connections
  - Port type compatibility

### Parameter Window

- **Per-module parameter editing**
- Open with Enter key when module selected
- Shows all module parameters with:
  - Parameter name and value
  - Visual slider representation
  - Units (Hz, dB, %, etc.)
- Close button

### Keyboard Shortcuts

- **M**: Toggle module list panel
- **G**: Toggle grid display
- **Enter**: Open parameter window for selected module
- **Delete/Backspace**: Remove selected module and its connections
- **Mouse Wheel**: Zoom in/out

### Mouse Controls

- **Left-click**: Select module or UI element
- **Left-click + drag**: Move module or create connection
- **Scroll wheel**: Zoom canvas

## Module Types

### 1. AudioManager Module
**Purpose**: Audio playback and sample management

**Ports**:
- Input: Audio In, Trigger
- Output: Audio Out, Mix Out

**Parameters**:
- Master Volume (0.0 - 2.0)
- Sample Rate (8000 - 96000 Hz)

**Color**: Purple (150, 100, 200)

### 2. Recorder Module
**Purpose**: Audio recording with noise reduction

**Ports**:
- Input: Audio In, Record Trigger
- Output: Audio Out, Recording Status

**Parameters**:
- Sample Rate (8000 - 96000 Hz)
- Noise Gate Threshold (0.0 - 1.0)
- High Pass Frequency (20 - 500 Hz)

**Color**: Red (200, 100, 100)

### 3. NeuronNetwork Module
**Purpose**: Spiking neural network for pattern learning

**Ports**:
- Input: Rhythm In, Control In, Beat Phase In
- Output: Neuron Out, Trigger Out

**Parameters**:
- Learning Rate (0.0 - 1.0)
- Weight Decay (0.0 - 0.01)
- Max Weight (0.1 - 10.0)
- Mapping Gain (0.0 - 2.0)
- Onset Bias (0.0 - 1.0)

**Color**: Green (100, 200, 150)

### 4. BeatTracker Module
**Purpose**: Agent-based beat and tempo tracking

**Ports**:
- Input: Onset In, Audio In
- Output: Beat Phase, Tempo, Beat Trigger

**Parameters**:
- Min Tempo (30 - 300 BPM)
- Max Tempo (30 - 300 BPM)
- Agent Count (1 - 20)
- Adaptation Rate (0.0 - 1.0)
- Confidence Threshold (0.0 - 1.0)

**Color**: Orange (200, 150, 100)

### 5. Rhythmogram Module
**Purpose**: Todd frequency band rhythm analysis

**Ports**:
- Input: Audio In
- Output: Rhythm Out, Onset Out, Band 0-7

**Parameters**:
- Sample Rate (8000 - 96000 Hz)
- Buffer Size (128 - 4096 samples)
- Onset Threshold (0.0 - 1.0)
- Smoothing (0.0 - 1.0)

**Color**: Light Green (150, 200, 150)

### 6. Quantizer Module
**Purpose**: Musical timing quantization

**Ports**:
- Input: Event In, Beat Phase In
- Output: Quantized Out, Timing Out

**Parameters**:
- BPM (30 - 300)
- Grid Resolution (0-5: Half, Quarter, Eighth, 16th, 32nd, 64th)
- Quantize Strength (0.0 - 1.0)
- Swing (0.0 - 1.0)
- Lookahead (0 - 500 ms)

**Color**: Blue (100, 150, 200)

## Port Types

The system defines several port types for type-safe connections:

- **AUDIO_IN/OUT**: Audio sample data
- **CONTROL_IN/OUT**: Control signals (triggers, modulation)
- **RHYTHM_IN/OUT**: Rhythm analysis data
- **ONSET_IN/OUT**: Onset detection data
- **NEURON_IN/OUT**: Neuron network data

## Usage Example

### Creating a Simple Audio Processing Chain

1. **Add an AudioManager module**
   - Click "AudioManager" in module list
   - Module appears at center of canvas

2. **Add a Rhythmogram module**
   - Click "Rhythmogram" in module list

3. **Connect them**
   - Click AudioManager's "Audio Out" port
   - Drag to Rhythmogram's "Audio In" port
   - Connection created

4. **Add a BeatTracker**
   - Click "BeatTracker" in module list
   - Connect Rhythmogram's "Onset Out" to BeatTracker's "Onset In"

5. **Add a NeuronNetwork**
   - Connect BeatTracker's "Beat Phase" to NeuronNetwork's "Beat Phase In"
   - Connect Rhythmogram's "Rhythm Out" to NeuronNetwork's "Rhythm In"

6. **Configure parameters**
   - Click on NeuronNetwork module
   - Press Enter
   - Adjust parameters in parameter window

### Building a Recording Setup

1. Add Recorder module
2. Add AudioManager module
3. Connect AudioManager "Mix Out" to Recorder "Audio In"
4. Set Recorder parameters (noise gate, high-pass filter)
5. Connect Recorder "Audio Out" back to processing chain

## Building and Running

### Build with CMake

```bash
cd /path/to/neuronSeqSampler
mkdir build_gui2
cd build_gui2
cmake -DCMAKE_BUILD_TYPE=Release ../CMakeLists_GUI2.txt
make
./gui2_demo
```

### Build with provided build script

```bash
# Create a build script
cat > build_gui2.sh << 'EOF'
#!/bin/bash
mkdir -p build_gui2
cd build_gui2
cmake -DCMAKE_BUILD_TYPE=Release ../CMakeLists_GUI2.txt
cmake --build .
echo "Build complete. Run with: ./build_gui2/gui2_demo"
EOF

chmod +x build_gui2.sh
./build_gui2.sh
```

## Extending the System

### Adding a New Module

1. Create a new module header (e.g., `MyModule.h`)
2. Inherit from `ModularComponent`
3. Define ports in constructor
4. Define parameters
5. Implement `process()` method
6. Implement `sendOutput()` and `receiveInput()`
7. Implement `renderParameterWindow()`
8. Add to GUI2's `availableModuleTypes` list
9. Add case in `GUI2::addModule()`

### Example Custom Module

```cpp
#pragma once
#include "ModularComponent.h"

class MyCustomModule : public ModularComponent {
private:
    std::vector<float> inputData;
    
public:
    MyCustomModule() 
        : ModularComponent("My Module", "Custom") {
        
        // Define ports
        inputPorts.push_back(Port("In", PortType::AUDIO_IN, 0, true));
        outputPorts.push_back(Port("Out", PortType::AUDIO_OUT, 0, false));
        
        // Define parameters
        parameters.push_back(Parameter("Gain", 1.0f, 0.0f, 2.0f, ""));
        
        moduleColor = sf::Color(200, 200, 100);
    }
    
    void process(float deltaTime) override {
        if (!enabled || inputData.empty()) return;
        
        auto* gainParam = getParameter("Gain");
        float gain = gainParam ? gainParam->value : 1.0f;
        
        // Process audio
        std::vector<float> output = inputData;
        for (float& sample : output) {
            sample *= gain;
        }
        
        sendOutput(0, output);
    }
    
    void sendOutput(int portIndex, const std::vector<float>& data) override {
        // Handled by GUI2 connection system
    }
    
    void receiveInput(int portIndex, const std::vector<float>& data) override {
        if (portIndex == 0) {
            inputData = data;
        }
    }
    
    void renderParameterWindow(sf::RenderWindow& window) override {
        // Custom parameter rendering
    }
};
```

## Future Enhancements

- [ ] ImGui integration for better parameter editing
- [ ] Preset save/load system (JSON)
- [ ] Module templates and favorites
- [ ] Connection animation and flow visualization
- [ ] Multi-select and group operations
- [ ] Undo/redo system
- [ ] Module search/filter
- [ ] Performance monitoring per module
- [ ] Audio routing with buffer management
- [ ] MIDI support
- [ ] Module scripting (Lua/Python)
- [ ] Graph-based signal flow optimization
- [ ] Module library browser with preview

## Troubleshooting

### Fonts not loading
The GUI tries to load system fonts. If you see font warnings, you can:
- Install DejaVu fonts (Linux)
- Point to a specific font file in `GUI2::loadResources()`

### Modules not appearing
- Check console for error messages
- Ensure all module headers are included in GUI2.h
- Verify module constructors are being called

### Connections not working
- Ensure ports are compatible types
- Check that modules implement `receiveInput()` and `sendOutput()`
- Verify connection validation logic

## License

This modular system is part of neuronSeqSampler and follows the same license as the main project.

## Credits

Modular GUI2 system designed and implemented for neuronSeqSampler.
