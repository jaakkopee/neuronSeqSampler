# Internal Recording Feature

## Overview
The NeuronSeqSampler now supports two types of recording:

1. **Internal Recording (R key)** - Records the audio output from NeuronSeqSampler itself
2. **External Recording (Shift+R)** - Records from system microphone (requires JACK to be stopped)

## Internal Recording Usage

### Quick Start
1. Start the NeuronSeqSampler: `./build/NeuronSeqSampler`
2. Create some neurons using the "Network" menu
3. Press **R key** to start internal recording
4. Trigger neurons by clicking them or using number keys
5. Use "Recording" → "Stop Recording" menu to end the recording

### Detailed Steps

#### 1. Setup Neural Network
- Go to "Network" menu → "Add Neuron" to create neurons
- Optionally add connections between neurons using "Network" → "Add Connection"
- Each neuron will play a different sample from the `samples/girliepop/` directory

#### 2. Start Internal Recording
**Method 1: Keyboard Shortcut**
- Press **R** key to open the internal recording dialog
- Confirm filename (or use the auto-generated one)
- Click "Start Recording"

**Method 2: Menu**
- Go to "Recording" menu → "Record NeuronSeq Output"
- Confirm filename and click "Start Recording"

#### 3. Generate Audio to Record
- Click on neurons with the mouse to trigger them
- Use number keys (1-6) to trigger specific neurons
- Press spacebar for manual network activation
- The recording captures all audio output from triggered neurons

#### 4. Stop Recording
- Go to "Recording" menu → "Stop Recording"
- The WAV file will be saved in the current directory

### Recording File Format
- **Format**: WAV file, 16-bit, 44.1kHz, stereo
- **Filename**: Auto-generated with timestamp (e.g., `neuronseq_output_20250101_120000.wav`)
- **Content**: Raw, unfiltered audio output from the NeuronSeqSampler
- **Processing**: No noise reduction, no lowpass filter, no highpass filter applied

## Key Differences

### Internal Recording (R key)
- ✅ Records clean audio output from NeuronSeqSampler
- ✅ No background noise or microphone artifacts
- ✅ Works while JACK is running
- ✅ Perfect for capturing composed sequences
- ✅ Raw, unfiltered audio - no noise reduction or filters applied
- ❌ Only captures NeuronSeqSampler output (no external audio)

### External Recording (Shift+R)
- ✅ Records everything the microphone picks up
- ✅ Can capture external instruments or vocals along with NeuronSeqSampler
- ❌ Requires JACK to be stopped (`./manage_jack.sh stop`)
- ❌ May include background noise and microphone artifacts
- ❌ Requires microphone setup and optimization

## Troubleshooting

### Internal Recording Issues
- **Problem**: No audio in recording file
  - **Solution**: Make sure to trigger neurons after starting recording
  - **Check**: Verify neurons are loaded with samples and producing sound

- **Problem**: Recording doesn't start
  - **Solution**: Check that no other recording is already active

### External Recording Issues
- **Problem**: External recording fails to start
  - **Solution**: Run `./manage_jack.sh stop` before recording
  - **Solution**: Run `./optimize_mic.sh` to configure microphone
  - **Check**: Ensure microphone is connected and recognized by system

## File Locations
- **Recordings**: Saved in the main project directory
- **Samples**: Located in `samples/girliepop/` (default)
- **Build**: Application binary at `./build/NeuronSeqSampler`

## Example Workflow

1. **Setup**:
   ```bash
   cd /path/to/neuronSeqSampler
   ./build/NeuronSeqSampler
   ```

2. **Create Network**:
   - Network → Add Neuron (repeat 3-4 times)
   - Network → Add Connection (connect neurons)

3. **Record Sequence**:
   - Press **R** → Start Recording
   - Click neurons or use number keys to create sequence
   - Recording → Stop Recording

4. **Result**:
   - Clean WAV file with your neural sequence
   - No background noise or microphone artifacts
   - Ready for further editing or sharing

This internal recording feature is perfect for capturing clean compositions and sequences created with the NeuronSeqSampler!
