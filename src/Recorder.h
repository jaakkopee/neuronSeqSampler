#pragma once
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <chrono>

class Recorder : public sf::SoundRecorder {
private:
    std::vector<sf::Int16> samples;
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
        sf::Uint32 chunkSize;
        char format[4] = {'W', 'A', 'V', 'E'};
        char subchunk1Id[4] = {'f', 'm', 't', ' '};
        sf::Uint32 subchunk1Size = 16;
        sf::Uint16 audioFormat = 1; // PCM
        sf::Uint16 numChannels;
        sf::Uint32 sampleRate;
        sf::Uint32 byteRate;
        sf::Uint16 blockAlign;
        sf::Uint16 bitsPerSample = 16;
        char subchunk2Id[4] = {'d', 'a', 't', 'a'};
        sf::Uint32 subchunk2Size;
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
    void addSamples(const sf::Int16* sampleData, size_t sampleCount);
    void addSamples(const std::vector<sf::Int16>& newSamples);
    void addSampleAtTime(const sf::Int16* sampleData, size_t sampleCount, int sampleIndex); // Time-aware addition for internal recording
    void stopSampleAtTime(int sampleIndex); // Stop a specific sample in the recording
    
    // Get current audio data
    const std::vector<sf::Int16>& getSamples() const { return samples; }

protected:
    // Inherited from sf::SoundRecorder
    virtual bool onStart() override;
    virtual bool onProcessSamples(const sf::Int16* sampleData, std::size_t sampleCount) override;
    virtual void onStop() override;

private:
    bool writeWavFile(const std::string& filename);
    WavHeader createWavHeader(sf::Uint32 dataSize, sf::Uint32 sampleRate, sf::Uint16 channels);
    
    // Helper methods to get effective recording parameters
    unsigned int getEffectiveSampleRate() const;
    unsigned int getEffectiveChannelCount() const;
    
    // Real-time buffer management for internal recording
    void finalizeRealtimeBuffer(); // Convert real-time buffer to samples
    void mixSampleIntoBuffer(const sf::Int16* sampleData, size_t sampleCount, size_t bufferOffset);
    void stopActiveSample(int sampleIndex, size_t currentPosition); // Stop active sample and remove from buffer
    void rebuildRealtimeBuffer(size_t currentPosition); // Rebuild buffer from active samples
    size_t getRealtimeBufferSize() const; // Get current buffer size in samples
    
    // Audio processing functions
    sf::Int16 applyNoiseGate(sf::Int16 sample);
    sf::Int16 applyHighPassFilter(sf::Int16 sample);
    void processSample(sf::Int16& sample);
};
