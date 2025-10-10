# NeuronSeqSampler

**A real-time neural network audio sampler that creates music through artificial neurons**

NeuronSeqSampler is an experimental audio application that uses interconnected artificial neurons to trigger audio samples. Each neuron can be connected to others, creating complex cascading patterns of rhythms and melodies.

![Neural Network Visualization](https://img.shields.io/badge/Interface-Real--time%20Neural%20Visualization-blue)
![Audio Engine](https://img.shields.io/badge/Audio-SFML%20Based-green)
![Build Status](https://img.shields.io/badge/Build-CMake%20%2B%20C%2B%2B17-orange)

## What Makes It Special

- **Neural Network Audio**: Audio samples are triggered by artificial neurons with realistic activation thresholds and decay
- **Per-Neuron Activation Functions**: Each neuron can use Linear, Sigmoid, ReLU, or Tanh activation functions independently
- **Self-Modulating Neurons**: Neurons can oscillate and evolve automatically with configurable self-activation rates
- **Interactive Visualization**: Real-time rendering of neural network state with curved connections and dynamic colors
- **Weight-Based Animation**: Connections dance and vibrate only when they have meaningful weights
- **Live Patching**: Modify network connections, activation functions, and parameters while the system is running
- **Dual Recording**: Record both external audio input and internal neural network output
- **Testing Mode**: Instant setup with pre-configured drum patterns for immediate experimentation

## Quick Start

### Get Running in 2 Minutes

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install build-essential cmake libsfml-dev libtgui-dev

# Clone and build
git clone <repository-url>
cd neuronSeqSampler
cmake .
make

# Launch with pre-configured network
./NeuronSeqSampler --testing
```

### System Requirements

- **OS**: Linux (tested), macOS (SFML 2.6 required)
- **Compiler**: GCC 7+ or Clang 7+ with C++17 support
- **Audio**: ALSA/PulseAudio (Linux) or CoreAudio (macOS)
- **Graphics**: OpenGL-capable system for real-time visualization

## Usage

### Command Line Options

```bash
./NeuronSeqSampler              # Start with empty network
./NeuronSeqSampler --testing    # Start with pre-configured 3-neuron drum network
```

### Basic Operation

1. **Testing Mode**: Use `--testing` for instant gratification with kick, clap, and 808 samples
2. **Manual Mode**: Build your own network using the GUI menus
3. **Interaction**: Click neurons to activate them, use spacebar for random activation
4. **Recording**: Press 'R' to record the neural network's audio output

### GUI Controls

**Main Interface:**
- **Mouse**: Click neurons to activate, drag to pan view, scroll to zoom
- **Spacebar**: Random network activation
- **R Key**: Toggle audio recording
- **Number Keys (1-9)**: Activate specific neurons
- **Menu System**: Add/remove neurons and connections

**Left Panel - Neuron Controls:**
- **Activation Sliders**: Control self-modulation rate (-0.1 to 0.6) for each neuron
- **Function Dropdowns**: Select activation function (Linear, Sigmoid, ReLU, Tanh) per neuron

**Right Panel - Connection Controls:**
- **Weight Sliders**: Adjust connection strength (-1.2 to 1.2) between neurons
- **Real-time Feedback**: Connections dance only when weights are non-zero

### Sample Organization

Place audio files in organized directories:
```
samples/
├── kick/     # Kick drum samples
├── clap/     # Clap samples
├── 808/      # Bass samples
├── hat/      # Hi-hat samples
└── fx/       # Sound effects
```

## How Neural Networks Make Music

NeuronSeqSampler simulates biological neural networks for music creation:

1. **Neurons** represent individual audio samples with activation thresholds and decay rates
2. **Activation Functions** shape how neurons respond: Linear (direct), Sigmoid (smooth curves), ReLU (hard thresholding), or Tanh (bipolar response)
3. **Self-Modulation** allows neurons to oscillate independently, creating rhythmic patterns without external input
4. **Connections** link neurons with adjustable weights (strength of influence)
5. **Activation** occurs when a neuron receives enough input to exceed its threshold
6. **Cascade Effects** create complex rhythmic patterns as neurons trigger each other
7. **Decay** causes neuron activation to fade over time, creating natural rhythmic spacing
8. **Weight-Based Dynamics** ensure only meaningful connections contribute to the visual and auditory complexity

### Advanced Neural Behaviors

- **Self-Oscillating Neurons**: Positive self-modulation rates create autonomous rhythm generators
- **Self-Dampening Neurons**: Negative self-modulation rates create naturally decaying patterns  
- **Function-Specific Responses**: Different activation functions create unique musical characteristics
- **Dynamic Networks**: Real-time parameter adjustment allows live performance and composition

This creates emergent musical behavior where simple biological principles generate complex, evolving patterns.

## Installation Details

### Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake libsfml-dev libtgui-dev
```

**Fedora:**
```bash
sudo dnf install gcc-c++ cmake sfml-devel tgui-devel
```

**Arch Linux:**
```bash
sudo pacman -S gcc cmake sfml tgui
```

**macOS:** 
Install dependencies via Homebrew, ensure SFML version 2.6 (not 3.x)

### Build Process

```bash
# Create build directory
mkdir -p build && cd build

# Configure and build
cmake ..
make -j$(nproc)

# Return to project root
cd ..

# Test the build
./NeuronSeqSampler --testing
```

## Advanced Features

### Network Visualization

- **Grid View**: Traditional network layout with organized positioning
- **Circular View**: Neurons arranged in a circle for aesthetic visualization
- **Connection Rendering**: Curved lines for bi-directional connections, straight lines for uni-directional
- **Real-time Animation**: Connections pulse and vibrate based on neural activity

### Audio Engine

- **Low-latency Playback**: SFML-based audio system optimized for real-time performance
- **Multiple Sample Support**: Load various audio formats (WAV recommended)
- **Internal Recording**: Capture neural network output directly to WAV files
- **External Recording**: Record microphone input alongside network activity

### Performance Tuning

- **Activation Intervals**: 1ms-1000ms range for controlling update frequency
- **Visual Optimization**: Adjustable zoom and rendering quality
- **Memory Management**: Efficient sample loading and audio buffer management

## Advanced Features

### Per-Neuron Activation Functions

Each neuron can independently use different mathematical functions to process activation:

- **📈 Linear**: `f(x) = x` - Direct, proportional response (default)
- **🌊 Sigmoid**: `f(x) = 1/(1 + e^(-x))` - Smooth saturation curves (0-1 range)
- **⚡ ReLU**: `f(x) = max(0, x)` - Hard thresholding, binary-like behavior
- **🌀 Tanh**: `f(x) = tanh(x)` - Bipolar response with smooth transitions (-1 to 1)

**Musical Applications:**
- **Drum Patterns**: ReLU neurons for sharp, percussive triggers
- **Ambient Textures**: Sigmoid neurons for smooth, flowing progressions
- **Rhythmic Bases**: Linear neurons for predictable, mathematical patterns
- **Dynamic Expression**: Tanh neurons for expressive, bipolar responses

### Self-Modulating Neural Networks

**Autonomous Oscillation:** Neurons can modify their own activation over time:
- **Positive Rates (0.01-0.6)**: Create self-exciting oscillators and rhythm generators
- **Negative Rates (-0.1-0.0)**: Create self-dampening, naturally decaying patterns
- **Zero Rate**: Standard neuron behavior, no self-modification

**Real-time Control:** Adjust self-modulation rates while the network is running to:
- Start/stop rhythmic patterns
- Create evolving musical sequences
- Generate autonomous musical compositions
- Build complex polyrhythmic structures

### Intelligent Visual Feedback

**Weight-Based Animation:** Connections only dance when they carry meaningful signal:
- **Zero Weights**: Static, faint display
- **Non-zero Weights**: Dynamic animation intensity based on neural activity
- **Real-time Response**: Visual feedback matches network topology changes

**Connection Types:**
- **Curved Lines**: Bi-directional connections with Bézier curves
- **Straight Lines**: Uni-directional connections with clean geometry
- **Color Coding**: Weight polarity (white=positive, red=negative)
- **Thickness Variation**: Visual representation of connection strength

## Technical Architecture

### Core Components

- **NeuronNetwork**: Manages neurons and their connections
- **AudioManager**: Handles sample loading and playback
- **Visualizer**: Renders neural network visualization
- **GUI**: Provides real-time parameter controls
- **Recorder**: Manages audio recording functionality

### Neural Network Implementation

Each neuron maintains:
- **Activation Level**: Current charge/energy state
- **Threshold**: Activation trigger point  
- **Decay Rate**: How quickly activation fades after firing
- **Self-Modulation Rate**: Per-iteration activation increase/decrease (-0.1 to 0.6)
- **Activation Function**: Mathematical transformation (Linear, Sigmoid, ReLU, Tanh)
- **Sample Index**: Which audio file to play when activated
- **Activation History**: Rolling buffer for visualization and analysis

Connections define:
- **Source/Target**: Which neurons are linked
- **Weight**: Strength of influence (-1.2 to 1.2, positive/negative)
- **Direction**: Uni or bi-directional flow
- **Visual State**: Dynamic animation based on weight and activity

## Troubleshooting

### Audio Issues
- Ensure audio system (ALSA/PulseAudio) is running
- Check sample file formats (WAV files work best)
- Verify sample directory structure matches expected layout

### Build Issues  
- Confirm C++17 compiler support
- Install all required development libraries
- Use SFML 2.6 on macOS (not 3.x versions)

### Performance Issues
- Increase activation interval (100-200ms) for lower CPU usage
- Reduce zoom level for better rendering performance
- Limit number of simultaneous connections (< 50 for optimal performance)

## Research Background and Inspiration

NeuronSeqSampler draws its theoretical foundation from the pioneering work of **Dr. Pauli Laine**, a Finnish doctor in musicology whose research explores the intersection of neural networks and music composition.

### Academic Foundation
Dr. Laine's doctoral thesis, "[A Method for Generating Musical Motion Patterns](http://hdl.handle.net/10138/19434)" at the University of Helsinki, investigates computational approaches to musical sequence generation.

### Biological Inspiration
The core concept behind NeuronSeqSampler is rooted in **Central Pattern Generators (CPGs)** - specialized neural circuits found in biological systems that produce rhythmic, repetitive outputs with remarkable stability.

### Technical Implementation

The NeuronSeqSampler algorithm implements a novel approach to musical sequence generation by leveraging the dynamics of **spiking neural networks**:

- **Leaky Integrate-and-Fire (LIF) Neurons**: The foundation of our neural network architecture
- **Real-time Sequence Generation**: Interactive composition and performance capabilities  
- **Clock Quantization**: Ensures rhythmic consistency and musical timing
- **Threshold-based Activation**: Mimics biological neuron firing patterns
- **Dynamic Network Topology**: User-configurable connections and weights

This research-backed approach enables NeuronSeqSampler to generate emergent musical patterns that feel both organic and structured, bridging the gap between computational music generation and biological neural dynamics.

## Contributing

This project evolves through experimental features and community feedback. Areas of active development:

- Enhanced neural network algorithms
- Additional audio effects and processing
- Improved visualization modes
- Network preset saving/loading
- MIDI integration possibilities

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

*NeuronSeqSampler - Where artificial intelligence meets experimental music*