# Sample File Path Implementation

## Overview

The Neuron class now stores relative file paths to sample files, making presets portable and shareable across different systems and installations.

## Key Features

### ✅ Relative Path Storage
- Sample paths are stored relative to the application's working directory
- Example: `"samples/kick/kick.wav"` instead of `"/absolute/path/to/samples/kick/kick.wav"`
- Maintains consistency with existing sample loading system

### ✅ Automatic Path Assignment
- When adding neurons through the GUI, sample file paths are automatically assigned
- The GUI constructs relative paths like: `"samples/" + category + "/" + filename`
- No manual path management required for typical usage

### ✅ Preset Portability
- Presets can be shared between users without path issues
- Moving the application folder preserves sample references
- Works across different operating systems (Windows, Linux, macOS)

## Technical Implementation

### Neuron Class Changes
```cpp
class Neuron {
private:
    std::string sampleFilePath;  // New member for storing relative path
    
public:
    // Updated constructor with sample file path parameter
    Neuron(int sampleIndex, float initialActivation = 0.0f,
           float threshold = 1.0f, float decayRate = 1.0f,
           float activationIncrease = 0.0f,
           ActivationFunction func = ActivationFunction::Linear,
           const std::string& sampleFilePath = "");
           
    // Getter and setter methods
    const std::string& getSampleFilePath() const;
    void setSampleFilePath(const std::string& filePath);
};
```

### JSON Preset Format
Neurons in preset files now include a `sample_file_path` field:
```json
{
  "neurons": [
    {
      "id": 0,
      "sample_index": 1,
      "sample_file_path": "samples/kick/kick.wav",
      "activation": 0.0,
      "threshold": 1.0,
      "decay_rate": 1.0,
      "activation_increase_per_iteration": 0.0,
      "activation_function": "Linear"
    }
  ]
}
```

## Usage Examples

### Creating Neurons with Sample Paths
```cpp
// Through GUI - automatic path assignment
// User selects: category="kick", file="kick.wav"
// Result: neuron stores "samples/kick/kick.wav"

// Programmatically
auto neuron = network.addNeuron(1, 0.0f, 1.0f, 1.0f, 0.0f, 
                               ActivationFunction::Linear, 
                               "samples/kick/kick.wav");
```

### Modifying Sample Paths
```cpp
// Change the sample associated with a neuron
neuron->setSampleFilePath("samples/snare/snare.wav");

// Get current sample path
std::string currentPath = neuron->getSampleFilePath();
```

## Directory Structure
The application expects this relative directory structure:
```
neuronSeqSampler/
├── src/
├── presets/
└── samples/
    ├── kick/
    │   ├── kick.wav
    │   └── kick (ghost).wav
    ├── snare/
    │   └── snare.wav
    ├── clap/
    │   └── clap (ghost).wav
    └── 808/
        └── ROBBERY 808 @prodopus.wav
```

## Benefits for Users

### 🔄 **Sharing Presets**
- Send preset files to other users without path dependencies
- Presets work immediately if recipient has the same sample files

### 📁 **Portable Projects**  
- Move entire project folder to different locations
- Works on different computers/operating systems
- No need to update file paths manually

### 🎵 **Sample Management**
- Clear organization with relative paths
- Easy to see which samples a preset uses
- Sample paths preserved across sessions

## Migration Notes

### Backward Compatibility
- Existing neurons without sample paths continue to work
- Empty sample paths are handled gracefully
- Old presets load normally (neurons get empty sample paths)

### Upgrading Existing Presets
- Save existing networks to update preset format
- New saves will include sample_file_path fields
- No data loss during the upgrade process

## Future Enhancements

### Potential Features
- **Sample Path Validation**: Check if referenced files exist when loading presets
- **Missing Sample Handling**: GUI warnings for missing sample files
- **Batch Path Updates**: Tools to update sample paths in multiple presets
- **Sample Pack Integration**: Automatic path assignment for sample pack imports