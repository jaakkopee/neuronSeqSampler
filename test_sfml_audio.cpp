#include <SFML/Audio.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <thread>
#include <chrono>

int main() {
    std::cout << "SFML Audio Output Test" << std::endl;
    std::cout << "=======================" << std::endl;
    
    // Generate a simple sine wave
    const unsigned int sampleRate = 44100;
    const unsigned int duration = 2; // seconds
    const float frequency = 440.0f; // A4 note
    
    std::vector<sf::Int16> samples(sampleRate * duration);
    
    for (unsigned int i = 0; i < samples.size(); ++i) {
        float time = static_cast<float>(i) / sampleRate;
        float amplitude = 0.3f; // 30% volume
        samples[i] = static_cast<sf::Int16>(amplitude * 32767 * sin(2 * M_PI * frequency * time));
    }
    
    // Create sound buffer and sound
    sf::SoundBuffer buffer;
    if (!buffer.loadFromSamples(samples.data(), samples.size(), 1, sampleRate)) {
        std::cerr << "Failed to create sound buffer" << std::endl;
        return 1;
    }
    
    sf::Sound sound(buffer);
    
    std::cout << "Playing 440Hz tone for 2 seconds..." << std::endl;
    sound.play();
    
    // Wait for the sound to finish
    while (sound.getStatus() == sf::Sound::Playing) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Status: " << sound.getStatus() << std::endl;
    }
    
    std::cout << "Sound finished. Status: " << sound.getStatus() << std::endl;
    
    return 0;
}
