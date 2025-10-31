# Keyboard Input Blocking Test

## Test the keyboard shortcut blocking functionality

### What was implemented:
1. **isTextInputActive()** method in GUI class that recursively checks all widgets for focused EditBox
2. **checkWidgetTreeForFocusedEditBox()** helper method that traverses the widget tree
3. **Modified main.cpp** to check if text input is active before processing keyboard shortcuts

### How to test:
1. Run the application: `./test_build`
2. Try keyboard shortcuts (like preset shortcuts) - they should work normally
3. Open a dialog that requires text input (e.g., "Save Preset As..." from File menu)
4. While in the text input field, try the same keyboard shortcuts
5. **Expected result**: Keyboard shortcuts should be blocked when typing in text fields
6. Press Escape or click outside to close the dialog
7. Keyboard shortcuts should work normally again

### Test scenarios:
- **Preset naming**: File → Save Preset As... → type in the name field
- **Recording naming**: When prompted to name a recording
- **Neuron threshold input**: Right-click on neuron → enter custom threshold
- **Any EditBox input**: All text input fields should block global keyboard shortcuts

### Technical details:
- The system checks if any EditBox widget anywhere in the GUI tree has focus
- When text input is active, keyboard events are not forwarded to the application
- Only affects global shortcuts, not basic text editing (Ctrl+C, Ctrl+V, etc.)
- TGUI still handles text editing keys internally even when our shortcuts are blocked