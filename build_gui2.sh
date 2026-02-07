#!/bin/bash
# Build script for GUI2 modular system

echo "==================================="
echo "neuronSeqSampler GUI2 Build Script"
echo "==================================="
echo ""

# Check for SFML
echo "Checking for SFML..."
if ! pkg-config --exists sfml-all 2>/dev/null && ! pkg-config --exists sfml-graphics 2>/dev/null; then
    echo "Warning: SFML not found via pkg-config"
    echo "Please ensure SFML 2.5+ is installed:"
    echo "  macOS: brew install sfml"
    echo "  Linux: sudo apt-get install libsfml-dev"
    echo ""
fi

# Create build directory
BUILD_DIR="build_gui2"
echo "Creating build directory: $BUILD_DIR"
mkdir -p $BUILD_DIR

# Temporarily use CMakeLists_GUI2.txt as the main CMakeLists.txt
# Save existing CMakeLists.txt if it exists
if [ -f "CMakeLists.txt" ]; then
    mv CMakeLists.txt CMakeLists.txt.backup_gui2
fi
cp CMakeLists_GUI2.txt CMakeLists.txt

# Run CMake from project root, output to build directory
echo ""
echo "Running CMake..."
cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    # Restore original CMakeLists.txt
    rm -f CMakeLists.txt
    if [ -f "CMakeLists.txt.backup_gui2" ]; then
        mv CMakeLists.txt.backup_gui2 CMakeLists.txt
    fi
    echo ""
    echo "ERROR: CMake configuration failed"
    echo "Please check that all dependencies are installed"
    exit 1
fi

# Build
echo ""
echo "Building..."
cmake --build $BUILD_DIR -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

BUILD_RESULT=$?

# Restore original CMakeLists.txt
rm -f CMakeLists.txt
if [ -f "CMakeLists.txt.backup_gui2" ]; then
    mv CMakeLists.txt.backup_gui2 CMakeLists.txt
fi

if [ $BUILD_RESULT -ne 0 ]; then
    echo ""
    echo "ERROR: Build failed"
    exit 1
fi

echo ""
echo "==================================="
echo "Build successful!"
echo "==================================="
echo ""
echo "Run the demo with:"
echo "  ./$BUILD_DIR/gui2_demo"
echo ""
echo "Or from project root:"
echo "  ./build_gui2.sh && ./build_gui2/gui2_demo"
echo ""
