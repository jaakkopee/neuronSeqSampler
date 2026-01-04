#pragma once
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>

class StartupAnimation {
public:
    static void showWindow(std::function<void()> backgroundTask = nullptr, std::atomic<bool>* loadingComplete = nullptr);

private:
    static std::vector<std::string> getASCIIText();
};