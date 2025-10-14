#include "AudioManager.h"
#include "Recorder.h"
#include "RhythmInterpreter.h"
#include "Debug.h"
#include <iostream>
#include <SFML/System/Time.hpp>
#include <cstdint>

AudioManager::AudioManager(const std::string& samplesDir, bool loadDefaults) 
    : samplesDirectory(samplesDir), internalRecorder(nullptr), recordingOutput(false), rhythmInterpreter(nullptr)
{
    if (loadDefaults) {
        loadDefaultSamples();
    }
}

bool AudioManager::loadSample(int sampleIndex, const std::string& filename) {
    std::string fullPath = samplesDirectory + filename;
    return loadSampleFromPath(sampleIndex, fullPath);
}

bool AudioManager::loadSampleFromPath(int sampleIndex, const std::string& fullPath) {
    auto buffer = std::make_unique<sf::SoundBuffer>();
    if (!buffer->loadFromFile(fullPath)) {
        std::cerr << "Failed to load audio file: " << fullPath << std::endl;
        return false;
    }
    auto sound = std::make_unique<sf::Sound>(*buffer);
    soundBuffers[sampleIndex] = std::move(buffer);
    sounds[sampleIndex] = std::move(sound);
    DEBUG_PRINT_STREAM("Loaded sample " << sampleIndex << " from " << fullPath);
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
    return playSample(sampleIndex, 0.0f); // Default to no offset
}

bool AudioManager::playSample(int sampleIndex, float offsetSeconds) {
    return playSample(sampleIndex, offsetSeconds, 100.0f); // Default to full volume
}

bool AudioManager::playSample(int sampleIndex, float offsetSeconds, float volume) {
    auto it = sounds.find(sampleIndex);
    if (it != sounds.end()) {
        // Stop any previous instance of this specific sample (neuron offset behavior)
        it->second->stop();
        
        // Set volume based on activation function result
        float clampedVolume = std::max(0.0f, std::min(100.0f, volume));
        it->second->setVolume(clampedVolume);
        
        // Always play audio directly (no filtering)
        it->second->setPlayingOffset(sf::Time::Zero);
        it->second->play();
        DEBUG_PRINT_STREAM("🔊 Playing sample " << sampleIndex << " directly (volume: " << clampedVolume << "%)");
        
        // Run rhythmogram analysis separately (if enabled) without affecting audio playback
        if (rhythmInterpreter) {
            std::vector<float> sampleData = getSampleData(sampleIndex);
            if (!sampleData.empty()) {
                DEBUG_PRINT_STREAM("🎛️ Running rhythmogram analysis on sample " << sampleIndex << " (" << sampleData.size() << " samples)");
                rhythmInterpreter->processAudioFrame(sampleData);
                DEBUG_PRINT("🎛️ Rhythmogram analysis complete - data available for neuron activation");
            }
        }
        
        // Handle F-key filtered audio output (if enabled via F key)
        if (filterCallback) {
            DEBUG_PRINT("🔥 F-key filter active - outputting filtered audio");
            std::vector<float> sampleData = getSampleData(sampleIndex);
            if (!sampleData.empty()) {
                std::vector<float> filteredData = filterCallback(sampleData);
                
                // Create a new sound buffer with filtered data and play it
                if (!filteredData.empty()) {
                    std::vector<std::int16_t> int16Data;
                    int16Data.reserve(filteredData.size());
                    for (float sample : filteredData) {
                        float clampedSample = std::max(-1.0f, std::min(1.0f, sample));
                        int16Data.push_back(static_cast<std::int16_t>(clampedSample * 32767.0f));
                    }
                    
                    auto filteredBuffer = std::make_unique<sf::SoundBuffer>();
                    std::vector<sf::SoundChannel> channelMap; // Empty for mono
                    if (filteredBuffer->loadFromSamples(reinterpret_cast<const std::int16_t*>(&int16Data[0]), int16Data.size(), 1, 44100, channelMap)) {
                        // Stop direct audio and play filtered version instead
                        it->second->stop();
                        it->second = std::make_unique<sf::Sound>(*filteredBuffer);
                        filteredBuffers[sampleIndex] = std::move(filteredBuffer);
                        it->second->setPlayingOffset(sf::Time::Zero);
                        it->second->play();
                        DEBUG_PRINT("🔥 Playing filtered audio instead of direct sample");
                    }
                }
            }
        }
        
        // If internal recording is active, capture the sample data
        if (recordingOutput && internalRecorder) {
            std::lock_guard<std::mutex> lock(recordingMutex);
            
            // Stop any previous instance of this sample in the recording
            internalRecorder->stopSampleAtTime(sampleIndex);
            
            auto bufferIt = soundBuffers.find(sampleIndex);
            if (bufferIt != soundBuffers.end()) {
                const sf::SoundBuffer* buffer = bufferIt->second.get();
                const std::int16_t* samples = reinterpret_cast<const std::int16_t*>(buffer->getSamples());
                std::size_t sampleCount = buffer->getSampleCount();
                // Add the sample data with timing information and sample index
                internalRecorder->addSampleAtTime(samples, sampleCount, sampleIndex);
                DEBUG_PRINT_STREAM("Stopped previous and added new sample " << sampleIndex 
                          << " (" << sampleCount << " samples) to internal recorder");
            }
        }
        
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

void AudioManager::setInternalRecorder(Recorder* recorder) {
    std::lock_guard<std::mutex> lock(recordingMutex);
    internalRecorder = recorder;
}

void AudioManager::startInternalRecording() {
    std::lock_guard<std::mutex> lock(recordingMutex);
    recordingOutput = true;
    ESSENTIAL_PRINT("Started internal recording of AudioManager output");
}

void AudioManager::stopInternalRecording() {
    std::lock_guard<std::mutex> lock(recordingMutex);
    recordingOutput = false;
    ESSENTIAL_PRINT("Stopped internal recording of AudioManager output");
}

bool AudioManager::isRecordingOutput() const {
    std::lock_guard<std::mutex> lock(recordingMutex);
    return recordingOutput;
}

std::vector<float> AudioManager::getSampleData(int sampleIndex) const {
    std::vector<float> data;
    auto bufferIt = soundBuffers.find(sampleIndex);
    DEBUG_PRINT_STREAM("🎛️  getSampleData for index " << sampleIndex << " - buffer found: " << (bufferIt != soundBuffers.end()));
    if (bufferIt != soundBuffers.end()) {
    const sf::SoundBuffer* buffer = bufferIt->second.get();
    const std::int16_t* samples = reinterpret_cast<const std::int16_t*>(buffer->getSamples());
    std::size_t sampleCount = buffer->getSampleCount();
        DEBUG_PRINT_STREAM("🎛️  Sample count: " << sampleCount);
        
        // Convert Int16 samples to float
        data.reserve(sampleCount);
        for (std::size_t i = 0; i < sampleCount; ++i) {
            data.push_back(static_cast<float>(samples[i]) / 32767.0f);
        }
        
        // Check if we have actual audio data
        float maxSample = 0.0f;
        for (float sample : data) {
            maxSample = std::max(maxSample, std::abs(sample));
        }
        DEBUG_PRINT_STREAM("🎛️  Converted " << data.size() << " samples, max level: " << maxSample);
    }
    return data;
}

void AudioManager::setFilterCallback(std::function<std::vector<float>(const std::vector<float>&)> callback) {
    if (filterCallback && callback) {
        DEBUG_PRINT("⚠️  WARNING: Overriding existing filter callback!");
    }
    filterCallback = callback;
    if (callback) {
        DEBUG_PRINT("🎛️  Filter callback ENABLED - samples will route through filter processing");
    } else {
        DEBUG_PRINT("🎛️  Filter callback DISABLED - samples play directly");
    }
}

void AudioManager::setRhythmInterpreter(RhythmInterpreter* interpreter) {
    rhythmInterpreter = interpreter;
}

void AudioManager::setFilterMode(bool enabled) {
    DEBUG_PRINT_STREAM("🎛️  setRhythmAnalysis called with enabled=" << enabled 
              << ", rhythmInterpreter=" << (rhythmInterpreter ? "valid" : "null"));
              
    // Clear any filter callback to ensure direct audio playback
    setFilterCallback(nullptr);
    
    if (enabled && rhythmInterpreter) {
        DEBUG_PRINT("🎛️  RHYTHMOGRAM ANALYSIS ENABLED - audio plays directly, analysis runs separately");
    } else {
        if (enabled && !rhythmInterpreter) {
            DEBUG_PRINT("⚠️  Rhythmogram analysis requested but RhythmInterpreter is null!");
        } else {
            DEBUG_PRINT("🎛️  Rhythmogram analysis DISABLED");
        }
    }
}

void AudioManager::setAdaptiveFilterMode(bool enabled) {
    if (enabled && rhythmInterpreter) {
        // Set up adaptive filter callback to analyze audio with adaptation but play original
        setFilterCallback([this](const std::vector<float>& audioData) -> std::vector<float> {
            // Process audio through rhythm interpreter with adaptive filtering for analysis
            rhythmInterpreter->processAudioFrame(audioData);
            // Return original audio data for playback (not the filtered version)
            return audioData; // Return original unfiltered audio
        });
        DEBUG_PRINT("🎛️  Adaptive Filter Mode ENABLED - analyzing through adaptive RhythmInterpreter, playing original audio");
    } else {
        // Disable adaptive filter callback
        setFilterCallback(nullptr);
        DEBUG_PRINT("🎛️  Adaptive Filter Mode DISABLED - direct sample playback");
    }
}