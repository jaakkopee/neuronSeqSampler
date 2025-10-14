#pragma once
#include <SFML/Audio.hpp>
#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <chrono>

class Recorder : public sf::SoundRecorder {
private:
    std::vector<std::int16_t> samples;
    std::string outputFilename;
    bool isRecordingToFile;
    bool currentlyRecording;
    bool isInternalRecording; // Flag for internal recording mode
    mutable std::mutex samplesMutex;
    
    // Recording parameters
    unsigned int recordingSampleRate; // Store sample rate for internal recording
    unsigned int recordingChannelCount; // Store channel count
    
    // Real-time recording buffer for internal recording
    std::vector<float> realtimeBuffer; // Mixed audio buffer
    std::chrono::steady_clock::time_point recordingStartTime;
    mutable std::mutex realtimeBufferMutex;
    
    // Active sample tracking for proper stopping behavior
    struct ActiveSample {
        int sampleIndex;
        size_t startPosition;
        size_t sampleLength;
        std::vector<float> sampleData; // Normalized sample data
        bool active; // Whether this sample should continue playing
    };
    std::vector<ActiveSample> activeSamples;
    mutable std::mutex activeSamplesMutex;
    
    // Noise reduction settings
    bool noiseGateEnabled;
    float noiseGateThreshold;
    bool highPassFilterEnabled;
    float highPassFrequency;
    
    // Filter state for high-pass filter
    float filterPrevInput;
    float filterPrevOutput;
    
    // WAV file format structures
    struct WavHeader {
        char chunkId[4] = {'R', 'I', 'F', 'F'};
        std::uint32_t chunkSize;
        char format[4] = {'W', 'A', 'V', 'E'};
        char subchunk1Id[4] = {'f', 'm', 't', ' '};
        std::uint32_t subchunk1Size = 16;
        std::uint16_t audioFormat = 1; // PCM
        std::uint16_t numChannels;
        std::uint32_t sampleRate;
        std::uint32_t byteRate;
        std::uint16_t blockAlign;
        std::uint16_t bitsPerSample = 16;
        char subchunk2Id[4] = {'d', 'a', 't', 'a'};
        std::uint32_t subchunk2Size;
    };

public:
    Recorder();
    ~Recorder();
    
    // Recording control
    bool startRecording(const std::string& filename = "");
    bool startInternalRecording(const std::string& filename = ""); // For capturing AudioManager output
    void stopRecording();
    bool isCurrentlyRecording() const;
    
    // File operations
    bool saveToFile(const std::string& filename);
    void clearBuffer();
    
    // Configuration
    void setSampleRate(unsigned int sampleRate);
    void setChannelCount(unsigned int channelCount);
    void setNoiseGate(bool enabled, float threshold = 0.01f);
    void setHighPassFilter(bool enabled, float frequency = 80.0f);
    
    // Statistics
    size_t getSampleCount() const;
    float getRecordingDuration() const; // in seconds
    
    // Audio processing
    void addSamples(const std::int16_t* sampleData, size_t sampleCount);
    void addSamples(const std::vector<std::int16_t>& newSamples);
    void addSampleAtTime(const std::int16_t* sampleData, size_t sampleCount, int sampleIndex); // Time-aware addition for internal recording
    void stopSampleAtTime(int sampleIndex); // Stop a specific sample in the recording
    
    // Get current audio data
    const std::vector<std::int16_t>& getSamples() const { return samples; }
    
    // Get current realtime audio buffer (for rhythm analysis)
    std::vector<float> getRealtimeAudioBuffer(size_t maxSamples = 512) const;

protected:
    // Inherited from sf::SoundRecorder
    virtual bool onStart() override;
    virtual bool onProcessSamples(const std::int16_t* sampleData, std::size_t sampleCount) override;
    virtual void onStop() override;

private:
    bool writeWavFile(const std::string& filename);
    WavHeader createWavHeader(std::uint32_t dataSize, std::uint32_t sampleRate, std::uint16_t channels);
    
    // Helper methods to get effective recording parameters
    unsigned int getEffectiveSampleRate() const;
    unsigned int getEffectiveChannelCount() const;
    
    // Real-time buffer management for internal recording
    void finalizeRealtimeBuffer(); // Convert real-time buffer to samples
    void mixSampleIntoBuffer(const std::int16_t* sampleData, size_t sampleCount, size_t bufferOffset);
    void stopActiveSample(int sampleIndex, size_t currentPosition); // Stop active sample and remove from buffer
    void rebuildRealtimeBuffer(size_t currentPosition); // Rebuild buffer from active samples
    size_t getRealtimeBufferSize() const; // Get current buffer size in samples
    
    // Audio processing functions
    std::int16_t applyNoiseGate(std::int16_t sample);
    std::int16_t applyHighPassFilter(std::int16_t sample);
    void processSample(std::int16_t& sample);
};
