#!/bin/bash

# Build script for Neuron Sequence Sampler (C++)

set -e

echo "Building Neuron Sequence Sampler..."

# Create build directory
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..

# Build
echo "Building..."
make -j$(nproc)

echo ""
echo "Build completed successfully!"
echo "Run the application with: ./build/NeuronSeqSampler"
echo ""
echo "Note: Make sure you have audio samples in samples/girliepop/ directory"
echo "      (files named 1.wav through 6.wav)"