#include "AudioManager.h"
#include <iostream>

AudioManager::AudioManager(const std::string& samplesDir) 
    : samplesDirectory(samplesDir)
{
    loadDefaultSamples();
}

bool AudioManager::loadSample(int sampleIndex, const std::string& filename) {
    std::string fullPath = samplesDirectory + filename;
    
    auto buffer = std::make_unique<sf::SoundBuffer>();
    if (!buffer->loadFromFile(fullPath)) {
        std::cerr << "Failed to load audio file: " << fullPath << std::endl;
        return false;
    }
    
    auto sound = std::make_unique<sf::Sound>();
    sound->setBuffer(*buffer);
    
    soundBuffers[sampleIndex] = std::move(buffer);
    sounds[sampleIndex] = std::move(sound);
    
    std::cout << "Loaded sample " << sampleIndex << " from " << fullPath << std::endl;
    return true;
}

void AudioManager::loadDefaultSamples() {
    // Load samples 1-6 from girliepop directory (based on the Python implementation)
    for (int i = 1; i <= 6; i++) {
        std::string filename = std::to_string(i) + ".wav";
        loadSample(i, filename);
    }
}

bool AudioManager::playSample(int sampleIndex) {
    auto it = sounds.find(sampleIndex);
    if (it != sounds.end()) {
        // Stop the sound if it's already playing to allow overlapping
        it->second->stop();
        it->second->play();
        std::cout << "Playing sample " << sampleIndex << std::endl;
        return true;
    }
    
    std::cerr << "Sample " << sampleIndex << " not found." << std::endl;
    return false;
}

void AudioManager::setVolume(float volume) {
    // Clamp volume to valid range
    volume = std::max(0.0f, std::min(100.0f, volume));
    
    for (auto& pair : sounds) {
        pair.second->setVolume(volume);
    }
}

void AudioManager::stopAllSounds() {
    for (auto& pair : sounds) {
        pair.second->stop();
    }
}

bool AudioManager::isSampleLoaded(int sampleIndex) const {
    return soundBuffers.find(sampleIndex) != soundBuffers.end();
}