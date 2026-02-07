# GUI2 - Modular Visual Interface for neuronSeqSampler

> A node-based, visual programming interface for the neuronSeqSampler audio system

![Status](https://img.shields.io/badge/status-ready-green)
![C++](https://img.shields.io/badge/C++-17-blue)
![SFML](https://img.shields.io/badge/SFML-2.5+-blue)

## 🎯 Overview

GUI2 is a complete modular GUI system that transforms neuronSeqSampler's components into visual, connectable modules. Build complex audio processing chains by dragging modules onto a canvas and connecting them together.

## ✨ Key Features

- **🎨 Visual Module Canvas**: Drag-and-drop module placement with pan and zoom
- **🔌 Connection System**: Click-and-drag port connections with type validation
- **⚙️ Parameter Windows**: Dedicated configuration windows for each module
- **📐 Grid System**: Toggleable alignment grid for clean layouts
- **🎹 Keyboard Shortcuts**: Fast workflow with keyboard controls
- **🎨 Color-Coded Modules**: Visual distinction between module types

## 🚀 Quick Start

### Build and Run

```bash
# Make build script executable
chmod +x build_gui2.sh

# Build the demo
./build_gui2.sh

# Run GUI2
./build_gui2/gui2_demo
```

### First Steps

1. **Add modules**: Click items in the left panel
2. **Move modules**: Click and drag modules around
3. **Create connections**: Drag from output port (red) to input port (green)
4. **Edit parameters**: Select module and press Enter
5. **Zoom**: Use mouse wheel
6. **Toggle grid**: Press 'G'

## 📦 Available Modules

| Module | Purpose | Color |
|--------|---------|-------|
| 🎵 **AudioManager** | Audio playback and mixing | Purple |
| 🎙️ **Recorder** | Audio recording with filtering | Red |
| 🧠 **NeuronNetwork** | Spiking neural network | Green |
| 🥁 **BeatTracker** | Tempo and beat detection | Orange |
| 📊 **Rhythmogram** | Frequency band analysis | Light Green |
| ⏱️ **Quantizer** | Musical timing quantization | Blue |

## ⌨️ Controls

### Keyboard Shortcuts
- **M**: Toggle module list panel
- **G**: Toggle grid display
- **Enter**: Open parameter window (module selected)
- **Delete/Backspace**: Remove selected module
- **Mouse Wheel**: Zoom in/out

### Mouse Controls
- **Left-click**: Select module or UI element
- **Left-click + drag module**: Move module
- **Left-click + drag port**: Create connection
- **Click empty space**: Deselect all

## 🔗 Connection Examples

### Audio Recording Chain
```
AudioManager → Recorder → AudioManager
```

### Beat Analysis Chain
```
AudioManager → Rhythmogram → BeatTracker → NeuronNetwork
```

### Full Processing Chain
```
Recorder → Rhythmogram → BeatTracker → Quantizer
         ↓                    ↓            ↓
    AudioManager ← NeuronNetwork ←────────┘
```

## 📋 Requirements

- **SFML 2.5+**: Graphics library
- **C++17**: Modern C++ standard
- **CMake 3.10+**: Build system

### Installing Dependencies

**macOS**:
```bash
brew install sfml cmake
```

**Ubuntu/Debian**:
```bash
sudo apt-get install libsfml-dev cmake
```

**Arch Linux**:
```bash
sudo pacman -S sfml cmake
```

## 🏗️ Architecture

```
ModularComponent (base class)
    ↓
Module Wrappers (AudioManagerModule, RecorderModule, etc.)
    ↓
GUI2 (canvas, connections, rendering)
    ↓
GUI2Demo (application entry point)
```

## 📁 File Structure

```
src/
├── ModularComponent.h          # Base class
├── GUI2.h/cpp                  # Main GUI system
├── AudioManagerModule.h        # Module wrappers
├── RecorderModule.h
├── NeuronNetworkModule.h
├── BeatTrackerModule.h
├── RhythmogramModule.h
└── QuantizerModule.h

GUI2Demo.cpp                    # Demo application
CMakeLists_GUI2.txt            # Build configuration
build_gui2.sh                  # Build script
```

## 📖 Documentation

- **[Comprehensive Documentation](GUI2_DOCUMENTATION.md)**: Full system documentation
- **[Quick Reference](GUI2_QUICK_REFERENCE.md)**: Cheat sheet for controls and workflows
- **[Implementation Summary](GUI2_IMPLEMENTATION_SUMMARY.md)**: Technical details
- **[Architecture Diagrams](GUI2_ARCHITECTURE.md)**: Visual architecture reference

## 🎓 Tutorial: Creating Your First Patch

### Example: Beat-Synchronized Audio Playback

1. **Add AudioManager**
   - Click "AudioManager" in module list
   - Module appears in center

2. **Add Rhythmogram**
   - Click "Rhythmogram"
   - Drag it to the right of AudioManager

3. **Connect Audio**
   - Click AudioManager's "Audio Out" (red circle, right side)
   - Drag to Rhythmogram's "Audio In" (green circle, left side)
   - Release to create connection

4. **Add BeatTracker**
   - Click "BeatTracker"
   - Position below Rhythmogram

5. **Connect Onset Data**
   - Connect Rhythmogram "Onset Out" → BeatTracker "Onset In"

6. **Add NeuronNetwork**
   - Click "NeuronNetwork"
   - Position at bottom

7. **Connect Everything**
   - Rhythmogram "Rhythm Out" → NeuronNetwork "Rhythm In"
   - BeatTracker "Beat Phase" → NeuronNetwork "Beat Phase In"

8. **Configure Parameters**
   - Click BeatTracker, press Enter
   - Set Min Tempo: 80 BPM
   - Set Max Tempo: 160 BPM
   - Close parameter window

9. **Save and Enjoy!**

## 🔧 Extending GUI2

### Creating a Custom Module

```cpp
#pragma once
#include "ModularComponent.h"

class MyModule : public ModularComponent {
public:
    MyModule() : ModularComponent("My Module", "Custom") {
        // Define ports
        inputPorts.push_back(Port("In", PortType::AUDIO_IN, 0, true));
        outputPorts.push_back(Port("Out", PortType::AUDIO_OUT, 0, false));
        
        // Define parameters
        parameters.push_back(Parameter("Gain", 1.0f, 0.0f, 2.0f, ""));
        
        moduleColor = sf::Color(200, 200, 100);
    }
    
    void process(float deltaTime) override {
        // Your processing logic here
    }
    
    void receiveInput(int portIndex, const std::vector<float>& data) override {
        // Handle input data
    }
    
    void sendOutput(int portIndex, const std::vector<float>& data) override {
        // Send output data
    }
    
    void renderParameterWindow(sf::RenderWindow& window) override {
        // Custom parameter UI
    }
};
```

Then add to GUI2:
1. Include your module header in GUI2.h
2. Add module type to `availableModuleTypes`
3. Add case in `GUI2::addModule()`

## 🐛 Troubleshooting

**Build fails**:
- Ensure SFML is installed: `pkg-config --modversion sfml-graphics`
- Check CMake version: `cmake --version` (need 3.10+)

**Fonts not loading**:
- Check console for font paths
- Modify `GUI2::loadResources()` to point to valid font

**Modules not connecting**:
- Ensure port types match (audio→audio, control→control)
- Check console for validation errors

## 📊 Performance

- **Target FPS**: 60
- **Module Processing**: Real-time (deltaTime-based)
- **Rendering**: Optimized with culling
- **Memory**: Minimal overhead, smart pointers used

## 🔮 Future Enhancements

- [ ] ImGui integration for advanced UI
- [ ] JSON preset save/load
- [ ] Connection animation and flow visualization
- [ ] Multi-select and group operations
- [ ] Undo/redo system
- [ ] Module search and favorites
- [ ] Performance monitoring per module
- [ ] MIDI support
- [ ] Module scripting (Lua/Python)

## 🤝 Contributing

To contribute to GUI2:
1. Follow existing code style
2. Document new modules thoroughly
3. Test all interactions
4. Update documentation

## 📜 License

This module system is part of neuronSeqSampler and follows the same license.

## 🎉 Acknowledgments

Built on top of:
- **SFML**: Simple and Fast Multimedia Library
- **neuronSeqSampler**: Core audio processing system

---

**Status**: ✅ Production Ready  
**Version**: 1.0.0  
**Last Updated**: 2026-02-07

For questions or issues, refer to the [comprehensive documentation](GUI2_DOCUMENTATION.md).

Happy patching! 🎵
