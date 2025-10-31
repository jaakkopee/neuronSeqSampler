# Fullscreen Toggle Feature

## Overview
Added a fullscreen toggle feature with F11 key and adaptive layout system that works at any screen resolution.

## Features Added:

### 🖥️ **Fullscreen Toggle (F11 Key)**
- **Windowed Mode**: Default 1280x720 window (more manageable size)
- **Fullscreen Mode**: Uses desktop resolution for maximum workspace
- **Seamless Toggle**: F11 key works anywhere in the app (global shortcut)
- **State Persistence**: Remembers window position when switching back

### 📐 **Adaptive Layout System**
The layout automatically adjusts to any screen size using proportional calculations:

#### **Canvas Area (Neuron Visualization)**:
- Takes up 65% of screen width (or 85% of height, whichever is smaller)
- Always maintains square aspect ratio to prevent neuron stretching
- Positioned with 80px padding from edges

#### **GUI Panel (Controls)**:
- Positioned to the right of the canvas area
- Takes remaining width minus small padding
- Full height of the window

#### **Spectral Display**:
- Positioned below the canvas area
- Width matches canvas width
- Fixed 150px height (or less if window is small)

#### **Neuron Sizing**:
- Radius automatically scales with canvas size (canvas_size / 40)
- Minimum radius of 15px for small windows
- Ensures neurons remain visible and clickable at any resolution

## Window Sizes Supported:

### **Windowed Mode (Default)**:
- **Size**: 1280x720 pixels
- **Canvas**: ~830x830 square
- **GUI Panel**: ~370x720
- **Neuron Radius**: ~20 pixels

### **Fullscreen Mode Examples**:

#### 1920x1080 (Full HD):
- **Canvas**: ~1250x1250 square  
- **GUI Panel**: ~590x1080
- **Neuron Radius**: ~31 pixels

#### 2560x1440 (2K):
- **Canvas**: ~1664x1664 square
- **GUI Panel**: ~776x1440  
- **Neuron Radius**: ~41 pixels

#### 3840x2160 (4K):
- **Canvas**: ~2496x2496 square
- **GUI Panel**: ~1244x2160
- **Neuron Radius**: ~62 pixels

## Usage:
1. **Start**: App opens in 1280x720 windowed mode
2. **Fullscreen**: Press F11 to go fullscreen using your monitor's native resolution
3. **Windowed**: Press F11 again to return to windowed mode
4. **Automatic**: Layout adapts instantly to new window size

## Technical Implementation:
- **Window Recreation**: Properly handles SFML window recreation for mode switching
- **TGUI Reinitialization**: GUI target and layout update after window changes  
- **State Management**: Tracks fullscreen state and saves/restores window position
- **Proportional Layout**: All elements scale proportionally to window dimensions

## Benefits:
- ✅ **Flexible Usage**: Start small, go big when needed
- ✅ **No Stretching**: Neurons always maintain perfect circular shape
- ✅ **Any Resolution**: Works on any monitor size from laptops to 4K displays
- ✅ **Smooth Toggle**: Instant switching with F11
- ✅ **Optimized Space**: Makes best use of available screen real estate