#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <functional>

class Recorder; // Forward declaration
class RhythmInterpreter; // Forward declaration

class AudioManager {
private:
    std::unordered_map<int, std::unique_ptr<sf::SoundBuffer>> soundBuffers;
    std::unordered_map<int, std::unique_ptr<sf::Sound>> sounds;
    std::string samplesDirectory;
    
    // Volume control
    std::unordered_map<int, float> sampleVolumes;  // Per-sample volume (0.0-1.0)
    float masterVolume = 1.0f;  // Master volume (0.0-1.0)
    
    // Internal recording support
    Recorder* internalRecorder;
    bool recordingOutput;
    mutable std::mutex recordingMutex;
    
    // Filter callback for routing samples through filter bank
    std::function<std::vector<float>(const std::vector<float>&)> filterCallback;
    
    // RhythmInterpreter reference for filter modes
    RhythmInterpreter* rhythmInterpreter;
    
    // Storage for filtered sound buffers (needed to keep them alive during playback)
    std::unordered_map<int, std::unique_ptr<sf::SoundBuffer>> filteredBuffers;

    // External input soundfile support
    std::unique_ptr<sf::SoundBuffer> inputBuffer;
    std::unique_ptr<sf::Sound> inputSound;
    std::vector<float> inputSamples;
    std::size_t inputCursor = 0;
    bool inputStreaming = false;

public:
    AudioManager(const std::string& samplesDir = "samples/girliepop/", bool loadDefaults = false);
    ~AudioManager() = default;
    
    bool loadSample(int sampleIndex, const std::string& filename);
    bool loadSampleFromPath(int sampleIndex, const std::string& fullPath);
    void loadDefaultSamples();
    bool playSample(int sampleIndex);
    bool playSample(int sampleIndex, float offsetSeconds); // Play sample from beginning with time offset
    bool playSample(int sampleIndex, float offsetSeconds, float volume); // Play sample with specific volume (0.0-100.0)
    
    void setVolume(float volume);  // 0.0 to 100.0 (legacy - sets master volume)
    void setMasterVolume(float volume);  // 0.0 to 1.0
    void setSampleVolume(int sampleIndex, float volume);  // 0.0 to 1.0
    float getMasterVolume() const { return masterVolume; }
    float getSampleVolume(int sampleIndex) const;
    void stopAllSounds();
    
    bool isSampleLoaded(int sampleIndex) const;
    
    // Sample data access for filtering
    std::vector<float> getSampleData(int sampleIndex) const; // Get sample as float data
    void setFilterCallback(std::function<std::vector<float>(const std::vector<float>&)> callback); // Set filter processing callback
    void setRhythmInterpreter(RhythmInterpreter* interpreter); // Set rhythm interpreter reference
    void setFilterMode(bool enabled); // Enable or disable filter mode
    void setAdaptiveFilterMode(bool enabled); // Enable or disable adaptive filter mode
    bool isFilterModeEnabled() const { return filterCallback != nullptr; }
    
    // Internal recording methods
    void setInternalRecorder(Recorder* recorder);
    void startInternalRecording();
    void stopInternalRecording();
    bool isRecordingOutput() const;

    // External input soundfile API
    bool loadInputFile(const std::string& fullPath);
    void startInputPlayback();
    void pauseInputPlayback();
    void stopInputPlayback();
    bool hasInputFile() const { return inputBuffer != nullptr; }
    bool isInputStreaming() const { return inputStreaming; }
    std::vector<float> getNextInputChunk(std::size_t chunkSize);
};