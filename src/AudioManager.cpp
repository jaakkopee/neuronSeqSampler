#include "AudioManager.h"
#include "Recorder.h"
#include "RhythmInterpreter.h"
#include "Debug.h"
#include <iostream>
#include <SFML/System/Time.hpp>
#include <SFML/Audio.hpp>
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
    sampleVolumes[sampleIndex] = 1.0f;  // Default volume
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
        
        // Set volume based on activation function result, sample volume, and master volume
        float sampleVol = sampleVolumes.count(sampleIndex) ? sampleVolumes[sampleIndex] : 1.0f;
        float finalVolume = volume * sampleVol * masterVolume;
        float clampedVolume = std::max(0.0f, std::min(200.0f, finalVolume));
        it->second->setVolume(clampedVolume);
        
        // Always play audio directly (no filtering)
        it->second->setPlayingOffset(sf::Time::Zero);
        it->second->play();
        DEBUG_PRINT_STREAM("🔊 Playing sample " << sampleIndex << " directly (volume: " << clampedVolume << "%)");
        
        // Mixed output analysis is now done in updateMixedAudioAnalysis() called from main loop
        // This allows capturing the complete mixed output instead of individual samples
        
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
        setFilterCallback([this](const std::vector<float>& audioData) -> std::vector<float> {
            rhythmInterpreter->processAudioFrame(audioData);
            return audioData;
        });
        DEBUG_PRINT("🎛️  Adaptive Filter Mode ENABLED - analyzing through adaptive RhythmInterpreter, playing original audio");
    } else {
        setFilterCallback(nullptr);
        DEBUG_PRINT("🎛️  Adaptive Filter Mode DISABLED - direct sample playback");
    }
}

// ========================= External Input Soundfile Support =========================

bool AudioManager::loadInputFile(const std::string& fullPath) {
    inputBuffer = std::make_unique<sf::SoundBuffer>();
    if (!inputBuffer->loadFromFile(fullPath)) {
        std::cerr << "Failed to load input audio file: " << fullPath << std::endl;
        inputBuffer.reset();
        inputSound.reset();
        inputSamples.clear();
        inputStreaming = false;
        inputCursor = 0;
        return false;
    }

    // Prepare playback sound
    inputSound = std::make_unique<sf::Sound>(*inputBuffer);

    // Convert samples to float for analysis
    inputSamples.clear();
    const std::int16_t* samples = reinterpret_cast<const std::int16_t*>(inputBuffer->getSamples());
    std::size_t sampleCount = inputBuffer->getSampleCount();
    inputSamples.reserve(sampleCount);
    for (std::size_t i = 0; i < sampleCount; ++i) {
        inputSamples.push_back(static_cast<float>(samples[i]) / 32767.0f);
    }
    inputCursor = 0;
    std::cout << "🎵 Loaded input file for rhythmogram: " << fullPath << " (" << sampleCount << " samples)" << std::endl;
    return true;
}

void AudioManager::startInputPlayback() {
    if (inputSound) {
        inputSound->stop();
        inputSound->play();
        inputStreaming = true;
        inputCursor = 0;
        std::cout << "▶️ Started input file playback" << std::endl;
    }
}

void AudioManager::pauseInputPlayback() {
    inputStreaming = false;
    if (inputSound) {
        inputSound->pause();
        std::cout << "⏸️ Paused input file playback" << std::endl;
    }
}

void AudioManager::stopInputPlayback() {
    inputStreaming = false;
    if (inputSound) {
        inputSound->stop();
    }
    inputCursor = 0;
    std::cout << "⏹️ Stopped input file playback" << std::endl;
}

std::vector<float> AudioManager::getNextInputChunk(std::size_t chunkSize) {
    std::vector<float> chunk;
    if (inputSamples.empty() || !inputStreaming) {
        return chunk;
    }
    chunk.reserve(chunkSize);
    for (std::size_t i = 0; i < chunkSize; ++i) {
        // Loop over the buffer
        float sample = inputSamples[inputCursor];
        chunk.push_back(sample);
        inputCursor = (inputCursor + 1) % inputSamples.size();
    }

    // Keep the input file audible: if playback stopped, restart from beginning
    if (inputSound) {
        if (inputSound->getStatus() != sf::SoundSource::Status::Playing) {
            inputSound->setPlayingOffset(sf::Time::Zero);
            inputSound->play();
        }
    }
    return chunk;
}

void AudioManager::setMasterVolume(float volume) {
    masterVolume = std::max(0.0f, std::min(2.0f, volume));
}

void AudioManager::setSampleVolume(int sampleIndex, float volume) {
    sampleVolumes[sampleIndex] = std::max(0.0f, std::min(2.0f, volume));
}

float AudioManager::getSampleVolume(int sampleIndex) const {
    auto it = sampleVolumes.find(sampleIndex);
    return (it != sampleVolumes.end()) ? it->second : 1.0f;
}

void AudioManager::updateMixedAudioAnalysis() {
    if (!rhythmInterpreter || !analyzeMixedOutput) {
        return;
    }
    
    // Only analyze at the specified interval (20ms by default)
    if (analysisTimer.getElapsedTime().asSeconds() < analysisInterval) {
        return;
    }
    analysisTimer.restart();
    
    // Collect samples from all currently playing sounds
    const size_t chunkSize = 1024; // Analyze 1024 samples at a time
    mixBuffer.clear();
    mixBuffer.resize(chunkSize, 0.0f);
    
    bool hasPlayingSounds = false;
    for (auto& [index, sound] : sounds) {
        if (sound->getStatus() == sf::Sound::Playing) {
            hasPlayingSounds = true;
            auto bufferIt = soundBuffers.find(index);
            if (bufferIt != soundBuffers.end()) {
                const sf::Int16* samples = bufferIt->second->getSamples();
                sf::Uint64 sampleCount = bufferIt->second->getSampleCount();
                sf::Time offset = sound->getPlayingOffset();
                unsigned int sampleRate = bufferIt->second->getSampleRate();
                unsigned int channels = bufferIt->second->getChannelCount();
                
                // Calculate current position in samples
                sf::Uint64 currentPos = static_cast<sf::Uint64>(offset.asSeconds() * sampleRate * channels);
                
                // Mix this sound into the buffer
                float volume = sound->getVolume() / 100.0f;
                for (size_t i = 0; i < chunkSize && (currentPos + i) < sampleCount; ++i) {
                    float sample = samples[currentPos + i] / 32768.0f; // Convert Int16 to float
                    mixBuffer[i] += sample * volume;
                }
            }
        }
    }
    
    // Only process if there are actually playing sounds
    if (hasPlayingSounds && !mixBuffer.empty()) {
        rhythmInterpreter->processAudioFrame(mixBuffer);
    }
}