# Audio Recorder

The Recorder class provides functionality for streaming audio to WAV files. It extends SFML's SoundRecorder to add file output capabilities and manual audio data processing.

## Features

- **Real-time recording**: Record from microphone to WAV files
- **Manual audio processing**: Add audio data programmatically
- **Thread-safe**: Safe to use from multiple threads
- **WAV file output**: Standard WAV format with proper headers
- **Configurable settings**: Sample rate and channel configuration

## Usage

### Basic Recording

```cpp
#include "Recorder.h"

Recorder recorder;

// Start recording (with automatic filename)
recorder.startRecording();

// Or specify a filename
recorder.startRecording("my_recording.wav");

// Stop recording (automatically saves to file)
recorder.stopRecording();
```

### Manual Audio Data

```cpp
Recorder recorder;

// Add audio samples manually
std::vector<sf::Int16> samples = { /* your audio data */ };
recorder.addSamples(samples);

// Save to file
recorder.saveToFile("manual_audio.wav");
```

### Integration with Neuron Sampler

The Recorder is integrated into the main application in two ways:

#### Keyboard Control:
- Press **R** to start recording
- Press **R** again to stop recording and save to file
- Files are automatically timestamped (e.g., `recording_20231218_143025.wav`)

#### GUI Menu Control:
- **Recording > Start Recording** - Opens a dialog to enter custom filename
- **Recording > Stop Recording** - Stops current recording and shows duration
- Default filenames include timestamps for easy organization
- Error handling for microphone availability and file permissions

## Key Methods

- `startRecording(filename)` - Begin recording to file
- `stopRecording()` - Stop recording and save
- `isCurrentlyRecording()` - Check recording status
- `addSamples(data)` - Add audio data manually
- `saveToFile(filename)` - Save buffer to WAV file
- `clearBuffer()` - Clear internal audio buffer
- `getRecordingDuration()` - Get recording length in seconds
- `getSampleCount()` - Get total number of samples

## File Format

The recorder outputs standard WAV files with:
- 16-bit PCM encoding
- Configurable sample rate (default: 44.1kHz)
- Configurable channels (default: mono)
- Proper WAV headers for compatibility

## Thread Safety

The Recorder class is thread-safe for concurrent access to audio data. Multiple threads can safely add samples while recording is in progress.

## Example

See `recorder_demo.cpp` for a complete example including:
- Microphone recording
- Programmatic audio generation
- File saving operations
