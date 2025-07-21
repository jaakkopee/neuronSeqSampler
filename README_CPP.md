# Neuron Sequence Sampler (C++ Version)

A visual neuron network simulator that plays audio samples when neurons fire, built with C++ using SFML and TGUI.

## Features

- **Visual Neural Network**: Neurons displayed as circles, connections as lines
- **Real-time Animation**: Neurons highlight when firing, connections show weight strength
- **Audio Integration**: Each neuron triggers audio samples when activated
- **Interactive GUI**: Sliders to adjust connection weights in real-time
- **Manual Control**: Click neurons or use keyboard shortcuts to trigger firing
- **Automatic Activation**: Background processing with configurable intervals

## Requirements

- C++17 compatible compiler
- CMake 3.16 or later
- SFML 2.6+ (graphics, audio, system, window)
- TGUI 1.10+ (for GUI components)

## Building

### Ubuntu/Debian

1. Install dependencies:
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libsfml-dev
```

2. Build and install TGUI from source:
```bash
git clone https://github.com/texus/TGUI.git
cd TGUI && mkdir build && cd build
cmake .. -DTGUI_BACKEND=SFML_GRAPHICS -DTGUI_SHARED_LIBS=TRUE
make -j$(nproc)
sudo make install
sudo ldconfig
```

3. Build the application:
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Running

```bash
./NeuronSeqSampler
```

**Note**: The application requires audio samples in `samples/girliepop/` directory with files named `1.wav` through `6.wav`.

## Controls

- **Mouse**: Click on the visualization area to activate neurons
- **Keyboard**:
  - `1-4`: Directly activate neurons 1-4
  - `Space`: Trigger manual network activation
- **GUI**: Use sliders in the right panel to adjust connection weights (-1.0 to 1.2)

## Architecture

### Core Classes

- **Neuron**: Individual processing unit with activation, threshold, and audio sample
- **Connection**: Weighted connection between neurons
- **NeuronNetwork**: Container managing neurons and connections
- **AudioManager**: SFML-based audio sample loading and playback
- **Visualizer**: SFML graphics rendering of the network state
- **GUI**: TGUI interface for real-time parameter control

### Visual Design

- **Neurons**: Circular nodes that change color (blue → red) when firing
- **Connections**: Lines with opacity indicating weight strength
  - White lines: Positive weights
  - Red lines: Negative weights
  - Transparency: Weight magnitude
- **Layout**: Automatic grid-based positioning for clarity

## Configuration

Default settings can be modified in `main.cpp`:

- `activationInterval`: Time between automatic activations (100ms)
- Neuron parameters: threshold (1.0), decay rate (1.0), initial activation (-0.2)
- Visual settings: neuron radius (20px), colors, canvas area
- Audio: sample directory path and volume levels

## Compatibility

The application includes fallback support for building without TGUI. Set `USE_TGUI=OFF` in CMake to build a version with visualization only (no GUI controls).

## Original Python Version

This C++ implementation is based on the original Python version found in the same repository. The Python version uses:
- Tkinter for GUI
- pydub for audio playback  
- Basic text-based neuron state display

The C++ version provides enhanced visualization, better performance, and more responsive audio handling.