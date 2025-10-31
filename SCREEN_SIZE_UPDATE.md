# Screen Size Update: 1920x1080 Layout

## Changes Made
Updated the default screen size from 1024x800 to 1920x1080 to prevent neuron stretching and provide more space.

### Window Size:
- **Before**: `sf::VideoMode({1024, 800})` 
- **After**: `sf::VideoMode({1920, 1080})`

### Layout Adjustments:

#### Visualizer Canvas (Neuron Area):
- **Before**: 50,50 to 700,700 (650x650 square)
- **After**: 80,80 to 980,980 (900x900 square)
- **Benefit**: Much larger square area for neurons, preventing stretching

#### Neuron Radius:
- **Before**: 20.0f pixels
- **After**: 25.0f pixels  
- **Benefit**: Slightly larger neurons for better visibility on larger screen

#### GUI Panel Area:
- **Before**: 700,0 to 1024,800 (324x800)
- **After**: 1100,0 to 1920,1080 (820x1080)
- **Benefit**: Much wider GUI panel with more space for controls

#### Spectral Display:
- **Before**: Position 50,600 Size 600x150
- **After**: Position 80,900 Size 900x150
- **Benefit**: Wider spectral display positioned at bottom

## Layout Proportions:
```
┌─────────────────────────────────────────────────────┐
│                1920 x 1080                          │
│  ┌─────────────────────┐  ┌───────────────────────┐ │
│  │                     │  │                       │ │
│  │   Neuron Canvas     │  │      GUI Controls     │ │
│  │   900x900 square    │  │      820x1080         │ │
│  │   (80,80->980,980)  │  │   (1100,0->1920,1080) │ │
│  │                     │  │                       │ │
│  └─────────────────────┘  └───────────────────────┘ │
│  ┌─────────────────────┐                            │
│  │  Spectral Display   │                            │
│  │  900x150            │                            │
│  │  (80,900->980,1050) │                            │
│  └─────────────────────┘                            │
└─────────────────────────────────────────────────────┘
```

## Benefits:
- ✅ **No Neuron Stretching**: Square canvas maintains proper aspect ratio
- ✅ **More Space**: Larger canvas for more neurons and better visualization
- ✅ **Better GUI**: Much wider control panel for easier access
- ✅ **Modern Resolution**: Matches common widescreen monitor resolution
- ✅ **Improved Visibility**: Larger elements are easier to see and interact with

## Testing:
Run `./test_build` to see the new layout. The window should now be much larger and neurons should maintain their proper circular shape without stretching.