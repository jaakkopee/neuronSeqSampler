#!/bin/bash

echo "🎵 Testing Quantization JSON Format"
echo "==================================="
echo ""

# Test that the updated presets have the correct JSON format
echo "📋 Checking factory preset formats..."

echo ""
echo "🥁 Factory Drum Pattern:"
if [ -f "presets/factory/drum_pattern.json" ]; then
    # Check if quantization section exists
    if grep -q '"quantization"' presets/factory/drum_pattern.json; then
        echo "✅ Quantization section found"
        # Show quantization settings
        echo "📊 Quantization settings:"
        grep -A 10 '"quantization"' presets/factory/drum_pattern.json | head -11
    else
        echo "❌ Quantization section missing"
    fi
else
    echo "❌ Drum pattern preset not found"
fi

echo ""
echo "🌊 Factory Ambient Textures:"
if [ -f "presets/factory/ambient_textures.json" ]; then
    # Check if quantization section exists
    if grep -q '"quantization"' presets/factory/ambient_textures.json; then
        echo "✅ Quantization section found"
        # Show quantization settings
        echo "📊 Quantization settings:"
        grep -A 10 '"quantization"' presets/factory/ambient_textures.json | head -11
    else
        echo "❌ Quantization section missing"
    fi
else
    echo "❌ Ambient textures preset not found"
fi

echo ""
echo "🧪 User Test Preset:"
if [ -f "presets/user/Test.json" ]; then
    # Check if quantization section exists
    if grep -q '"quantization"' presets/user/Test.json; then
        echo "✅ Quantization section found"
        # Show quantization settings
        echo "📊 Quantization settings:"
        grep -A 10 '"quantization"' presets/user/Test.json | head -11
    else
        echo "❌ Quantization section missing"
    fi
else
    echo "❌ Test preset not found"
fi

echo ""
echo "🔧 Testing JSON validity..."

# Test JSON validity of all presets
valid_count=0
total_count=0

for preset in presets/factory/*.json presets/user/*.json; do
    if [ -f "$preset" ]; then
        total_count=$((total_count + 1))
        filename=$(basename "$preset")
        
        # Test JSON validity with python
        if python3 -c "import json; json.load(open('$preset'))" 2>/dev/null; then
            echo "✅ $filename - Valid JSON"
            valid_count=$((valid_count + 1))
        else
            echo "❌ $filename - Invalid JSON"
        fi
    fi
done

echo ""
echo "📊 Summary:"
echo "   Valid JSON files: $valid_count/$total_count"

if [ $valid_count -eq $total_count ]; then
    echo "🎉 All preset files have valid JSON format!"
    
    echo ""
    echo "🔍 Testing new quantization fields..."
    
    # Check for required quantization fields
    required_fields=("enabled" "grid_resolution" "grid_resolution_value" "quantization_amount" "swing_factor" "bpm")
    
    for preset in presets/factory/*.json presets/user/*.json; do
        if [ -f "$preset" ]; then
            filename=$(basename "$preset")
            echo ""
            echo "📁 $filename:"
            
            missing_fields=()
            for field in "${required_fields[@]}"; do
                if grep -q "\"$field\"" "$preset"; then
                    echo "   ✅ $field"
                else
                    echo "   ❌ $field (missing)"
                    missing_fields+=("$field")
                fi
            done
            
            if [ ${#missing_fields[@]} -eq 0 ]; then
                echo "   🎉 All quantization fields present!"
            else
                echo "   ⚠️  Missing ${#missing_fields[@]} field(s)"
            fi
        fi
    done
    
else
    echo "❌ Some preset files have invalid JSON format"
    exit 1
fi

echo ""
echo "✅ Quantization preset format test completed!"