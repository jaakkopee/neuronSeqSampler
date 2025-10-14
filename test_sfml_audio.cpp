
#include <SFML/Audio.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdint>

int main() {
    std::cout << "SFML Audio Output Test" << std::endl;
    std::cout << "=======================" << std::endl;
    
    // Generate a simple sine wave
    const unsigned int sampleRate = 44100;
    const unsigned int duration = 2; // seconds
    const float frequency = 440.0f; // A4 note
    
    std::vector<std::int16_t> samples(sampleRate * duration);
    for (unsigned int i = 0; i < samples.size(); ++i) {
        float time = static_cast<float>(i) / sampleRate;
        float amplitude = 0.3f; // 30% volume
        samples[i] = static_cast<std::int16_t>(amplitude * 32767 * sin(2 * M_PI * frequency * time));
    }
    
    // Create sound buffer and sound
    sf::SoundBuffer buffer;
    if (!buffer.loadFromSamples(samples.data(), samples.size(), 1, sampleRate, std::vector<sf::SoundChannel>{})) {
        std::cerr << "Failed to create sound buffer" << std::endl;
        return 1;
    }
    
    sf::Sound sound(buffer);
    
    std::cout << "Playing 440Hz tone for 2 seconds..." << std::endl;
    sound.play();
    
    // Wait for the sound to finish
    while (sound.getStatus() == sf::Sound::Status::Playing) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Status: Playing" << std::endl;
    }
    std::cout << "Sound finished. Status: "
              << (sound.getStatus() == sf::Sound::Status::Stopped ? "Stopped" : "Other") << std::endl;
    
    return 0;
}
