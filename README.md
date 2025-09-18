NeuronSeqSampler is a sampler controlled by a neural network.
To start exploring, create a couple of neurons and
connect them via the menu.

## Audio Recording

The application supports audio recording through both keyboard shortcuts and GUI controls:

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

**No sound from sample playback:**
- Check speaker/audio output settings:
  ```bash
  amixer sget Master
  amixer -c 0 sget Speaker
  ```
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
