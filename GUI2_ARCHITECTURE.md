# GUI2 System Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                         GUI2 Main Window                            │
│  ┌───────────────┐  ┌──────────────────────────────────────────┐   │
│  │ Module List   │  │         Module Canvas                     │   │
│  │ Panel         │  │                                           │   │
│  ├───────────────┤  │  ┌──────────┐         ┌──────────┐       │   │
│  │ AudioManager  │  │  │AudioMgr  │────────→│Rhythmogram       │   │
│  │ Recorder      │  │  │ ●○       │         │ ●○       │       │   │
│  │ NeuronNetwork │  │  └──────────┘         └──────────┘       │   │
│  │ BeatTracker   │  │       │                    │              │   │
│  │ Rhythmogram   │  │       │                    ↓              │   │
│  │ Quantizer     │  │       │              ┌──────────┐         │   │
│  └───────────────┘  │       │              │BeatTracker        │   │
│                     │       │              │ ●○       │         │   │
│                     │       │              └──────────┘         │   │
│                     │       │                    │              │   │
│                     │       ↓                    ↓              │   │
│                     │  ┌─────────────────────────┐             │   │
│                     │  │   NeuronNetwork         │             │   │
│                     │  │   ●●●○○                 │             │   │
│                     │  └─────────────────────────┘             │   │
│                     └──────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘

Legend:
● = Input port (green circles)
○ = Output port (red circles)
→ = Connection (blue lines)
```

## Component Hierarchy

```
ModularComponent (abstract base)
├── AudioManagerModule
│   ├── wraps: AudioManager
│   ├── inputs: Audio In, Trigger
│   ├── outputs: Audio Out, Mix Out
│   └── params: Master Volume, Sample Rate
│
├── RecorderModule
│   ├── wraps: Recorder
│   ├── inputs: Audio In, Record Trigger
│   ├── outputs: Audio Out, Recording Status
│   └── params: Sample Rate, Noise Gate, High Pass
│
├── NeuronNetworkModule
│   ├── wraps: NeuronNetwork
│   ├── inputs: Rhythm In, Control In, Beat Phase In
│   ├── outputs: Neuron Out, Trigger Out
│   └── params: Learning Rate, Weight Decay, etc.
│
├── BeatTrackerModule
│   ├── wraps: BeatTracker
│   ├── inputs: Onset In, Audio In
│   ├── outputs: Beat Phase, Tempo, Beat Trigger
│   └── params: Min/Max Tempo, Agents, Adaptation
│
├── RhythmogramModule
│   ├── wraps: RhythmInterpreter
│   ├── inputs: Audio In
│   ├── outputs: Rhythm Out, Onset Out, Band 0-7
│   └── params: Sample Rate, Buffer Size, Threshold
│
└── QuantizerModule
    ├── wraps: Quantizer
    ├── inputs: Event In, Beat Phase In
    ├── outputs: Quantized Out, Timing Out
    └── params: BPM, Grid, Strength, Swing
```

## Data Flow Example

```
Input Audio (Recorder/AudioManager)
    │
    ↓
Rhythmogram (Todd Frequency Analysis)
    │
    ├──→ Rhythm Data ──→ NeuronNetwork
    │                         ↑
    └──→ Onset Data ──→ BeatTracker
                              │
                              ├──→ Beat Phase ──→ NeuronNetwork
                              │
                              └──→ Tempo ──→ Quantizer
                                               │
                                               └──→ Quantized Events ──→ NeuronNetwork
                                                                            │
                                                                            ↓
                                                                      Audio Output
```

## GUI2 Class Structure

```
GUI2
├── window: sf::RenderWindow*
├── modules: vector<unique_ptr<ModularComponent>>
├── connections: vector<VisualConnection>
│
├── Methods:
│   ├── update(deltaTime)
│   ├── render()
│   │   ├── renderCanvas()
│   │   ├── renderGrid()
│   │   ├── renderModules()
│   │   ├── renderConnections()
│   │   ├── renderModuleList()
│   │   └── renderParameterWindow()
│   │
│   ├── handleMousePress(event)
│   ├── handleMouseRelease(event)
│   ├── handleMouseMove(event)
│   ├── handleMouseWheel(event)
│   ├── handleKeyPress(event)
│   │
│   ├── addModule(type, position)
│   ├── removeModule(module)
│   ├── createConnection(src, srcPort, dst, dstPort)
│   ├── removeConnection(connection)
│   │
│   └── Utility:
│       ├── screenToCanvas(pos)
│       ├── canvasToScreen(pos)
│       ├── openParameterWindow(module)
│       └── closeParameterWindow()
│
└── State:
    ├── currentMode: InteractionMode
    ├── selectedModule: ModularComponent*
    ├── draggedModule: ModularComponent*
    ├── moduleListVisible: bool
    └── parameterWindowVisible: bool
```

## Interaction State Machine

```
           ┌────────┐
           │  IDLE  │
           └────┬───┘
                │
     ┌──────────┼──────────┐
     │          │          │
     ↓          ↓          ↓
┌─────────┐ ┌─────┐ ┌──────────┐
│DRAGGING │ │PAN  │ │DRAGGING  │
│MODULE   │ │     │ │CONNECTION│
└─────────┘ └─────┘ └──────────┘
     │          │          │
     └──────────┼──────────┘
                ↓
           ┌────────┐
           │  IDLE  │
           └────────┘
```

## Port Type Compatibility Matrix

```
           │ AUDIO │CONTROL│RHYTHM │ONSET  │NEURON │
───────────┼───────┼───────┼───────┼───────┼───────┤
AUDIO_OUT  │   ✓   │   ✗   │   ✗   │   ✗   │   ✗   │
CONTROL_OUT│   ✗   │   ✓   │   ✗   │   ✗   │   ✗   │
RHYTHM_OUT │   ✗   │   ✗   │   ✓   │   ✗   │   ✗   │
ONSET_OUT  │   ✗   │   ✗   │   ✗   │   ✓   │   ✗   │
NEURON_OUT │   ✗   │   ✗   │   ✗   │   ✗   │   ✓   │
───────────┴───────┴───────┴───────┴───────┴───────┘
```

## File Organization

```
neuronSeqSampler/
│
├── src/
│   ├── ModularComponent.h          (Base class - 250 lines)
│   ├── GUI2.h                      (GUI interface - 230 lines)
│   ├── GUI2.cpp                    (GUI implementation - 620 lines)
│   │
│   ├── AudioManagerModule.h        (60 lines)
│   ├── RecorderModule.h            (80 lines)
│   ├── NeuronNetworkModule.h       (90 lines)
│   ├── BeatTrackerModule.h         (85 lines)
│   ├── RhythmogramModule.h         (70 lines)
│   └── QuantizerModule.h           (90 lines)
│
├── GUI2Demo.cpp                    (100 lines)
├── CMakeLists_GUI2.txt             (60 lines)
├── build_gui2.sh                   (40 lines)
│
└── Documentation/
    ├── GUI2_DOCUMENTATION.md        (Comprehensive guide)
    ├── GUI2_QUICK_REFERENCE.md      (Quick reference)
    ├── GUI2_IMPLEMENTATION_SUMMARY.md (This summary)
    └── GUI2_ARCHITECTURE.md         (Architecture diagrams)
```

## Rendering Pipeline

```
Main Loop
    │
    ├──→ Update (60 FPS)
    │    ├── Process all modules
    │    └── Update deltaTime
    │
    └──→ Render
         ├── Clear window
         ├── Render canvas background
         ├── Render grid (if enabled)
         ├── Render connections
         │   └── For each connection:
         │       ├── Get source port position
         │       ├── Get dest port position
         │       └── Draw line + arrow
         ├── Render modules
         │   └── For each module:
         │       ├── Draw module rectangle
         │       ├── Draw module name
         │       ├── Draw input ports (green)
         │       └── Draw output ports (red)
         ├── Render module list panel (if visible)
         ├── Render parameter window (if open)
         └── Render active connection drag
```

## Memory Management

```
GUI2 owns:
├── modules: vector<unique_ptr<ModularComponent>>
│   └── Each module owns:
│       └── unique_ptr to wrapped component
│           (AudioManager, Recorder, etc.)
│
├── connections: vector<VisualConnection>
│   └── Each connection stores:
│       ├── Raw pointers to modules (not owning)
│       └── Port indices
│
└── UI state:
    └── Raw pointers to selected modules (not owning)
```

## Build Process

```
build_gui2.sh
    │
    ├──→ Check SFML installation
    ├──→ Create build_gui2/ directory
    ├──→ Run CMake configuration
    │    ├── Find SFML packages
    │    ├── Set C++17 standard
    │    └── Configure build
    ├──→ Build with cmake --build
    │    ├── Compile all sources
    │    └── Link SFML libraries
    └──→ Create gui2_demo executable
```

---

**System Status**: ✅ Complete and ready for testing!
