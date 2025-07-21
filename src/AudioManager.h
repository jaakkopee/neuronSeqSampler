#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>

class AudioManager {
private:
    std::unordered_map<int, std::unique_ptr<sf::SoundBuffer>> soundBuffers;
    std::unordered_map<int, std::unique_ptr<sf::Sound>> sounds;
    std::string samplesDirectory;

public:
    AudioManager(const std::string& samplesDir = "samples/girliepop/");
    ~AudioManager() = default;
    
    bool loadSample(int sampleIndex, const std::string& filename);
    void loadDefaultSamples();
    bool playSample(int sampleIndex);
    
    void setVolume(float volume);  // 0.0 to 100.0
    void stopAllSounds();
    
    bool isSampleLoaded(int sampleIndex) const;
};