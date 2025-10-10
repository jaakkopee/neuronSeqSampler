#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <functional>

class Recorder; // Forward declaration

class AudioManager {
private:
    std::unordered_map<int, std::unique_ptr<sf::SoundBuffer>> soundBuffers;
    std::unordered_map<int, std::unique_ptr<sf::Sound>> sounds;
    std::string samplesDirectory;
    
    // Internal recording support
    Recorder* internalRecorder;
    bool recordingOutput;
    mutable std::mutex recordingMutex;
    
    // Filter callback for routing samples through filter bank
    std::function<std::vector<float>(const std::vector<float>&)> filterCallback;

public:
    AudioManager(const std::string& samplesDir = "samples/girliepop/", bool loadDefaults = false);
    ~AudioManager() = default;
    
    bool loadSample(int sampleIndex, const std::string& filename);
    bool loadSampleFromPath(int sampleIndex, const std::string& fullPath);
    void loadDefaultSamples();
    bool playSample(int sampleIndex);
    bool playSample(int sampleIndex, float offsetSeconds); // Play sample from beginning with time offset
    
    void setVolume(float volume);  // 0.0 to 100.0
    void stopAllSounds();
    
    bool isSampleLoaded(int sampleIndex) const;
    
    // Sample data access for filtering
    std::vector<float> getSampleData(int sampleIndex) const; // Get sample as float data
    void setFilterCallback(std::function<std::vector<float>(const std::vector<float>&)> callback); // Set filter processing callback
    bool isFilterModeEnabled() const { return filterCallback != nullptr; }
    
    // Internal recording methods
    void setInternalRecorder(Recorder* recorder);
    void startInternalRecording();
    void stopInternalRecording();
    bool isRecordingOutput() const;
};