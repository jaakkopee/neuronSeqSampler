#!/bin/bash

echo "🎵 Demonstrating Portable Preset Example"
echo "======================================="

# Create example preset JSON with relative sample paths
cat > example_portable_preset.json << 'JSON_EOF'
{
  "preset_info": {
    "name": "Portable Beat Pattern",
    "version": "1.0",
    "author": "NeuronSeqSampler",
    "description": "Example preset demonstrating portable sample paths",
    "created_date": "2025-10-25",
    "tags": "beat,portable,example"
  },
  "neurons": [
    {
      "id": 0,
      "sample_index": 1,
      "sample_file_path": "samples/kick/kick.wav",
      "activation": 0.0,
      "threshold": 1.0,
      "decay_rate": 1.0,
      "activation_increase_per_iteration": 0.1,
      "activation_function": "Linear"
    },
    {
      "id": 1,
      "sample_index": 2,
      "sample_file_path": "samples/snare/snare.wav",
      "activation": 0.0,
      "threshold": 0.8,
      "decay_rate": 0.9,
      "activation_increase_per_iteration": 0.15,
      "activation_function": "ReLU"
    },
    {
      "id": 2,
      "sample_index": 3,
      "sample_file_path": "samples/hihat/hihat.wav",
      "activation": 0.0,
      "threshold": 0.6,
      "decay_rate": 0.95,
      "activation_increase_per_iteration": 0.2,
      "activation_function": "Sigmoid"
    }
  ],
  "connections": [
    {
      "source_id": 0,
      "target_id": 1,
      "weight": 0.7
    },
    {
      "source_id": 1,
      "target_id": 2,
      "weight": 0.5
    },
    {
      "source_id": 2,
      "target_id": 0,
      "weight": 0.3
    }
  ],
  "quantization": {
    "grid_resolution": "Sixteenth",
    "quantization_amount": 0.8,
    "swing_factor": 0.1,
    "bpm": 120.0
  },
  "rhythmogram_matrix": []
}
JSON_EOF

echo "📄 Created example portable preset: example_portable_preset.json"
echo ""
echo "🔍 Key features of this preset:"
echo "• Uses relative sample paths (samples/kick/kick.wav, etc.)"
echo "• Works on any system with matching sample directory structure"  
echo "• Can be shared without path dependencies"
echo "• Includes complete neuron network configuration"
echo ""

echo "🌍 Portability demonstration:"
echo "User A (Linux):   /home/userA/neuronSeqSampler/samples/kick/kick.wav"
echo "User B (Windows): C:\\Music\\neuronSeqSampler\\samples\\kick\\kick.wav"
echo "User C (macOS):   /Users/userC/Music/neuronSeqSampler/samples/kick/kick.wav"
echo ""
echo "✅ Same preset works for all users (relative paths resolve correctly)"
echo ""

echo "📋 Sample paths in this preset:"
python3 << 'PYTHON_EOF'
import json

with open('example_portable_preset.json', 'r') as f:
    preset = json.load(f)

for i, neuron in enumerate(preset['neurons']):
    sample_path = neuron.get('sample_file_path', 'No path specified')
    sample_index = neuron['sample_index']
    print(f"  Neuron {i}: Sample #{sample_index} -> {sample_path}")
PYTHON_EOF

# Clean up the example file
rm -f example_portable_preset.json

echo ""
echo "✨ With relative paths, presets are now truly portable and shareable!"