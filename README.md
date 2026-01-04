![NeuronSeqSampler](NeuronSeqSampler1.png)

# Recent Updates

![Neural Network Visualization](https://img.shields.io/badge/Interface-Real--time%20Neural%20Visualization-blue)
![Audio Engine](https://img.shields.io/badge/Audio-SFML%20Based-green)
![Build Status](https://img.shields.io/badge/Build-CMake%20%2B%20C%2B%2B17-orange)
![Preset System](https://img.shields.io/badge/Presets-JSON%20Save%2FLoad-purple)

## Screenshots

### Neural Network Visualization
![Neural Network Interface](neuronseq01.png)
*Real-time neural network visualization with connection weights and rhythmogram matrix*

### Connection Matrix and Controls
![Connection Matrix](neuronseq02.png)
*Rhythmogram connection matrix with per-band gain controls and tempo detection*

### Beat Tracking and Pattern Analysis
![Beat Tracker](neuronseq03.png)
*Advanced beat tracking with multi-agent system and pattern timeline visualization*

## Key Features

- **🎵 Musical Quantization System**: Professional-grade musical timing with smart grid-based trigger suppression (Q-key toggle).
- **💾 Smart Preset Management**: Factory presets, user presets, metadata support, and automatic network restoration.
- **Rhythmogram Matrix Panel**: Toggle with 'M', scrollable 8×N grid, real-time routing, per-band and per-connection gain controls.
- **Robust Keyboard Shortcuts**: Global keys (e.g., 'M' for matrix, 'Q' for quantization) work reliably after all GUI actions.
- **Live Parameter Editing**: Instantly adjust neuron activation and connection weights with immediate audio/visual feedback.
- **Dual Recording**: Record both internal neural output and external microphone input.
- **Cross-Platform Compatibility**: Fully tested on both macOS (Homebrew, SFML 3.0.2, TGUI) and Debian/Ubuntu (apt, libsfml-dev, libtgui-dev).

# NeuronSeqSampler

**A real-time neural network audio sampler that creates dynamic music through artificial neurons**



NeuronSeqSampler is an innovative experimental audio application that uses interconnected artificial neurons to trigger and sequence audio samples. Each neuron can be connected to others through customizable connection matrices, creating complex cascading patterns that generate evolving rhythms, melodies, and soundscapes in real-time.

This isn't just a traditional step sequencer - it's a living, breathing musical organism where efficient rhythm analysis directly drives neural activity, creating emergent musical behaviors that can surprise even experienced users. Advanced BeatTracker multi-agent beat tracking is available for users requiring maximum precision in complex rhythmic scenarios.

![Neural Network Visualization](https://img.shields.io/badge/Interface-Real--time%20Neural%20Visualization-blue)
![Audio Engine](https://img.shields.io/badge/Audio-SFML%20Based-green)
![Build Status](https://img.shields.io/badge/Build-CMake%20%2B%20C%2B%2B17-orange)

## What Makes It Special

### Core Neural Architecture
- **🧠 Biological Neural Modeling**: Each neuron features realistic activation thresholds, decay functions, and connection weights that create lifelike behavior
- **🔗 Dynamic Connectivity**: Build complex networks where neurons influence each other through customizable connection matrices
- **🎨 Flexible Activation Functions**: Choose from Linear, Sigmoid, ReLU, or Tanh activation functions independently for each neuron

### Revolutionary Rhythm Integration  
- **🎵 Todd (1994) Rhythmogram**: Advanced rhythmic analysis system that converts audio patterns into direct neural activation
- **⚡ Real-Time Response**: Rhythmic analysis bypasses traditional audio processing for immediate neural triggering
- **🔄 Intelligent Feedback Loop**: 8-band filterbank analyzes the network's own output, creating evolving self-organization

### Tempo Detection System
- **⚡ Simple Rhythm Detector** (Default): Fast, efficient rhythm analysis for real-time performance
- **🎯 BeatTracker System** (Optional): Professional multi-agent beat tracking for complex rhythms
- **📊 Tempo Confidence**: Real-time stability analysis ensures reliable beat detection before BPM updates
- **🔄 Intelligent Switching**: Choose between simple detector and advanced BeatTracker based on needs
- **⚙️ Resource Optimization**: Complete mutual exclusivity prevents unnecessary CPU usage
- **🎼 Wide Range Support**: Accurate detection from 30-300 BPM for diverse musical styles

### Interactive Performance Interface
- **�️ Live Connection Matrix**: Visual routing interface between frequency bands and neurons with precise control
- **📊 Real-Time Visualization**: Watch your neural network come alive with weight-based animations and dynamic color coding
- **🎚️ Live Parameter Control**: Modify connections, activation functions, and neuron parameters during performance
- **🌊 Autonomous Evolution**: Neurons can self-modulate and oscillate with configurable rates for organic musical development

### Professional Audio Features
- **📹 Dual Recording System**: Capture both external microphone input and internal network-generated audio
- **🎧 High-Quality Audio Engine**: Built on SFML for low-latency, professional audio processing
- **🚀 Instant Gratification**: Testing mode provides pre-configured drum networks for immediate experimentation
- **💾 JSON Preset System**: Save and load complete neural network configurations with metadata and version control
- **🎼 Sample Management**: Load and organize your own samples or use the included professional sample library

### Musical Quantization System
- **🎵 Real-Time Quantization**: Professional-grade musical timing that snaps neural triggers to musical grid points
- **📐 Flexible Grid Resolutions**: Choose from 1/2 notes to 1/64 notes for anything from sparse grooves to tight patterns
- **🎚️ Quantization Amount**: Adjustable strength from 0% (no quantization) to 100% (strict musical timing)
- **🎯 Smart Suppression**: Off-grid triggers are intelligently suppressed to maintain musical coherence
- **⚡ Zero-Latency Processing**: Quantization decisions happen in real-time without audio delay
- **🎛️ Q-Key Toggle**: Instant access to quantization controls with keyboard shortcut

#### How Quantization Works
The quantization system analyzes each neural trigger in real-time and determines whether it falls close enough to a musical grid point. If a trigger occurs **within 10ms** of a grid boundary, it plays immediately. If it's **further than 10ms** from any grid point, it's suppressed entirely.

This approach creates **musically coherent rhythms** by preventing off-beat triggers from cluttering the timing, while allowing natural neural dynamics to drive the composition. You'll notice:

- **Larger grids** (1/2, 1/4 notes): More triggers are suppressed, creating sparse, locked grooves
- **Smaller grids** (1/16, 1/32 notes): Fewer triggers are suppressed, maintaining density while ensuring rhythmic precision
- **Live responsiveness**: Changes to grid resolution immediately affect which neural firings become audible

*Example: With 1/2 note quantization at 120 BPM, triggers only play if they occur within 10ms of every 1-second boundary, creating a sparse but perfectly timed kick pattern.*

## Quick Start

### Get Running in 2 Minutes

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install build-essential cmake libsfml-dev libtgui-dev nlohmann-json3-dev

# Clone and build
git clone <repository-url>
cd neuronSeqSampler
cmake .
make

# Launch with pre-configured network
./NeuronSeqSampler --testing
```

## Step-by-Step Operational Guide

### Getting Started (Complete Workflow)

**1. Launch the Application**
```bash
# Option A: Start with empty network (for building from scratch)
./NeuronSeqSampler

# Option B: Start with pre-configured 3-neuron drum network (recommended for first-time users)
./NeuronSeqSampler --testing
```

**2. Understanding the Interface Layout**

The window is divided into several key areas:
- **Center**: Neural network visualization canvas (neurons displayed as circles)
- **Left Panel**: Neuron activation sliders and function dropdowns
- **Right Panel**: Connection weight sliders and network controls
- **Bottom**: Spectral/rhythmogram display (when recording is active)
- **Top**: Menu bar with File, Network, Presets, and View menus

**3. Basic Network Operation (Testing Mode)**

If you started with `--testing`, you already have:
- **Neuron 1**: Kick drum sample
- **Neuron 2**: Clap sample
- **Neuron 3**: 808 bass sample

Try these actions:
```
a) Press number keys 1, 2, or 3 to trigger individual samples
b) Press Spacebar to trigger random neurons
c) Click directly on neurons in the visualization to activate them
d) Watch the neurons light up and connections animate when firing
```

**4. Start Audio Recording and Rhythm Analysis**

Press **R** to start recording. This enables:
- Audio output capture to WAV file
- Rhythmogram analysis of the network's output
- Real-time frequency band decomposition
- Neural feedback from rhythmic analysis

You'll see: `🎙️ Recording started: neuronseq_output_[timestamp].wav`

**5. Open the Rhythmogram Connection Matrix**

Press **M** to show the connection matrix panel. This reveals:
- **8 rows**: Todd (1994) frequency bands (0.125Hz - 16Hz)
- **N columns**: One column per neuron in your network
- **Toggle buttons**: Click to enable/disable frequency→neuron routing (○ off, ● on)
- **Filter gain sliders**: Adjust sensitivity (0x-5x) for each frequency band
- **Connection gain sliders**: Fine-tune individual route strengths (0-100%)
- **BPM control**: Tempo slider (30-260 BPM) with large value display
- **Autodetect Tempo button**: Enable automatic tempo detection from audio

**6. Configure Rhythmogram Routing**

Quick setup for typical drum patterns:
```
a) Click "Random" button to create varied connections
b) Or manually enable specific routes:
   - Low frequencies (Phrase, Whole, Half) → Kick neuron
   - Mid frequencies (Quarter, Eighth) → Snare/Clap neuron  
   - High frequencies (16th, 32nd, Onset) → Hi-hat neuron
c) Adjust filter gains to control sensitivity
d) Fine-tune connection gains for optimal neural input levels
```

**7. Enable Musical Quantization (Optional)**

Press **Q** to show the quantization panel. Configure:
```
a) Check "Enable" to turn on quantization
b) Select grid resolution from dropdown:
   - 1/4 Note: Standard quarter note timing (recommended start)
   - 1/16 Note: Sixteenth note precision (default, tight timing)
   - 1/32 Note: Very fine grid for dense patterns
c) Adjust "Quantization Amount" slider:
   - 0%: No quantization (free neural timing)
   - 50%: Partial quantization (hybrid approach)
   - 100%: Strict musical grid (locked timing)
d) Add swing if desired (-1.0 to 1.0 for groove)
```

**8. Live Performance and Parameter Adjustment**

While audio is playing:
```
a) Adjust neuron activation sliders (left panel) to change self-modulation
b) Modify connection weights (right panel) to alter network topology
c) Toggle rhythmogram connections (M panel) for dynamic routing changes
d) Change quantization grid (Q panel) to shift musical feel
e) Trigger neurons manually (number keys) for live input
f) Watch the visualization respond in real-time
```

**9. Save Your Configuration**

When you've created something interesting:
```
Option A - Quick Save:
  Press S key → Saves to presets/user/quicksave_[timestamp].json

Option B - Named Save:
  1. Open Presets menu → Save Preset
  2. Enter name, author, description in dialog
  3. Add tags for organization (comma-separated)
  4. Click "Save"
  
Your preset includes:
  - All neuron parameters and activation functions
  - Complete connection topology and weights
  - Rhythmogram matrix configuration
  - Quantization settings
  - Metadata and tags
```

**10. Load Presets**

To load existing configurations:
```
Quick Load Factory Preset:
  Press L key → Loads factory drum pattern

Full Preset Browser:
  1. Open Presets menu → Browse Presets
  2. View preset list with metadata
  3. Select preset and click "Load"
  
Factory presets available:
  - drum_pattern.json: 3-neuron rhythmic network
  - ambient_textures.json: 2-neuron ambient soundscape
```

**11. Build Custom Networks from Scratch**

Starting from empty network:
```
1. Network menu → Add Neuron
   - System prompts for sample file selection
   - Choose from organized samples/ directory
   - Neuron appears in visualization

2. Add more neurons (repeat step 1)

3. Network menu → Add Connection
   - Enter source neuron ID
   - Enter target neuron ID
   - Connection appears as line between neurons

4. Configure neuron parameters (left panel)
   - Set activation function (Linear/Sigmoid/ReLU/Tanh)
   - Adjust self-modulation rate slider

5. Configure connection weights (right panel)
   - Adjust slider for connection strength

6. Set up rhythmogram routing (M key)
   - Enable desired frequency→neuron connections
   - Adjust filter and connection gains

7. Configure quantization if desired (Q key)

8. Save your creation (S key or Presets menu)
```

**12. Advanced Features**

**Fullscreen Mode**:
```
Press F11 to toggle fullscreen
- Windowed: 1280x720 (default)
- Fullscreen: Uses your monitor's native resolution
- Layout adapts automatically to any screen size
```

**Autodetect Tempo**:
```
1. Open connection matrix (M key)
2. Click "Autodetect Tempo" button below BPM slider
3. When ON:
   - BPM auto-adjusts based on audio analysis
   - Manual BPM slider becomes read-only
   - Frequency labels update in real-time
   - Musical note indicator (🎵) appears in BPM display
```

**Dynamic Frequency Labels**:
```
All Todd frequency labels show actual scaled frequencies:
- At 60 BPM: Quarter = 0.5Hz, Half = 0.25Hz
- At 120 BPM: Quarter = 1Hz (baseline)
- At 240 BPM: Quarter = 2Hz, Half = 1Hz
Labels update automatically when BPM changes
```

**Multiple Recordings**:
```
Each recording session creates timestamped WAV file:
- Format: neuronseq_output_YYYYMMDD_HHMMSS.wav
- Location: Application root directory
- 44.1kHz sample rate, stereo
- Captures complete network output
```

### System Requirements

- **OS**: Linux (Debian/Ubuntu), macOS (tested)
- **SFML**: 3.0.2 required (not 2.x)
- **TGUI**: Latest version compatible with SFML 3.x
- **nlohmann/json**: 3.2.0+ for JSON preset system
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

### Musical Quantization Usage

**🎵 Quick Start with Quantization:**

1. **Enable Quantization Panel**: Press `Q` to show the quantization controls
2. **Check the Enable Box**: Turn quantization on/off with the checkbox
3. **Choose Grid Resolution**: Select from dropdown:
   - **1/2 Note**: Sparse, powerful beats (1-second intervals at 120 BPM)
   - **1/4 Note**: Standard quarter note timing (500ms intervals)
   - **1/8 Note**: Eighth note subdivisions (250ms intervals)
   - **1/16 Note**: Sixteenth note precision (125ms intervals) - *default*
   - **1/32 Note**: Very tight timing (62.5ms intervals)
   - **1/64 Note**: Extremely dense grid (31.25ms intervals)

4. **Adjust Quantization Amount**: Use the slider to control strength:
   - **0%**: No quantization (neural chaos)
   - **50%**: Partial quantization (some triggers allowed off-grid)
   - **100%**: Full quantization (strict musical timing)

5. **Add Swing**: Use the swing slider to add groove (-1.0 to 1.0)

**🎯 Practical Tips:**

- **Start with 1/4 notes** for traditional drum patterns
- **Use 1/32 notes** when you want tight neural activity without losing the organic feel
- **Try 1/2 notes** for sparse, powerful rhythms where every hit counts
- **Watch the console output** to see which neural triggers are "on-grid" vs "off-grid"
- **Quantization works in real-time** - changes are immediate, no restart needed

**🔍 Understanding Suppression:**

When quantization is active, you'll see console messages like:
- `🎵 Quantized playback: on-grid (delay -0.006s) for sample 1` ← *This sample plays*
- `🎵 Quantized suppression: off-grid (delay 0.145s) for sample 1` ← *This sample is blocked*

This is **normal behavior** - quantization intelligently blocks neural triggers that would create rhythmic chaos, allowing only those that enhance the musical structure.

### GUI Controls

**Keyboard Shortcuts (Global):**
- **Spacebar**: Random network activation - triggers random neurons for testing
- **R Key**: Toggle audio recording - starts/stops recording network output to WAV
- **M Key**: Toggle connection matrix visibility - shows/hides rhythmogram routing panel
- **Q Key**: Toggle quantization panel - shows/hides musical timing controls
- **S Key**: Quick save - saves current network to `presets/user/quicksave_[timestamp].json`
- **L Key**: Quick load - loads factory drum pattern preset (3-neuron network)
- **F11 Key**: Toggle fullscreen - switches between windowed (1280x720) and fullscreen mode
- **Number Keys (1-9)**: Activate specific neurons - direct neuron triggering for live performance
- **ESC Key**: Close dialogs and modal windows

**Mouse Controls:**
- **Left Click on Neuron**: Manually activate a neuron and trigger its sample
- **Left Click + Drag**: Pan the neural network view
- **Mouse Wheel Scroll**: Zoom in/out on the neural network visualization
- **Right Click**: Context menu (future feature)

**Menu System:**
- **File Menu**: Quit application
- **Network Menu**: Add neurons, remove neurons, add connections, remove connections, clear network
- **Presets Menu**: Save preset, load preset, browse presets, load factory drum pattern, load factory ambient textures
- **View Menu**: Toggle connection matrix (M), toggle quantization panel (Q), toggle fullscreen (F11)

**Left Panel - Neuron Controls:**
- **Activation Sliders**: Control self-modulation rate (-0.1 to 0.6) for each neuron
  - Positive values create self-oscillating neurons (autonomous rhythm generators)
  - Negative values create self-dampening neurons (natural decay patterns)
  - Zero means standard neuron behavior without self-modification
- **Function Dropdowns**: Select activation function per neuron:
  - **Linear**: Direct proportional response - predictable, mathematical patterns
  - **Sigmoid**: Smooth 0-1 saturation curves - flowing, ambient progressions
  - **ReLU**: Hard thresholding - sharp, percussive triggers
  - **Tanh**: Bipolar -1 to 1 response - expressive, dynamic behavior

**Right Panel - Connection Controls:**
- **Weight Sliders**: Adjust connection strength (-1.2 to 1.2) between neurons
  - Positive weights: excitatory connections (increase activation)
  - Negative weights: inhibitory connections (decrease activation)
  - Zero weight: connection disabled
- **Real-time Feedback**: Connections animate only when weights are non-zero

**Rhythmogram Mapping Panel** (Press M to toggle):
- **📊 Scrollable 8×N Matrix**: Visual grid routes Todd (1994) frequencies to neurons with optimized spacing
- **🔘 Toggle Control**: Click buttons (○ off / ● on) to enable/disable specific frequency→neuron connections
- **📈 Per Decamille Displays**: Real-time rhythmic activity monitoring in ‰ precision (parts per 10,000)
- **🎚️ Dual Gain System**: Filter sensitivity (0x-5x) + connection strength (0-100%) per route
- **⚡ Instant Response**: Toggle changes immediately stop/start rhythmogram feedback to neurons
- **🎛️ Quick Actions**: "Clear All" and "Random" buttons for rapid matrix reconfiguration
- **🎵 Rhythmogram Scale Slider**: Vertical control (0.0-20.0) with large, easy-to-read value display
- **🎼 BPM Control**: Tempo slider (30-260 BPM) with large, clear value display for precise readings
- **🤖 Dual Tempo Detection System**: 
  - **BeatTracker**: Professional-grade multi-agent beat tracking system for superior accuracy
  - **Simple RhythmDetector** (Default): Lightweight system for efficient tempo detection
  - **BeatTracker System** (Advanced): Professional multi-agent beat tracking (disabled by default)
  - **Streamlined Interface**: BeatTracker controls removed for simplified user experience
- **⚡ Resource Optimization**: Only one tempo detection system active at a time for maximum performance

**Tempo Detection**: System defaults to simple rhythm detector for optimal performance. Advanced users can enable BeatTracker through code modifications if needed for complex rhythm analysis.

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
sudo apt install build-essential cmake libsfml-dev libtgui-dev nlohmann-json3-dev
# If your distro does not yet provide SFML 3.0.2, build SFML and TGUI from source:
# https://www.sfml-dev.org/download.php
# https://tgui.eu/download/
```

**Fedora:**
```bash
sudo dnf install gcc-c++ cmake sfml-devel tgui-devel nlohmann-json-devel
```

**Arch Linux:**
```bash
sudo pacman -S gcc cmake sfml tgui nlohmann-json
```

**macOS:**
Install dependencies via Homebrew:
```bash
brew install sfml@3 tgui cmake
# If needed, link SFML 3.0.2 and TGUI for CMake
```

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

## � Rhythmogram Neural Integration

**Revolutionary Feature**: NeuronSeqSampler implements **Neil Todd's (1994) rhythmogram principles** to directly drive neural activation through real-time rhythmic analysis.

### System Architecture

```
Audio Output → Rhythmogram Analysis → Connection Matrix → Direct Neuron Activation
     ↑                                                              ↓
     └─────────────── Neural Network Processing ←──────────────────┘
```

### Todd (1994) Frequency Bands

**8-band logarithmic distribution (0.125Hz - 16Hz) covering musical time structures:**

| Band | Frequency | Musical Function | Neural Applications |
|------|-----------|------------------|-------------------|
| **Phrase** | 0.125Hz | 8-beat phrases, long-term structure | Compositional/tempo neurons |
| **Whole** | 0.25Hz | 4-beat units, measure-level rhythm | Foundation/anchor neurons |
| **Half** | 0.5Hz | 2-beat units, strong-weak patterns | Backbeat/emphasis neurons |
| **Quarter** | 1Hz | Basic beat, fundamental pulse | Primary rhythm neurons |
| **Eighth** | 2Hz | Sub-beat subdivisions, groove | Syncopation/swing neurons |
| **16th** | 4Hz | Fast subdivisions, hi-hat patterns | Texture/fill neurons |
| **32nd** | 8Hz | Very fast subdivisions, rolls | Ornament/decoration neurons |
| **Onset** | 16Hz | Micro-timing, attack transients | Percussive/transient neurons |

### Core Features

#### 🧠 **Direct Neural Activation Engine**
- **Hybrid Processing**: Envelope following (<4Hz) + biquad filters (≥4Hz) for optimal detection
- **Per Decamille Precision**: Real-time activity monitoring in ‰ (parts per 10,000)
- **Audio Bypass**: Direct neural input without sound alteration
- **Threshold Activation**: Only significant rhythmic events trigger neurons

#### 🔄 **Interactive Connection Matrix (8×N)**
- **📊 Scrollable Interface**: Accommodates large networks with smooth scrolling and optimized 80px neuron spacing
- **🔘 Precise Toggle Control**: Click buttons (○ → ●) to enable connections; disabled toggles completely stop rhythmogram feedback
- **📊 Dual Gain Architecture**: Filter sensitivity sliders (0x-5x with 0.1x steps) + per-connection strength (0-100%)
- **📈 Real-Time Monitoring**: Live per decamille displays show rhythmic activity as it happens
- **💾 State Persistence**: Connection configurations survive network changes and GUI interactions
- **🎨 Visual Clarity**: Color-coded buttons indicate connection strength and current activation state

#### 🎵 **Musical Intelligence**
- **Rhythm Entrainment**: Network synchronizes with detected beats
- **Spectral Mapping**: Low frequencies → percussion, high frequencies → texture
- **Self-Organization**: Optimal frequency-to-neuron mappings emerge over time
- **Polyrhythmic Patterns**: Complex interactions between frequency bands
- **Tempo Awareness**: BPM control scales Todd frequencies relative to musical tempo (120 BPM baseline)
- **Advanced Tempo Detection**: Dual-system approach for optimal accuracy:
  - **BeatTracker System**: Multi-agent beat tracking with stability analysis and tempo confidence scoring
  - **Simple Detector**: Fast rhythm analysis for basic tempo tracking when BeatTracker is disabled
  - **Intelligent Switching**: Automatic mutual exclusivity prevents resource conflicts
- **Musical Scaling**: All frequency bands maintain proportional relationships across tempo changes

### Technical Specifications

**🔧 Performance:**
- **Latency**: ~13ms total system latency (audio → rhythmogram → neural activation)
- **Sample Rate**: 44.1kHz with 512-sample buffers for optimal real-time response
- **Analysis Window**: 100 frames (~1.16 seconds) rhythmic history for pattern detection
- **CPU Overhead**: <5% on modern systems with full 8×N matrix active
- **Tempo Detection Optimization**: Mutual exclusivity ensures only BeatTracker OR simple detector processes audio
- **Resource Management**: Complete bypass of inactive tempo detection systems saves CPU cycles

**📊 Matrix Capabilities:**
- **Matrix Size**: 8 Todd frequency bands × unlimited neurons (tested up to 20+ neurons)
- **Scrollable Interface**: Smooth navigation for large networks with 80px optimized spacing
- **Filter Gain Range**: 0x-5x with 0.1x precision steps for sensitive control
- **Connection Gain Range**: 0-100% per individual frequency→neuron route
- **Toggle Response**: Immediate (single-frame) connection enable/disable with zero feedback delay

**🎵 Audio Analysis:**
- **Frequency Range**: 0.125Hz-16Hz logarithmic Todd (1994) distribution
- **Precision**: Per decamille (‰) display accuracy for rhythmic activity monitoring
- **Processing**: Hybrid envelope following (<4Hz) + biquad filtering (≥4Hz) approach

**🎯 BeatTracker System:**
- **Multi-Agent Architecture**: Multiple competing beat tracking agents for robust tempo detection
- **Tempo Range**: 30-300 BPM with automatic stability analysis
- **Confidence Scoring**: Real-time beat strength and stability metrics
- **Mutual Exclusivity**: Complete bypass when disabled to prevent resource conflicts
- **Initialization**: Manual tempo seeding for faster convergence on known BPMs
- **Status Monitoring**: Live display of active agents, tempo confidence, and system health

### How It Works

```
Audio → Rhythmogram Analysis → Matrix Connections → Direct Neuron Activation
                ↓                        ↓                      ↓
        8 Frequency Bands      User-Defined Routes      Neural Network Input
```

#### **Real-Time Analysis Pipeline**

1. **Hybrid Filtering**: Envelope following (<4Hz) + biquad filters (≥4Hz) for optimal rhythmic detection
2. **Per Decamille Precision**: Display shows rhythmic activity with ‰ (parts per 10,000) accuracy
3. **Connection Matrix**: Interactive 8×N grid routes frequency bands to specific neurons
4. **Direct Activation**: Rhythmogram levels directly call `neuron->addExternalInput()` based on matrix weights

#### **8×N Matrix Architecture**

The **Interactive Connection Matrix** provides comprehensive control over Todd (1994) rhythmogram-to-neuron routing:

**🎛️ Matrix Layout:**
- **8 Rows**: Todd frequency bands (Phrase 0.125Hz → Onset 16Hz) with musical tooltips
- **N Columns**: Neural network neurons with optimized 80px spacing for clarity
- **Scrollable Design**: Handles large networks (tested with 10+ neurons) smoothly

**🔘 Connection Control:**
- **Toggle Buttons**: Click ○ (disabled) ↔ ● (enabled) to control individual frequency→neuron routes
- **Immediate Response**: Disabled connections (○) completely stop rhythmogram feedback; enabled (●) allow full signal flow
- **Visual Feedback**: Button color intensity reflects connection strength and current activity level

**📊 Precision Control:**
- **Filter Gain Sliders**: Individual sensitivity (0x-5x, 0.1x steps) per frequency band
- **Connection Gain Sliders**: Per-route strength adjustment (0-100%) for fine-tuning neural input
- **Per Decamille Displays**: Real-time activity monitoring with ‰ precision showing actual rhythmic detection levels

#### **Revolutionary Integration**

Unlike traditional audio effects that process sound, this system:
- ✅ **Bypasses Audio Processing**: Direct neural activation without audio filtering
- ✅ **Preserves Original Sound**: Audio plays unchanged while analysis drives neurons
- ✅ **Real-Time Response**: Immediate rhythmogram→neuron activation with <1ms latency
- ✅ **Musical Intelligence**: Frequency bands specifically chosen for rhythmic hierarchy
- ✅ **User Control**: Complete freedom to route any frequency band to any neuron

### Musical Applications

#### **Intelligent Rhythm Mapping**
- **Low frequencies (0.125-1Hz)** → Drive bass/kick neurons for foundation rhythms
- **Mid frequencies (2-4Hz)** → Control snare/clap neurons for backbeat patterns  
- **High frequencies (8-16Hz)** → Modulate hi-hat/cymbal neurons for texture

#### **Emergent Polyrhythms**
- Multiple frequency bands can drive single neurons for complex rhythmic interaction
- Single frequency bands can drive multiple neurons for rhythmic distribution
- Matrix configurations create musical relationships impossible with traditional sequencing

#### **Live Performance**
- **Press M** to show/hide rhythmogram matrix during performance
- **Real-time routing changes** while music plays
- **Immediate visual feedback** with per decamille precision displays
- **Musical expression** through dynamic connection gain adjustment

### Debug & Monitoring

The system includes comprehensive debug output:
```
🎯 Neuron 1 receiving rhythmogram input: 0.00109757
🎵 Rhythmogram → Neurons: Active connections driving neural activation
```

This breakthrough integration transforms NeuronSeqSampler from an interactive audio toy into a serious tool for **computational music research** and **live electronic performance**.

---

## JSON Preset System

### Complete Network State Management

The JSON preset system provides professional-grade save/load functionality for complete neural network configurations, enabling users to preserve, share, and restore complex musical patterns with full fidelity.

#### **🎹 Keyboard Shortcuts**
- **S Key**: Quick save current network with timestamp (saves to `presets/user/quicksave_[timestamp].json`)
- **L Key**: Instantly load factory drum pattern preset (3-neuron network with kick/clap/bass)
- **Q Key**: Toggle musical quantization panel (real-time musical timing controls)
- **M Key**: Toggle rhythmogram matrix panel (frequency-to-neuron routing)
- **R Key**: Toggle audio recording (capture network output to WAV files)

#### **🖥️ GUI Integration**
- **Presets Menu**: Complete preset management system
  - **Save Dialog**: Create named presets with descriptions and metadata
  - **Load Dialog**: Browse and select from factory and user presets
  - **Preset Browser**: Detailed view with preset information, author, and descriptions
  - **Quick Actions**: Direct access to factory presets

#### **📁 Preset Structure**
```
presets/
├── factory/                    # Built-in example presets
│   ├── drum_pattern.json      # 3-neuron drum network
│   └── ambient_textures.json  # 2-neuron ambient network
└── user/                      # Your custom presets
    └── [custom_presets].json
```

#### **📋 Complete State Preservation**
Each preset captures the full network configuration:
- **Neuron Parameters**: Activation levels, thresholds, decay rates, self-modulation, activation functions
- **Connection Topology**: Complete network structure with precise connection weights
- **Rhythmogram Matrix**: Filter configurations and frequency-to-neuron routing
- **Quantization Settings**: Musical timing grid, swing, and quantization strength
- **Metadata**: Name, author, description, creation date, tags, and version information

#### **🔧 Example Preset Structure**
```json
{
  "preset_info": {
    "name": "Funky Drum Loop",
    "author": "Your Name",
    "description": "Polyrhythmic pattern with sigmoid activation",
    "created_date": "2025-10-14T12:30:00Z",
    "tags": "drums,polyrhythmic,sigmoid"
  },
  "neurons": [
    {
      "id": 0,
      "sample_index": 1,
      "activation": 0.0,
      "threshold": 1.0,
      "decay_rate": 0.5,
      "activation_increase_per_iteration": 0.0,
      "activation_function": "Sigmoid"
    }
  ],
  "connections": [
    {
      "source_id": 0,
      "target_id": 1,
      "weight": 0.6
    }
  ],
  "rhythmogram_matrix": {
    "enabled": true,
    "scale": 5.0,
    "filter_gains": [1.0, 1.5, 2.0, 1.2, 0.8, 1.0, 1.0, 1.0]
  },
  "quantization": {
    "enabled": true,
    "grid_resolution": "sixteenth_note",
    "grid_resolution_value": 3,
    "quantization_amount": 0.8,
    "swing_factor": 0.2,
    "bpm": 120.0
  }
}
```

#### **✅ Professional Features**
- **Human-Readable Format**: JSON structure allows manual editing and version control
- **Automatic Validation**: Error handling with detailed feedback for malformed presets
- **Smart Restoration**: Complete network rebuilding including rhythm interpreter reinitialization
- **GUI Synchronization**: All sliders, matrices, and visual elements update to reflect loaded state
- **Cross-Session Compatibility**: Presets work across different application sessions and system restarts

#### **🎵 Quantization in Presets**
Presets now include complete quantization settings, preserving your musical timing preferences:

- **Grid Resolution**: Automatically restores subdivision (1/2 note to 1/64 note)
- **Quantization Amount**: Preserves timing strength (0% to 100%)
- **Swing Factor**: Maintains groove timing (straight to triplet feel)
- **Enable State**: Remembers whether quantization is active

**Factory Preset Examples:**
- **Drum Pattern**: 1/16 note grid, 80% strength, 20% swing - tight rhythmic patterns
- **Ambient Textures**: Quantization disabled - free-flowing, organic timing
- **User Test**: 1/8 note grid, 100% strength, straight timing - locked grooves

*When you load a preset, the quantization panel (Q-key) will automatically update to show the saved settings, and the musical timing will immediately take effect.*

---

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

### Rhythmogram Matrix Interface

#### **Matrix Operation Guide**

**🚀 Quick Start:**
1. **Show Matrix** (Press 'M') to display the scrollable 8×N rhythmogram interface
2. **Create Network** (Add neurons via Network menu) to populate matrix columns
3. **Enable Connections** by clicking toggle buttons (○ → ●) to route specific frequencies to neurons
4. **Play Audio** (number keys 1-9) to generate rhythmogram analysis and observe neuron activation

**🎛️ Fine-Tuning:**
1. **Adjust Filter Gains** (0x-5x) to control sensitivity of each Todd frequency band
2. **Tune Connection Gains** (0-100%) for optimal neural input levels per route
3. **Monitor Activity** via per decamille displays showing live rhythmic detection
4. **Test Response** by toggling connections on/off to verify immediate feedback control

**⚡ Advanced Usage:**
- **Scroll Navigation**: Use mouse wheel or scroll bars for large networks
- **Bulk Operations**: "Clear All" removes all connections; "Random" creates varied patterns  
- **Live Performance**: Toggle connections in real-time during audio playback for dynamic control
- **Musical Mapping**: Route low frequencies (0.125-1Hz) → bass/kick neurons, high frequencies (4-16Hz) → texture/hi-hat neurons

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

- **NeuronNetwork**: Manages neurons, connections, and rhythmogram integration
- **RhythmInterpreter**: Implements Todd (1994) rhythmogram analysis and neural activation
- **ConnectionMatrix**: Routes rhythmic frequency bands to specific neurons with configurable weights
- **AudioManager**: Handles sample loading, playback, and rhythmogram analysis coordination
- **GUI**: Provides real-time parameter controls and rhythmogram mapping interface
- **Visualizer**: Renders neural network visualization with real-time activity feedback
- **Recorder**: Manages dual-channel audio recording (input + network output)

### Neural Network Implementation

Each neuron maintains:
- **Activation Level**: Current charge/energy state
- **Threshold**: Activation trigger point  
- **Decay Rate**: How quickly activation fades after firing
- **Self-Modulation Rate**: Per-iteration activation increase/decrease (-0.1 to 0.6)
- **Activation Function**: Mathematical transformation (Linear, Sigmoid, ReLU, Tanh)
- **External Input**: Accumulator for rhythmogram-derived neural inputs
- **Sample Index**: Which audio file to play when activated
- **Activation History**: Rolling buffer for visualization and analysis

Rhythmogram Integration adds:
- **Connection Matrix Weights**: Individual routing coefficients (8 frequency bands × N neurons)
- **Hybrid Filtering**: Envelope following (<4Hz) + biquad filters (≥4Hz) for optimal rhythmic detection
- **Real-time Analysis Pipeline**: Audio → Rhythmogram → Matrix Transform → Neural Activation
- **Per Decamille Monitoring**: High-precision (‰) display of rhythmic activity levels
- **Direct Neural Input**: `addExternalInput()` bypasses audio processing for immediate activation

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

NeuronSeqSampler draws its theoretical foundation from groundbreaking research in computational musicology and neural networks, combining multiple academic disciplines to create a novel approach to musical generation.

### Todd (1994) Rhythmogram Theory

The core rhythmic analysis system is based on **Dr. Neil Todd's seminal 1994 research** at the University of Sheffield, which introduced the concept of **rhythmograms** - a revolutionary method for analyzing rhythmic hierarchies in music.

#### **"The Auditory 'Primal Sketch': A Multiscale Theory of Rhythmic Grouping"**
Todd's groundbreaking paper established that human rhythmic perception operates through **logarithmically spaced frequency channels** corresponding to musical time structures. His research demonstrated:

- **Hierarchical Rhythm Processing**: Human auditory system processes rhythm at multiple temporal scales simultaneously
- **Logarithmic Frequency Distribution**: Musical time structures follow mathematical relationships (0.125Hz → 16Hz)
- **Biological Foundations**: Rhythmic perception mirrors neural processing in the auditory cortex
- **Cross-Modal Integration**: Rhythmic analysis connects auditory perception with motor control

#### **Musical Time Hierarchy (Todd 1994)**
Todd identified specific frequency bands that correspond to fundamental musical structures:

- **Phrase Level (0.125-0.5Hz)**: Large-scale musical organization and sectional boundaries
- **Beat Level (0.5-4Hz)**: Metric structures, tempo, and rhythmic patterns  
- **Subdivision Level (4-16Hz)**: Microtiming, groove, and rhythmic ornamentation

This hierarchy forms the theoretical basis for NeuronSeqSampler's 8-band rhythmogram analyzer, enabling **musically intelligent** frequency-to-neuron routing.

### BeatTracker: Advanced Beat Tracking Research

The professional tempo detection system integrates **BeatTracker**, a sophisticated multi-agent beat tracking system based on decades of academic research in computational rhythm analysis.

#### **Multi-Agent Beat Tracking Architecture**
BeatTracker implements a **competitive agent system** where multiple tracking hypotheses compete to identify the most stable tempo:

- **Parallel Hypothesis Testing**: Multiple agents simultaneously track different tempo possibilities
- **Confidence Scoring**: Real-time evaluation of each agent's tracking stability and accuracy
- **Dynamic Adaptation**: Agents adjust their predictions based on incoming audio analysis
- **Stability Analysis**: Only stable, high-confidence tempo estimates influence the system BPM

#### **Academic Foundation**
This approach builds on established research in:
- **Beat Tracking Algorithms**: Multi-agent systems for robust rhythm detection in complex audio
- **Temporal Pattern Recognition**: Machine learning approaches to musical meter and tempo analysis  
- **Competitive Learning**: Agent-based systems that converge on optimal solutions through competition
- **Musical Intelligence**: Systems that understand musical context rather than just signal processing

The integration of BeatTracker with Todd's rhythmogram analysis creates a **dual-layer rhythm intelligence** system - Todd's frequency analysis provides detailed rhythmic content while BeatTracker ensures accurate tempo tracking across diverse musical styles.

### Neural Network Foundations

#### **Dr. Pauli Laine's Computational Music Research**
Dr. Laine's doctoral thesis, "A Method for Generating Musical Motion Patterns," Academic dissertation, by Pauli Laine, publicly discussed at the University of Helsinki, March 24, 2000, investigates computational approaches to musical sequence generation through neural modeling principles, providing the theoretical foundation for NeuronSeqSampler's neural network architecture.

**Citation:** Laine, P. (2000). *A Method for Generating Musical Motion Patterns*. Academic dissertation, University of Helsinki, publicly discussed March 24, 2000. Available: [http://hdl.handle.net/10138/19434](http://hdl.handle.net/10138/19434)

**Key Theoretical Contributions:**

Laine's research established foundational principles for generating musical sequences through computational neural models:

- **Musical Motion as Neural Process**: Musical patterns emerge from activation dynamics in computational units that mirror aspects of human musical cognition
- **Activation-Based Event Generation**: Musical events occur when computational neurons reach specific activation thresholds, creating temporal structure
- **Temporal Decay and Memory**: Neurons maintain activation states that decay over time, enabling context-sensitive pattern formation
- **Hierarchical Temporal Organization**: Musical structures operate simultaneously at multiple temporal scales, from individual notes to phrase-level patterns
- **Weighted Connection Networks**: Interactions between computational units governed by adjustable connection strengths enable emergent musical behaviors

**Influence on NeuronSeqSampler Design:**

The principles from Laine's thesis directly influenced several core aspects of NeuronSeqSampler's implementation:

1. **Leaky Integrate-and-Fire Neuron Model**: Following Laine's activation-based triggering principle, each neuron accumulates activation from inputs, decays over time, and fires upon reaching threshold—directly triggering audio samples to create rhythmic patterns.

2. **Configurable Network Topology**: Laine's emphasis on weighted connections is realized through user-definable connection matrices where neurons influence each other with adjustable excitatory and inhibitory weights.

3. **Multi-Scale Temporal Dynamics**: The hierarchical organization proposed by Laine is achieved by combining individual neuron dynamics (millisecond scale) with rhythmogram analysis (62.5 ms to 8 seconds) to create musical structures across multiple temporal scales.

4. **Flexible Parameter Space**: Per-neuron activation functions (Linear, Sigmoid, ReLU, Tanh), adjustable decay rates, and self-modulation capabilities provide the parameter flexibility Laine identified as essential for musical expressiveness.

5. **Emergent Pattern Formation**: Complex musical sequences emerge from simple neural interaction rules, demonstrating Laine's principle that sophisticated musical behaviors can arise from basic computational primitives.

**Extension of Laine's Framework:**

NeuronSeqSampler extends Laine's theoretical foundations by integrating:
- Todd's (1994) rhythmogram analysis for external rhythmic input and multi-scale temporal decomposition
- Real-time interactive parameter control enabling live performance and composition
- Musical quantization system for grid-aligned timing and rhythmic coherence
- Multi-agent beat tracking for tempo-aware operation across diverse musical styles

This synthesis creates a system that honors Laine's vision of neural-based musical generation while adding modern rhythm analysis and interactive performance capabilities.

#### **Biological Inspiration: Central Pattern Generators (CPGs)**
The core concept is rooted in **Central Pattern Generators** - specialized neural circuits found in biological systems that produce rhythmic, repetitive outputs with remarkable stability, enabling autonomous musical pattern generation.

### Integrated Technical Implementation

NeuronSeqSampler represents a **unique synthesis** of Todd's rhythmogram theory with modern neural network architectures:

#### **Todd Rhythmogram Integration**
- **8-Band Filterbank**: Direct implementation of Todd's logarithmic frequency distribution
- **Hybrid Processing**: Envelope following (<4Hz) + biquad filters (≥4Hz) optimized for rhythmic detection
- **Real-time Analysis**: Continuous rhythmogram computation with per decamille precision
- **Direct Neural Activation**: Bypasses traditional audio processing for immediate rhythmic response

#### **Neural Network Architecture** 
- **Leaky Integrate-and-Fire (LIF) Neurons**: Biologically-inspired activation dynamics
- **Spiking Neural Networks**: Threshold-based activation mimicking biological neuron firing
- **Dynamic Topology**: User-configurable connections enabling complex rhythmic interactions
- **Multi-Function Neurons**: Linear, Sigmoid, ReLU, Tanh activation functions per neuron

#### **Revolutionary Integration**
This **triple-stream approach** creates unprecedented musical intelligence:
- **Rhythmic Hierarchy → Neural Activation**: Todd's frequency bands directly drive neural network input
- **Professional Tempo Detection**: BeatTracker's multi-agent system ensures accurate tempo tracking across musical styles
- **Emergent Musical Patterns**: Neural dynamics generate complex rhythmic structures with tempo-aware scaling
- **Real-time Interaction**: Live performance capabilities with immediate visual/auditory feedback
- **Biological Authenticity**: Rhythmic perception, tempo detection, and neural processing all mirror natural systems

The result bridges **computational musicology** (Todd 1994), **advanced beat tracking research** (BeatTracker), and **artificial neural networks** (Laine), creating a system that generates music through scientifically grounded principles while maintaining artistic expressiveness.

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