#include "StartupAnimation.h"

void StartupAnimation::showWindow(std::function<void()> backgroundTask, std::atomic<bool>* loadingComplete) {
    // Create a splash window
    sf::RenderWindow splashWindow(sf::VideoMode({600, 400}), "Loading...", sf::Style::None);
    splashWindow.setFramerateLimit(60);
    
    // Start background task if provided
    std::thread* taskThread = nullptr;
    if (backgroundTask) {
        taskThread = new std::thread([backgroundTask, loadingComplete]() {
            backgroundTask();
            if (loadingComplete) {
                loadingComplete->store(true);
            }
        });
    }
    
    // Get ASCII text
    auto asciiText = getASCIIText();
    
    // Load font
    sf::Font font;
    if (!font.openFromFile("/System/Library/Fonts/Menlo.ttc")) {
        if (!font.openFromFile("assets/fonts/DejaVuSans.ttf")) {
            std::cerr << "⚠️  Could not load font for startup animation" << std::endl;
            return;
        }
    }
    
    const int totalFrames = 300;
    const float animationDuration = 5.0f; // 5 seconds
    sf::Clock clock;
    
    while (splashWindow.isOpen() && clock.getElapsedTime().asSeconds() < animationDuration) {
        // Handle events
        while (auto eventOpt = splashWindow.pollEvent()) {
            if (!eventOpt) break;
            const auto& event = *eventOpt;
            
            if (event.is<sf::Event::Closed>()) {
                splashWindow.close();
            }
        }
        
        // Calculate frame
        float progress = clock.getElapsedTime().asSeconds() / animationDuration;
        int frameNumber = static_cast<int>(progress * totalFrames);
        
        // Clear window
        splashWindow.clear(sf::Color(20, 20, 30));
        
        // Draw border
        sf::RectangleShape border(sf::Vector2f{596, 396});
        border.setPosition(sf::Vector2f{2, 2});
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineThickness(3);
        border.setOutlineColor(sf::Color(100, 150, 255));
        splashWindow.draw(border);
        
        // Render animation with centered static text
        const double sinePhase = frameNumber * 0.08;
        
        // Center the text block
        int startX = 80;
        int startY = 60;
        
        for (size_t row = 0; row < asciiText.size(); ++row) {
            // Gentle sine wave offset
            double sineOffset = std::sin(sinePhase + row * 0.5) * 15.0;
            int x = startX + static_cast<int>(sineOffset);
            int y = startY + row * 18;
            
            // Color cycling
            sf::Color textColor;
            if (frameNumber % 120 < 60) {
                textColor = sf::Color(100, 200, 255); // Cyan
            } else {
                textColor = sf::Color(200, 100, 255); // Magenta
            }
            
            // Fade in effect
            float fadeAlpha = std::min(1.0f, progress * 2.0f);
            textColor.a = static_cast<uint8_t>(255 * fadeAlpha);
            
            // Draw text with smaller monospace-friendly size
            sf::Text text(font, asciiText[row], 12);
            text.setPosition(sf::Vector2f{static_cast<float>(x), static_cast<float>(y)});
            text.setFillColor(textColor);
            splashWindow.draw(text);
        }
        
        // Add loading message at bottom
        sf::Text loadingText(font, "Loading Neuron Sequence Sampler...", 12);
        loadingText.setPosition(sf::Vector2f{120, 320});
        loadingText.setFillColor(sf::Color(150, 150, 150));
        splashWindow.draw(loadingText);
        
        // Progress bar background
        sf::RectangleShape progressBg(sf::Vector2f{400, 6});
        progressBg.setPosition(sf::Vector2f{90, 345});
        progressBg.setFillColor(sf::Color(40, 40, 50));
        progressBg.setOutlineThickness(1);
        progressBg.setOutlineColor(sf::Color(80, 80, 100));
        splashWindow.draw(progressBg);
        
        // Progress bar
        float barWidth = 400.0f * progress;
        sf::RectangleShape progressFill(sf::Vector2f{barWidth, 6});
        progressFill.setPosition(sf::Vector2f{90, 345});
        progressFill.setFillColor(sf::Color(100, 200, 255));
        splashWindow.draw(progressFill);
        
        splashWindow.display();
    }
    
    splashWindow.close();
    
    // Wait for background task to complete if it's still running
    if (taskThread) {
        if (taskThread->joinable()) {
            taskThread->join();
        }
        delete taskThread;
    }
}

std::vector<std::string> StartupAnimation::getASCIIText() {
    return {
        " _   _ _____ _   _ ____   ___  _   _ ",
        "| \\ | | ____| | | |  _ \\ / _ \\| \\ | |",
        "|  \\| |  _| | | | | |_) | | | |  \\| |",
        "| |\\  | |___| |_| |  _ <| |_| | |\\  |",
        "|_| \\_|_____|\\___/|_| \\_\\\\___/|_| \\_|",
        "                                      ",
        "     ____  _____ ___                  ",
        "    / ___|| ____/ _ \\                 ",
        "    \\___ \\|  _|| | | |                ",
        "     ___) | |__| |_| |                ",
        "    |____/|_____\\___/                 ",
        "                                      ",
        "   ____    _    __  __ ____  _     _____ ____  ",
        "  / ___|  / \\  |  \\/  |  _ \\| |   | ____|  _ \\ ",
        "  \\___ \\ / _ \\ | |\\/| | |_) | |   |  _| | |_) |",
        "   ___) / ___ \\| |  | |  __/| |___| |___|  _ < ",
        "  |____/_/   \\_\\_|  |_|_|   |_____|_____|_| \\_\\"
    };
}
