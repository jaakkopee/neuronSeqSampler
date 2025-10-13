#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>

class StartupAnimation {
public:
    static void playAnimation();

private:
    static void clearScreen();
    static void moveCursorTo(int row, int col);
    static void hideCursor();
    static void showCursor();
    static std::vector<std::string> getASCIIText();
    static void renderFrame(const std::vector<std::string>& asciiText, int frameNumber);
};