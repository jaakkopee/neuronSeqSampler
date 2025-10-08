# NeuronSeqSampler

A real-time neural network-based audio sampler that uses artificial neurons to trigger audio samples through threshold-based activation. Features an interactive GUI for network control, real-time audio recording, and dynamic parameter adjustment.

## Features

### Core System
- **Neural Network Engine**: Interconnected neurons with threshold-based activation
- **Real-Time Audio**: SFML-based audio playback with low-latency sample triggering
- **Live Recording**: Dual recording modes - microphone input and internal audio output
- **Interactive GUI**: TGUI-based control interface with real-time parameter adjustment
- **Sample Management**: Organized sample libraries with multiple categories
- **Connection Network**: Configurable neuron connections with adjustable weights

### Advanced Features
- **Activation Interval Control**: 1ms-1000ms range for ultra-precise timing control
- **Dynamic Network Modification**: Add/remove neurons and connections during runtime
- **Threshold Visualization**: Real-time visual feedback of neuron states
- **Audio Export**: Record internal neural network output to WAV files
- **Multiple Sample Categories**: 808s, claps, kicks, hats, FX, percussion, and more

## Quick Start

### Prerequisites
- **C++17 compatible compiler** (GCC 7+ or Clang 7+)
- **CMake 3.10** or higher
- **SFML 2.5+** (Graphics and Audio modules)
- **TGUI 0.9+** (Optional but recommended for GUI)
- **Linux audio system** (ALSA/PulseAudio)

### Installation

#### 1. Clone the Repository
```bash
git clone <repository-url>
cd neuronSeqSampler
```

#### 2. Install Dependencies

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

#### 3. Build the Project
```bash
# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Compile
make -j$(nproc)
```

#### 4. Set Up Sample Directory
```bash
# Return to project root
cd ..

# Ensure samples directory exists with audio files
# The application expects samples in subdirectories:
# samples/808/, samples/clap/, samples/kick/, etc.
```

#### 5. Run the Application
```bash
# Main application
./build/NeuronSeqSampler

# Or using the build script
./build.sh
```

## Sample Library Setup

### Directory Structure
The application expects samples organized in the following structure:
```
samples/
├── 808/          # 808 drum samples
├── clap/         # Clap samples  
├── kick/         # Kick drum samples
├── hat/          # Hi-hat samples
├── snare/        # Snare samples
├── fx/           # Sound effects
├── perc/         # Percussion samples
├── stabs/        # Stab/chord samples
├── girliepop/    # Genre-specific samples
└── loops & samples/ # Loops and longer samples
```

### Supported Audio Formats
- **WAV files** (16-bit, 24-bit)
- **Sample rates**: 44.1kHz, 48kHz, 96kHz
- **Channels**: Mono and stereo supported

### Sample Requirements
- Place audio files (.wav) in appropriate subdirectories
- Files are automatically detected and loaded at startup
- No specific naming convention required
- Maximum recommended: 50MB per directory for optimal performance

## GUI Controls and Usage

### Main Interface Layout

#### Left Panel: Neural Network Visualization
- **Real-time neuron display**: Shows current activation levels
- **Connection visualization**: Lines between connected neurons
- **Threshold indicators**: Visual feedback of firing states
- **Interactive clicking**: Manual neuron triggering via mouse

#### Right Panel: Control Interface
- **Status display**: Network information and current state
- **Activation interval slider**: Timing control (1ms-1000ms)
- **Connection weight sliders**: Adjust neural network connections
- **Scrollable controls**: Accommodates multiple connections

#### Top Menu Bar
- **Network Menu**:
  - Add Neuron
  - Remove Neuron  
  - Add Connection
  - Remove Connection
  - Reset Network
- **Recording Menu**:
  - Start External Recording (Microphone)
  - Start Internal Recording (Network Output)
  - Stop Recording

### Detailed Controls

#### Activation Interval Slider
- **Location**: Right panel, below status display
- **Label**: "Update Rate: XXXms"
- **Range**: 1ms (ultra-fast) to 1000ms (slow)
- **Effect**: Controls how frequently the neural network updates
- **Real-time**: Changes take effect immediately

**Recommended Settings:**
- **Live Performance**: 50-100ms for responsiveness
- **Composition**: 100-200ms for musical timing  
- **Ambient/Experimental**: 300-500ms for slow evolution
- **Real-time Synthesis**: 1-10ms (high CPU usage)

#### Connection Weight Sliders
- **Auto-generated**: One slider per neural connection
- **Range**: 0.0 to 1.0
- **Effect**: Determines connection strength between neurons
- **Live adjustment**: Immediate effect on network behavior

#### Manual Triggering
- **Spacebar**: Triggers random neuron activation
- **Mouse clicks**: Click neurons in visualization to manually activate
- **Combined control**: Manual triggers work alongside automatic network

### Recording Features

#### External Recording (Microphone)
1. Select "Recording → Start External Recording" from menu
2. Enter filename (auto-generates timestamp if empty)
3. Click "Start Recording"
4. Record your audio input
5. Click "Recording → Stop Recording" to finish
6. File saved in project directory

#### Internal Recording (Network Output)
1. Select "Recording → Start Internal Recording" from menu
2. Enter filename for output
3. Click "Start Recording"
4. Let the neural network play and record its output
5. Stop recording when finished
6. Pure neural network audio saved to file

#### Recording Tips
- **File format**: All recordings saved as WAV files
- **Quality**: 44.1kHz, 16-bit by default
- **Location**: Files saved in project root directory
- **Naming**: Automatic timestamp naming available
- **Duration**: No built-in time limits

### Network Configuration

#### Adding Neurons
1. Menu: "Network → Add Neuron"
2. Select sample directory from dropdown
3. Choose specific audio file
4. Set initial activation threshold
5. Neuron appears in visualization

#### Creating Connections  
1. Menu: "Network → Add Connection"
2. Select source neuron (input)
3. Select target neuron (output)
4. Set connection weight (0.0-1.0)
5. New slider appears in control panel

#### Network Management
- **Reset Network**: Returns all neurons to default state
- **Remove Elements**: Delete specific neurons or connections
- **Live Editing**: All changes take effect immediately
- **State Preservation**: Network continues running during modifications

## Test Code and Examples

### Running Test Applications

#### Audio System Test
```bash
# Test SFML audio output
./build/test_sfml_audio

# This verifies:
# - Audio device detection
# - Sample playback capability
# - Basic SFML audio functionality
```

#### Recording System Test
```bash
# Test audio recording functionality  
./build/recorder_demo

# This demonstrates:
# - Microphone input capture
# - Real-time audio processing
# - WAV file output generation
# - Manual sample injection
```

#### Debug Recorder
```bash
# Advanced recording diagnostics
./build/debug_recorder

# Features:
# - Detailed audio buffer analysis
# - Recording latency measurement
# - Audio device information
# - Error diagnostics
```

### Python Demo Scripts

#### Basic Neural Network Demo
```bash
# Simple neural network demonstration
python3 nSGUIDemo.py

# Features:
# - Basic neuron interaction
# - Simple GUI controls  
# - Educational network visualization
# - Manual activation controls
```

#### Advanced Sampling Demo
```bash
# Full sampler demonstration
python3 nSSampler.py

# Includes:
# - Sample loading and playback
# - Neural network integration
# - Advanced parameter control
# - Real-time audio processing
```

### Manual Testing Procedures

#### 1. Basic Functionality Test
```bash
# Start the main application
./build/NeuronSeqSampler

# Test checklist:
# ✓ GUI loads without errors
# ✓ Sample directories detected
# ✓ Neural network visualization active
# ✓ Audio output working
# ✓ Controls respond to input
```

#### 2. Neural Network Test
```bash
# In the running application:
# 1. Press spacebar - should trigger random neuron
# 2. Adjust activation interval slider - watch timing change
# 3. Add new neuron via menu - verify it appears
# 4. Create connection - verify slider appears
# 5. Adjust connection weight - observe network behavior change
```

#### 3. Recording Test
```bash
# External recording test:
# 1. Start external recording
# 2. Make sound into microphone
# 3. Stop recording
# 4. Verify WAV file created
# 5. Play back file to confirm audio captured

# Internal recording test:
# 1. Start internal recording
# 2. Let neural network play for 10-30 seconds
# 3. Stop recording
# 4. Verify network output captured in file
```

## Audio Recording (Legacy JACK Configuration)

The application supports dual audio recording modes with JACK audio server compatibility:

### Recording Controls
- **Keyboard**: Press `R` to start/stop recording
- **GUI**: Use the "Recording" menu to start/stop with custom filenames

### Noise Reduction Features
The recorder includes built-in gentle noise reduction:
- **Noise Gate**: Silences audio below 0.1% threshold to eliminate only background hiss
- **High-Pass Filter**: Removes low-frequency electrical hum (60Hz cutoff)
- **Optimized Microphone Settings**: Automatic configuration for clean recording

### Setup for Best Quality

1. **Stop JACK** for recording:
   ```bash
   ./manage_jack.sh stop
   ```

2. **Optimize microphone settings**:
   ```bash
   ./optimize_mic.sh
   ```

3. **Record your audio** using the application

4. **Restart JACK** for sample playback:
   ```bash
   ./manage_jack.sh start
   ```

### IMPORTANT: Audio System Configuration

If recordings contain no audio (silent files), this is usually caused by JACK audio server blocking microphone access. 

**Solution:**
1. Stop JACK before recording:
   ```bash
   ./manage_jack.sh stop
   ```
2. Record your audio
3. Restart JACK when needed:
   ```bash
   ./manage_jack.sh start
   ```

### Troubleshooting Recording Issues

**Static/hissing noise in recordings:**
- Run `./optimize_mic.sh` to set optimal microphone levels
- The built-in noise gate and high-pass filter should eliminate most static
- If noise persists, try reducing microphone boost further:
  ```bash
  amixer -c 0 sset "Headphone Mic Boost" 1   # Minimum boost
  ```

**No audio in recordings:**
- Make sure JACK is stopped (see above)
- Check microphone settings:
  ```bash
  amixer -c 0 sget "Input Source"
  amixer -c 0 sset "Headphone Mic Boost" 100%
  ```
- Verify microphone input source is correct:
  ```bash
  amixer -c 0 sset "Input Source" "Headphone Mic"
  ```

## Advanced Configuration

### Performance Tuning

#### CPU Optimization
- **Activation Interval**: Increase to 100-200ms for lower CPU usage
- **Connection Count**: Fewer connections = better performance
- **Sample Quality**: Use 44.1kHz samples for best balance
- **TGUI**: Optional dependency - can run without GUI for headless operation

#### Audio Latency
- **Buffer Size**: Modify in AudioManager.cpp if needed
- **Sample Rate**: Match your audio interface capabilities
- **Driver**: Use ALSA for lowest latency on Linux
- **Real-time Priority**: Run with real-time scheduling for best performance

#### Memory Usage
- **Sample Loading**: Samples loaded into RAM for instant playback
- **Large Files**: Keep samples under 10MB each for best performance
- **Directory Limits**: Maximum ~100 samples per directory recommended
- **Format Optimization**: Use 16-bit WAV files to reduce memory usage

### Build Configuration

#### CMake Options
```bash
# Build without TGUI (headless mode)
cmake -DUSE_TGUI=OFF ..

# Debug build with verbose output
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..

# Custom SFML path
cmake -DSFML_DIR=/path/to/sfml ..
```

#### Compiler Flags
```bash
# For maximum performance
export CXXFLAGS="-O3 -march=native -DNDEBUG"

# For debugging
export CXXFLAGS="-g -O0 -DDEBUG"

# For profiling
export CXXFLAGS="-pg -O2"
```

### Audio Device Configuration

#### ALSA Setup
```bash
# List audio devices
aplay -l

# Test audio output
aplay /usr/share/sounds/alsa/Front_Left.wav

# Configure default device in ~/.asoundrc if needed
```

#### PulseAudio Setup
```bash
# Check audio devices
pactl list sinks

# Test output
pactl play-sample audio-test-left

# Adjust latency in /etc/pulse/daemon.conf if needed
```

## Troubleshooting

### Common Issues

#### Build Problems
```bash
# SFML not found
sudo apt install libsfml-dev
# or check cmake output for specific missing components

# TGUI not found (optional)
cmake -DUSE_TGUI=OFF ..
# Build without GUI components

# C++17 errors
# Ensure GCC 7+ or Clang 7+
g++ --version
```

#### Audio Issues
```bash
# No audio output
# Check ALSA/PulseAudio setup
aplay -l
pulseaudio --check

# Samples not loading
# Verify samples/ directory structure and file permissions
ls -la samples/*/

# Recording fails
# Check microphone permissions and device availability
arecord -l
```

#### Performance Issues
```bash
# High CPU usage
# Increase activation interval in GUI (100ms+)
# Reduce number of neural connections
# Check system load: htop

# Audio dropouts
# Increase audio buffer size
# Close other audio applications
# Use real-time kernel if available
```

#### GUI Problems
```bash
# GUI doesn't load
# Check TGUI installation
pkg-config --exists tgui

# Controls unresponsive
# Ensure proper window focus
# Check for competing applications using audio

# Visualization issues
# Verify graphics drivers
# Check SFML graphics functionality
```

### Debug Mode

#### Enable Detailed Logging
```bash
# Build in debug mode
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run with verbose output
./build/NeuronSeqSampler --verbose

# Check console output for detailed system information
```

#### Performance Profiling
```bash
# Build with profiling
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make

# Profile with gprof
gprof ./build/NeuronSeqSampler gmon.out > profile.txt

# Or use system monitor
htop
iotop
```

## Project Structure

### Core Components
- **`src/main.cpp`**: Application entry point and main loop
- **`src/NeuronNetwork.cpp`**: Neural network logic and management
- **`src/Neuron.cpp`**: Individual neuron behavior and threshold logic
- **`src/AudioManager.cpp`**: Audio playback and sample management
- **`src/GUI.cpp`**: User interface and control logic
- **`src/Recorder.cpp`**: Audio recording and file output
- **`src/Visualizer.cpp`**: Real-time network visualization

### Configuration Files
- **`CMakeLists.txt`**: Build system configuration
- **`build.sh`**: Automated build script
- **`manage_jack.sh`**: JACK audio server management
- **`optimize_mic.sh`**: Microphone optimization script
- **`test_audio.sh`**: Audio system testing script

### Documentation
- **`ACTIVATION_INTERVAL_SLIDER.md`**: Detailed slider feature documentation
- **`README_RECORDER.md`**: Recording system documentation
- **`SAMPLE_RATE_FIX.md`**: Audio rate configuration
- **`TEMPORAL_AUGMENTATION_FIX.md`**: Timing accuracy documentation

## Contributing

### Development Setup
1. Fork the repository
2. Create feature branch: `git checkout -b feature-name`
3. Make changes and test thoroughly
4. Run all test applications
5. Submit pull request with detailed description

### Code Style
- **C++17 standard** compliance required
- **RAII principles** for resource management
- **Const-correctness** where applicable
- **Clear variable naming** and documentation
- **Error handling** with appropriate exceptions

### Testing
- Test all sample loading scenarios
- Verify recording functionality on different systems
- Check performance with various network configurations
- Validate GUI responsiveness across different platforms

## License

Free copylefted. Use and modify as you wish.

## Acknowledgments

- **SFML**: Cross-platform multimedia library
- **TGUI**: Modern C++ GUI library  
- **Neural Network Research**: Inspiration from biological neural networks
- **Audio Processing**: Community contributions to real-time audio

---

*For technical questions or feature requests, please open an issue on the project repository.*
- Enable and set volume:
  ```bash
  amixer sset Master 80%
  amixer -c 0 sset Speaker 80% unmute
  ```
- Run complete audio optimization:
  ```bash
  ./optimize_mic.sh
  ```

**Error messages:**
- "Audio recording is not available": No microphone detected
- "Dummy/Monitor device detected": JACK is blocking access - stop JACK first
