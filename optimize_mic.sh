#!/bin/bash
# Script to optimize audio settings for recording and playback

echo "Optimizing audio settings for recording and playback..."

# Microphone settings for recording
echo "Setting input source to Headphone Mic..."
amixer -c 0 sset "Input Source" "Headphone Mic" > /dev/null 2>&1
amixer -c 0 sset "Input Source",1 "Headphone Mic" > /dev/null 2>&1

# Set moderate microphone boost (reduces static noise)
echo "Setting microphone boost to moderate level (67%)..."
amixer -c 0 sset "Headphone Mic Boost" 2 > /dev/null 2>&1

# Ensure capture is enabled and at reasonable level
echo "Setting capture level to 80%..."
amixer -c 0 sset "Capture" 80% > /dev/null 2>&1
amixer -c 0 sset "Capture" cap > /dev/null 2>&1

# Audio output settings for sample playback
echo "Optimizing audio output settings..."
amixer sset Master 80% > /dev/null 2>&1
amixer -c 0 sset Speaker 80% unmute > /dev/null 2>&1
amixer -c 0 sset PCM 99% > /dev/null 2>&1

# Check current settings
echo ""
echo "Current audio settings:"
echo "=== INPUT (Recording) ==="
echo "Input Source: $(amixer -c 0 sget "Input Source" | grep "Item0:" | cut -d"'" -f2)"
echo "Mic Boost: $(amixer -c 0 sget "Headphone Mic Boost" | grep "Front Left:" | cut -d'[' -f2 | cut -d']' -f1)"
echo "Capture Level: $(amixer -c 0 sget "Capture" | grep "Front Left:" | cut -d'[' -f2 | cut -d']' -f1)"
echo ""
echo "=== OUTPUT (Playback) ==="
echo "Master Volume: $(amixer sget Master | grep "Front Left:" | cut -d'[' -f2 | cut -d']' -f1)"
echo "Speaker: $(amixer -c 0 sget Speaker | grep "Front Left:" | cut -d'[' -f2 | cut -d']' -f1) $(amixer -c 0 sget Speaker | grep "Front Left:" | grep -o "\[on\]\|\[off\]")"
echo "PCM: $(amixer -c 0 sget PCM | grep "Front Left:" | cut -d'[' -f2 | cut -d']' -f1)"

echo ""
echo "Audio system optimized!"
echo "- Recording: Reduced mic boost, optimal capture levels"
echo "- Playback: Enabled speakers, set optimal volume levels"
echo "- Noise reduction applied in software during recording"
