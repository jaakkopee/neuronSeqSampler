#include "StartupAnimation.h"

void StartupAnimation::playAnimation() {
    // Hide cursor and clear screen
    hideCursor();
    clearScreen();
    
    auto asciiText = getASCIIText();
    const int totalFrames = 120; // Animation duration
    const int fps = 20; // Frames per second
    const int frameDelay = 1000 / fps; // Milliseconds per frame
    
    for (int frame = 0; frame < totalFrames; ++frame) {
        moveCursorTo(0, 0); // Move to top-left
        renderFrame(asciiText, frame);
        std::this_thread::sleep_for(std::chrono::milliseconds(frameDelay));
    }
    
    // Show cursor and clear screen for normal operation
    showCursor();
    clearScreen();
}

void StartupAnimation::clearScreen() {
    std::cout << "\033[2J"; // Clear entire screen
    std::cout.flush();
}

void StartupAnimation::moveCursorTo(int row, int col) {
    std::cout << "\033[" << (row + 1) << ";" << (col + 1) << "H";
}

void StartupAnimation::hideCursor() {
    std::cout << "\033[?25l"; // Hide cursor
}

void StartupAnimation::showCursor() {
    std::cout << "\033[?25h"; // Show cursor
}

std::vector<std::string> StartupAnimation::getASCIIText() {
    // 8 rows high ASCII art for "NeuronSeqSampler"
    return {
        "╔╗╔┌─┐┬ ┬┬─┐┌─┐┌┐┌╔═╗┌─┐┌─┐╔═╗┌─┐┌┬┐┌─┐┬  ┌─┐┬─┐",
        "║║║├┤ │ │├┬┘│ ││││╚═╗├┤ │─┼┬╚═╗├─┤│││├─┘│  ├┤ ├┬┘",
        "╝╚╝└─┘└─┘┴└─└─┘┘└┘╚═╝└─┘└─┘┴╚═╝┴ ┴┴ ┴┴  ┴─┘└─┘┴└─",
        "                                                  ",
        "    ♪♫ Neural Network Audio Sampling System ♫♪    ",
        "                                                  ",
        "      █▓▒░ Real-time Audio Processing ░▒▓█        ",
        "                                                  "
    };
}

void StartupAnimation::renderFrame(const std::vector<std::string>& asciiText, int frameNumber) {
    const int screenWidth = 80;  // Terminal width
    const int textWidth = 50;    // Text width
    const double sinePhase = frameNumber * 0.2; // Sine wave speed
    const double scrollSpeed = 2.0; // Horizontal scroll speed
    
    // Calculate base horizontal position (scrolling from right to left)
    int baseX = screenWidth - static_cast<int>(frameNumber * scrollSpeed);
    
    // Render each line with sine wave modulation
    for (int row = 0; row < static_cast<int>(asciiText.size()); ++row) {
        // Calculate sine wave offset for this row
        double sineOffset = std::sin(sinePhase + row * 0.3) * 8.0; // Amplitude of 8 characters
        int x = baseX + static_cast<int>(sineOffset);
        
        // Move cursor to calculated position
        moveCursorTo(row + 2, 0); // Start a bit down from top
        
        // Clear the line first
        std::cout << "\033[K"; // Clear to end of line
        
        // Only render if the text is at least partially visible
        if (x > -textWidth && x < screenWidth) {
            // Move to the calculated x position if positive
            if (x > 0) {
                moveCursorTo(row + 2, x);
            }
            
            // Determine which part of the text to show
            std::string lineText = asciiText[row];
            if (x < 0) {
                // Text starts off-screen to the left, show partial text
                int skipChars = -x;
                if (skipChars < static_cast<int>(lineText.length())) {
                    lineText = lineText.substr(skipChars);
                    moveCursorTo(row + 2, 0);
                }
            }
            
            // Add some color effects
            if (frameNumber % 40 < 20) {
                std::cout << "\033[36m"; // Cyan
            } else {
                std::cout << "\033[35m"; // Magenta
            }
            
            // Add brightness effect based on sine wave
            double brightness = (std::sin(sinePhase * 1.5 + row * 0.5) + 1.0) / 2.0;
            if (brightness > 0.7) {
                std::cout << "\033[1m"; // Bold/bright
            }
            
            std::cout << lineText;
            std::cout << "\033[0m"; // Reset colors
        }
    }
    
    // Add some sparkle effects at the bottom
    moveCursorTo(12, 0);
    std::cout << "\033[K"; // Clear line
    std::string sparkles = "    ";
    for (int i = 0; i < 10; ++i) {
        double sparklePhase = sinePhase * 2.0 + i * 0.8;
        if (std::sin(sparklePhase) > 0.6) {
            sparkles += "✦ ";
        } else if (std::sin(sparklePhase) > 0.0) {
            sparkles += "· ";
        } else {
            sparkles += "  ";
        }
    }
    std::cout << "\033[33m" << sparkles << "\033[0m"; // Yellow sparkles
    
    std::cout.flush();
}