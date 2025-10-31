#!/usr/bin/env python3
"""
Manual factory preset creation test
Creates the 6 factory presets directly by copying the logic from PresetManager.cpp
"""
import json
import os

def create_factory_presets():
    """Create the 6 factory presets as defined in PresetManager.cpp"""
    
    # Ensure directory exists
    os.makedirs("presets/factory", exist_ok=True)
    
    # 1. Basic Drum Pattern (beginner-friendly)
    basic_drum = {
        "preset_info": {
            "name": "Basic Drum Pattern",
            "version": "1.0",
            "author": "NeuronSeqSampler",
            "description": "Simple 3-neuron drum pattern perfect for beginners",
            "created_date": "2025-10-29T00:00:00Z",
            "tags": "drum,basic,beginner,3neuron"
        },
        "neurons": [
            {
                "id": 0,
                "sample_index": 1,
                "sample_file_path": "samples/kick/kick (ghost).wav",
                "activation": 0.0,
                "threshold": 0.7,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Linear",
                "position": [100.0, 100.0]
            },
            {
                "id": 1,
                "sample_index": 2,
                "sample_file_path": "samples/snare/snare (ghost).wav",
                "activation": 0.0,
                "threshold": 0.6,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Linear",
                "position": [200.0, 100.0]
            },
            {
                "id": 2,
                "sample_index": 3,
                "sample_file_path": "samples/hat/hat 2 (ghost).wav",
                "activation": 0.0,
                "threshold": 0.5,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Linear",
                "position": [300.0, 100.0]
            }
        ],
        "connections": [
            {"from": 0, "to": 1, "weight": 0.8},
            {"from": 1, "to": 2, "weight": 0.6},
            {"from": 2, "to": 0, "weight": 0.4}
        ],
        "quantization": {
            "enabled": True,
            "resolution": 3,  # 1/16 note
            "swing": 0.0,
            "tempo_bpm": 120.0
        },
        "rhythmogram_matrix": [
            [0.8, 0.2, 0.1],
            [0.3, 0.9, 0.2],
            [0.2, 0.3, 0.7]
        ]
    }
    
    # 2. Complex Network (5-neuron interconnected)
    complex_network = {
        "preset_info": {
            "name": "Complex Network",
            "version": "1.0", 
            "author": "NeuronSeqSampler",
            "description": "5-neuron interconnected network with mixed activation functions",
            "created_date": "2025-10-29T00:00:00Z",
            "tags": "complex,5neuron,mixed,interconnected"
        },
        "neurons": [
            {
                "id": 0,
                "sample_index": 1,
                "sample_file_path": "samples/kick/kick (krystle).wav",
                "activation": 0.0,
                "threshold": 0.6,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "ReLU",
                "position": [150.0, 80.0]
            },
            {
                "id": 1,
                "sample_index": 2,
                "sample_file_path": "samples/snare/snare (ionic funk).wav",
                "activation": 0.0,
                "threshold": 0.5,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Sigmoid",
                "position": [250.0, 120.0]
            },
            {
                "id": 2,
                "sample_index": 3,
                "sample_file_path": "samples/hat/open hat (ionic funk).wav",
                "activation": 0.0,
                "threshold": 0.7,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Tanh",
                "position": [200.0, 200.0]
            },
            {
                "id": 3,
                "sample_index": 4,
                "sample_file_path": "samples/perc/clav (frenesi).wav",
                "activation": 0.0,
                "threshold": 0.4,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "ReLU",
                "position": [100.0, 180.0]
            },
            {
                "id": 4,
                "sample_index": 5,
                "sample_file_path": "samples/perc/rim (ghost).wav",
                "activation": 0.0,
                "threshold": 0.8,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Sigmoid",
                "position": [300.0, 160.0]
            }
        ],
        "connections": [
            {"from": 0, "to": 1, "weight": 0.9},
            {"from": 1, "to": 2, "weight": 0.7},
            {"from": 2, "to": 3, "weight": 0.6},
            {"from": 3, "to": 4, "weight": 0.8},
            {"from": 4, "to": 0, "weight": 0.5},
            {"from": 0, "to": 3, "weight": 0.4},
            {"from": 2, "to": 4, "weight": 0.3}
        ],
        "quantization": {
            "enabled": True,
            "resolution": 4,  # 1/8 note
            "swing": 0.1,
            "tempo_bpm": 128.0
        },
        "rhythmogram_matrix": [
            [0.9, 0.1, 0.2, 0.3, 0.1],
            [0.2, 0.8, 0.4, 0.1, 0.2],
            [0.1, 0.3, 0.7, 0.5, 0.3],
            [0.4, 0.2, 0.2, 0.6, 0.4],
            [0.1, 0.1, 0.3, 0.2, 0.8]
        ]
    }
    
    # 3. Sigmoid Cascade (smooth dynamics)
    sigmoid_cascade = {
        "neurons": [
            {
                "id": 0,
                "sample_index": 1,
                "sample_file_path": "samples/kick/kick (hidden power).wav",
                "activation": 0.0,
                "threshold": 0.3,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Sigmoid",
                "position": [120.0, 100.0]
            },
            {
                "id": 1,
                "sample_index": 2,
                "sample_file_path": "samples/snare/snare (post rave maximalist).wav",
                "activation": 0.0,
                "threshold": 0.4,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Sigmoid",
                "position": [220.0, 120.0]
            },
            {
                "id": 2,
                "sample_index": 3,
                "sample_file_path": "samples/hat/open hat (post rave maximalist).wav",
                "activation": 0.0,
                "threshold": 0.5,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Sigmoid",
                "position": [180.0, 200.0]
            },
            {
                "id": 3,
                "sample_index": 4,
                "sample_file_path": "samples/perc/chant 2 (wlfgrl acid).wav",
                "activation": 0.0,
                "threshold": 0.35,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Sigmoid",
                "position": [280.0, 180.0]
            }
        ],
        "connections": [
            {"from": 0, "to": 1, "weight": 0.6},
            {"from": 1, "to": 2, "weight": 0.7},
            {"from": 2, "to": 3, "weight": 0.5},
            {"from": 3, "to": 0, "weight": 0.8}
        ],
        "quantization": {
            "enabled": True,
            "resolution": 2,  # 1/4 note
            "swing": 0.05,
            "tempo_bpm": 95.0
        },
        "rhythmogram_matrix": [
            [0.7, 0.3, 0.2, 0.1],
            [0.4, 0.8, 0.3, 0.2],
            [0.2, 0.4, 0.9, 0.3],
            [0.3, 0.2, 0.4, 0.6]
        ]
    }
    
    # 4. High Energy (fast-paced)
    high_energy = {
        "neurons": [
            {
                "id": 0,
                "sample_index": 1,
                "sample_file_path": "samples/kick.wav",
                "activation": 0.0,
                "threshold": 0.2,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Linear",
                "position": [150.0, 90.0]
            },
            {
                "id": 1,
                "sample_index": 2,
                "sample_file_path": "samples/snare.wav",
                "activation": 0.0,
                "threshold": 0.15,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "ReLU",
                "position": [250.0, 110.0]
            },
            {
                "id": 2,
                "sample_index": 3,
                "sample_file_path": "samples/hihat.wav",
                "activation": 0.0,
                "threshold": 0.1,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Linear",
                "position": [200.0, 170.0]
            }
        ],
        "connections": [
            {"from": 0, "to": 1, "weight": 0.9},
            {"from": 1, "to": 2, "weight": 0.8},
            {"from": 2, "to": 0, "weight": 0.7},
            {"from": 0, "to": 2, "weight": 0.6}
        ],
        "quantization": {
            "enabled": True,
            "resolution": 5,  # 1/32 note
            "swing": 0.0,
            "tempo_bpm": 140.0
        },
        "rhythmogram_matrix": [
            [0.9, 0.4, 0.3],
            [0.5, 0.8, 0.4],
            [0.4, 0.5, 0.9]
        ]
    }
    
    # 5. Ambient Flow (slow, organic)
    ambient_flow = {
        "neurons": [
            {
                "id": 0,
                "sample_index": 1,
                "sample_file_path": "samples/kick.wav",
                "activation": 0.0,
                "threshold": 0.8,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Tanh",
                "position": [180.0, 120.0]
            },
            {
                "id": 1,
                "sample_index": 2,
                "sample_file_path": "samples/perc1.wav",
                "activation": 0.0,
                "threshold": 0.75,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Tanh",
                "position": [220.0, 180.0]
            },
            {
                "id": 2,
                "sample_index": 3,
                "sample_file_path": "samples/perc2.wav",
                "activation": 0.0,
                "threshold": 0.9,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Tanh",
                "position": [140.0, 200.0]
            }
        ],
        "connections": [
            {"from": 0, "to": 1, "weight": 0.3},
            {"from": 1, "to": 2, "weight": 0.4},
            {"from": 2, "to": 0, "weight": 0.2}
        ],
        "quantization": {
            "enabled": True,
            "resolution": 1,  # 1/2 note
            "swing": 0.15,
            "tempo_bpm": 85.0
        },
        "rhythmogram_matrix": [
            [0.6, 0.2, 0.1],
            [0.3, 0.7, 0.2],
            [0.2, 0.3, 0.5]
        ]
    }
    
    # 6. Experimental (mixed unusual setup)
    experimental = {
        "neurons": [
            {
                "id": 0,
                "sample_index": 1,
                "sample_file_path": "samples/kick.wav",
                "activation": 0.0,
                "threshold": 0.95,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "ReLU",
                "position": [160.0, 80.0]
            },
            {
                "id": 1,
                "sample_index": 2,
                "sample_file_path": "samples/snare.wav",
                "activation": 0.0,
                "threshold": 0.1,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Sigmoid",
                "position": [240.0, 140.0]
            },
            {
                "id": 2,
                "sample_index": 3,
                "sample_file_path": "samples/hihat.wav",
                "activation": 0.0,
                "threshold": 0.5,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Tanh",
                "position": [120.0, 190.0]
            },
            {
                "id": 3,
                "sample_index": 4,
                "sample_file_path": "samples/perc1.wav",
                "activation": 0.0,
                "threshold": 0.25,
                "decay_rate": 1.0,
                "activation_increase_per_iteration": 0.0,
                "activation_function": "Linear",
                "position": [280.0, 160.0]
            }
        ],
        "connections": [
            {"from": 1, "to": 0, "weight": 0.9},
            {"from": 0, "to": 2, "weight": 0.1},
            {"from": 2, "to": 3, "weight": 0.8},
            {"from": 3, "to": 1, "weight": 0.3},
            {"from": 1, "to": 2, "weight": 0.6}
        ],
        "quantization": {
            "enabled": True,
            "resolution": 6,  # Triplet
            "swing": 0.2,
            "tempo_bpm": 110.0
        },
        "rhythmogram_matrix": [
            [0.2, 0.8, 0.3, 0.1],
            [0.9, 0.1, 0.4, 0.5],
            [0.1, 0.6, 0.4, 0.2],
            [0.3, 0.2, 0.7, 0.6]
        ]
    }
    
    # Write all presets to files
    presets = {
        "drum_pattern": basic_drum,
        "complex_network": complex_network,
        "sigmoid_cascade": sigmoid_cascade,
        "high_energy": high_energy,
        "ambient_flow": ambient_flow,
        "experimental": experimental
    }
    
    print("🏭 Creating 6 factory presets...")
    for name, preset in presets.items():
        filename = f"presets/factory/{name}.json"
        try:
            with open(filename, 'w') as f:
                json.dump(preset, f, indent=2)
            print(f"✅ Created: {filename}")
        except Exception as e:
            print(f"❌ Failed to create {filename}: {e}")
    
    print(f"\n🎵 Successfully created {len(presets)} factory presets!")
    
    # Verify files exist
    print("\n📁 Verification:")
    for name in presets.keys():
        filename = f"presets/factory/{name}.json"
        if os.path.exists(filename):
            size = os.path.getsize(filename)
            print(f"  ✅ {name}.json ({size} bytes)")
        else:
            print(f"  ❌ {name}.json (missing)")

if __name__ == "__main__":
    create_factory_presets()