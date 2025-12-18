# JSON Preset System - NeuronSeqSampler

## Overview

The JSON preset system allows you to save and load complete neural network configurations including:
- **Neurons**: All parameters (activation, threshold, decay, activation function, etc.)
- **Connections**: Network topology and connection weights
- **Rhythmogram Matrix**: Filter configurations and routing
- **Metadata**: Preset information (name, author, description, tags)

## Features

### ✅ **Complete Implementation**
- **JSON Format**: Human-readable, editable preset files
- **Keyboard Shortcuts**: Quick save (S) and load (L) operations  
- **GUI Integration**: Full menu system with dialogs
- **Factory Presets**: Built-in examples ready to use
- **Metadata Support**: Rich preset information and tagging
- **Validation**: Error handling and JSON structure validation

### 🎹 **Keyboard Controls**
- **S Key**: Quick save current network (timestamp-based filename)
- **L Key**: Load factory drum pattern preset
- **GUI Menu**: Full preset management via "Presets" menu

### 🖥️ **GUI Features**
- **Save Dialog**: Name and describe your presets
- **Load Dialog**: Browse and select from factory/user presets  
- **Preset Browser**: Detailed view of all available presets with metadata
- **Quick Load**: Direct access to factory drum pattern

## File Structure

```
presets/
├── factory/                    # Built-in presets
│   ├── drum_pattern.json      # 3-neuron drum pattern
│   └── ambient_textures.json  # 2-neuron ambient preset
└── user/                      # User-created presets
    └── [your_presets].json
```

## JSON Format Specification

### Complete Preset Structure
```json
{
  "preset_info": {
    "name": "Preset Name",
    "version": "1.0",
    "author": "Creator Name", 
    "description": "Detailed description",
    "created_date": "2025-10-14T08:00:00Z",
    "tags": "comma,separated,tags"
  },
  "neurons": [
    {
      "id": 0,
      "sample_index": 1,
      "activation": 0.0,
      "threshold": 1.0,
      "decay_rate": 0.5,
      "activation_increase_per_iteration": 0.0,
      "activation_function": "Linear"
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
    "filter_gains": [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]
  }
}
```

### Field Descriptions

#### **Preset Info**
- `name`: Display name for the preset
- `version`: Version number (semantic versioning recommended)
- `author`: Creator identification
- `description`: Detailed explanation of the preset's purpose/sound
- `created_date`: ISO 8601 timestamp
- `tags`: Comma-separated categories for organization

#### **Neurons**
- `id`: Unique identifier (0-based index)
- `sample_index`: Which audio sample to play (1-based)
- `activation`: Current activation level (0.0-1.0+)
- `threshold`: Activation trigger point (typically 1.0)
- `decay_rate`: How quickly activation fades (0.0-1.0)
- `activation_increase_per_iteration`: Self-modulation rate (-0.1 to 0.6)
- `activation_function`: "Linear", "Sigmoid", "ReLU", or "Tanh"

#### **Connections**
- `source_id`: Source neuron ID
- `target_id`: Target neuron ID  
- `weight`: Connection strength (-1.2 to 1.2, positive/negative)

#### **Rhythmogram Matrix**
- `enabled`: Whether rhythmogram analysis is active
- `scale`: Overall rhythmogram scaling factor (0.0-20.0)
- `filter_gains`: Array of 8 filter sensitivities (0.0-5.0 each)

## Usage Examples

### 1. **Quick Save During Session**
```
Press 'S' → Saves as quicksave_[timestamp].json in presets/user/
```

### 2. **Load Factory Pattern**
```
Press 'L' → Loads 3-neuron drum pattern with kick/clap/bass
```

### 3. **GUI Save with Metadata**
```
Menu → Presets → Save Preset
→ Enter name and description
→ Saves to presets/user/[name].json
```

### 4. **GUI Browse and Load**
```
Menu → Presets → Load Preset
→ Select from factory or user presets
→ Preview metadata before loading
```

### 5. **Create Custom Preset**
```json
{
  "preset_info": {
    "name": "My Rhythm",
    "description": "Custom 4/4 pattern"
  },
  "neurons": [
    {
      "id": 0,
      "sample_index": 1,
      "activation_function": "ReLU",
      "threshold": 0.8
    }
  ],
  "connections": [],
  "rhythmogram_matrix": {
    "enabled": false
  }
}
```

## Technical Implementation

### **C++ Classes**
- `PresetManager`: Core save/load functionality
- `GUI`: Menu integration and dialogs
- Integrated with `NeuronNetwork`, `Neuron`, `Connection` classes

### **Dependencies**
- `nlohmann/json`: JSON parsing and generation
- `TGUI`: GUI dialogs and controls  
- `std::filesystem`: Directory management

### **Error Handling**
- JSON validation on load
- Directory creation if missing
- Graceful failure with user feedback
- Console logging for debugging

## Advanced Features

### **Preset Validation**
```cpp
bool PresetManager::validatePreset(const std::string& filename);
```

### **Batch Operations**
```cpp
std::vector<std::string> PresetManager::getAvailablePresets(const std::string& directory);
std::vector<PresetInfo> PresetManager::getPresetInfos(const std::string& directory);
```

### **Factory Preset Creation**
```cpp
bool PresetManager::createFactoryPresets(); // Auto-generates examples
bool PresetManager::loadFactoryPreset(NeuronNetwork& network, const std::string& name);
```

## Future Enhancements

### **Planned Features**
- **Preset Categories**: Organize by genre/style
- **Preset Blending**: Morph between two presets
- **Auto-backup**: Save session state automatically
- **Import/Export**: Share presets between users
- **Version Migration**: Handle format changes gracefully
- **Partial Presets**: Save only specific components (neurons, connections, etc.)

### **Rhythmogram Integration**
Currently basic matrix settings are saved. Future versions will include:
- Full connection matrix state (8×N routing)
- Per-connection gain settings  
- Toggle states for each frequency→neuron route
- Tempo detection settings (autodetect state, BPM)

## Troubleshooting

### **Common Issues**
1. **"Failed to save preset"**: Check write permissions in presets/ directory
2. **"Failed to load preset"**: Verify JSON syntax with `jq . preset.json`
3. **"Invalid preset format"**: Ensure required fields (neurons, connections) exist
4. **GUI not showing presets**: Rebuild with TGUI support enabled

### **Validation Commands**
```bash
# Check JSON syntax
jq . presets/factory/drum_pattern.json

# Run preset system tests  
./test_json_presets.sh

# List all presets
find presets/ -name "*.json" -exec basename {} .json \;
```

## Integration Status

### ✅ **Completed**
- [x] JSON serialization/deserialization
- [x] Neuron parameter saving/loading
- [x] Connection topology preservation
- [x] Metadata system with timestamps
- [x] Keyboard shortcuts (S/L keys)
- [x] GUI menu integration  
- [x] Factory preset creation
- [x] Directory management
- [x] Error handling and validation
- [x] User and factory preset separation

### 🔄 **Future Work**
- [ ] Full rhythmogram matrix state preservation
- [ ] Preset categories and tagging system
- [ ] Preset blending capabilities
- [ ] Auto-backup functionality
- [ ] Network migration tools

---

🎵 **The JSON preset system is fully implemented and ready for use!** Launch NeuronSeqSampler and explore the "Presets" menu to save and load your neural network configurations.