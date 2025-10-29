#!/usr/bin/env python3
"""
Factory Preset Verification Script
Validates all 6 factory presets for correct JSON format and required fields
"""
import json
import os

def verify_preset(filename):
    """Verify a single preset file"""
    print(f"\n🔍 Verifying {filename}...")
    
    try:
        with open(filename, 'r') as f:
            data = json.load(f)
        
        # Check required top-level keys
        required_keys = ['neurons', 'connections', 'quantization', 'rhythmogram_matrix']
        missing_keys = [key for key in required_keys if key not in data]
        if missing_keys:
            print(f"  ❌ Missing keys: {missing_keys}")
            return False
        
        # Check neurons
        neurons = data['neurons']
        print(f"  📊 Neurons: {len(neurons)}")
        for i, neuron in enumerate(neurons):
            neuron_keys = ['id', 'threshold', 'activation_function', 'sample_file', 'is_output', 'position']
            missing = [key for key in neuron_keys if key not in neuron]
            if missing:
                print(f"    ❌ Neuron {i} missing: {missing}")
                return False
            
            # Check if sample file exists
            sample_path = neuron['sample_file']
            if not os.path.exists(sample_path):
                print(f"    ⚠️  Sample file not found: {sample_path}")
            else:
                print(f"    ✅ Sample file OK: {sample_path}")
        
        # Check connections
        connections = data['connections']
        print(f"  🔗 Connections: {len(connections)}")
        
        # Check quantization
        quant = data['quantization']
        quant_keys = ['enabled', 'resolution', 'swing', 'tempo_bpm']
        missing = [key for key in quant_keys if key not in quant]
        if missing:
            print(f"  ❌ Quantization missing: {missing}")
            return False
        print(f"  ⏱️  Tempo: {quant['tempo_bpm']} BPM, Resolution: {quant['resolution']}")
        
        # Check rhythmogram matrix
        matrix = data['rhythmogram_matrix']
        print(f"  📊 Rhythmogram matrix: {len(matrix)}x{len(matrix[0]) if matrix else 0}")
        
        print(f"  ✅ {filename} is valid!")
        return True
        
    except json.JSONDecodeError as e:
        print(f"  ❌ JSON decode error: {e}")
        return False
    except Exception as e:
        print(f"  ❌ Error: {e}")
        return False

def main():
    print("🏭 Factory Preset Verification")
    print("=" * 40)
    
    preset_dir = "presets/factory"
    if not os.path.exists(preset_dir):
        print(f"❌ Factory preset directory not found: {preset_dir}")
        return
    
    preset_files = [f for f in os.listdir(preset_dir) if f.endswith('.json')]
    if not preset_files:
        print(f"❌ No preset files found in {preset_dir}")
        return
    
    print(f"📁 Found {len(preset_files)} factory presets")
    
    valid_count = 0
    for preset_file in sorted(preset_files):
        filepath = os.path.join(preset_dir, preset_file)
        if verify_preset(filepath):
            valid_count += 1
    
    print(f"\n📊 Summary: {valid_count}/{len(preset_files)} presets are valid")
    
    if valid_count == len(preset_files):
        print("🎉 All factory presets are ready!")
    else:
        print("⚠️  Some presets need attention")

if __name__ == "__main__":
    main()