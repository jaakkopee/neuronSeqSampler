#include "AudioManager.h"
#include "Recorder.h"
#include "RhythmInterpreter.h"
#include <iostream>
#include <SFML/System/Time.hpp>

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
    return playSample(sampleIndex, 0.0f); // Default to no offset
}

bool AudioManager::playSample(int sampleIndex, float offsetSeconds) {
    auto it = sounds.find(sampleIndex);
    if (it != sounds.end()) {
        // Stop any previous instance of this specific sample (neuron offset behavior)
        it->second->stop();
        
        // Check if filter mode is enabled
        if (filterCallback) {
            std::cout << "🎛️  Filter callback is active - processing sample " << sampleIndex << std::endl;
            // Route through filter bank
            std::vector<float> sampleData = getSampleData(sampleIndex);
            std::cout << "🎛️  Sample data size: " << sampleData.size() << std::endl;
            if (!sampleData.empty()) {
                std::cout << "🎛️  Calling filter callback..." << std::endl;
                std::vector<float> filteredData = filterCallback(sampleData);
                std::cout << "🎛️  Filter callback returned " << filteredData.size() << " samples" << std::endl;
                
                // Create a new sound buffer with filtered data
                if (!filteredData.empty()) {
                    // Convert back to Int16
                    std::vector<sf::Int16> int16Data;
                    int16Data.reserve(filteredData.size());
                    for (float sample : filteredData) {
                        float clampedSample = std::max(-1.0f, std::min(1.0f, sample));
                        int16Data.push_back(static_cast<sf::Int16>(clampedSample * 32767.0f));
                    }
                    
                    // Create persistent buffer and play filtered audio
                    auto filteredBuffer = std::make_unique<sf::SoundBuffer>();
                    if (filteredBuffer->loadFromSamples(&int16Data[0], int16Data.size(), 1, 44100)) {
                        it->second->setBuffer(*filteredBuffer);
                        filteredBuffers[sampleIndex] = std::move(filteredBuffer); // Store buffer to keep it alive
                        it->second->setPlayingOffset(sf::Time::Zero);
                        it->second->play();
                        std::cout << "🎛️  Playing sample " << sampleIndex << " through filter bank (" << filteredData.size() << " samples)" << std::endl;
                    }
                }
            }
        } else {
            // Direct playback (original behavior)
            it->second->setPlayingOffset(sf::Time::Zero);
            it->second->play();
            std::cout << "Playing sample " << sampleIndex << " from beginning (stopped previous instance)" << std::endl;
        }
        
        // If internal recording is active, capture the sample data
        if (recordingOutput && internalRecorder) {
            std::lock_guard<std::mutex> lock(recordingMutex);
            
            // Stop any previous instance of this sample in the recording
            internalRecorder->stopSampleAtTime(sampleIndex);
            
            auto bufferIt = soundBuffers.find(sampleIndex);
            if (bufferIt != soundBuffers.end()) {
                const sf::SoundBuffer* buffer = bufferIt->second.get();
                const sf::Int16* samples = buffer->getSamples();
                std::size_t sampleCount = buffer->getSampleCount();
                
                // Add the sample data with timing information and sample index
                internalRecorder->addSampleAtTime(samples, sampleCount, sampleIndex);
                std::cout << "Stopped previous and added new sample " << sampleIndex 
                          << " (" << sampleCount << " samples) to internal recorder" << std::endl;
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
    std::cout << "Started internal recording of AudioManager output" << std::endl;
}

void AudioManager::stopInternalRecording() {
    std::lock_guard<std::mutex> lock(recordingMutex);
    recordingOutput = false;
    std::cout << "Stopped internal recording of AudioManager output" << std::endl;
}

bool AudioManager::isRecordingOutput() const {
    std::lock_guard<std::mutex> lock(recordingMutex);
    return recordingOutput;
}

std::vector<float> AudioManager::getSampleData(int sampleIndex) const {
    std::vector<float> data;
    auto bufferIt = soundBuffers.find(sampleIndex);
    std::cout << "🎛️  getSampleData for index " << sampleIndex << " - buffer found: " << (bufferIt != soundBuffers.end()) << std::endl;
    if (bufferIt != soundBuffers.end()) {
        const sf::SoundBuffer* buffer = bufferIt->second.get();
        const sf::Int16* samples = buffer->getSamples();
        std::size_t sampleCount = buffer->getSampleCount();
        std::cout << "🎛️  Sample count: " << sampleCount << std::endl;
        
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
        std::cout << "🎛️  Converted " << data.size() << " samples, max level: " << maxSample << std::endl;
    }
    return data;
}

void AudioManager::setFilterCallback(std::function<std::vector<float>(const std::vector<float>&)> callback) {
    if (filterCallback && callback) {
        std::cout << "⚠️  WARNING: Overriding existing filter callback!" << std::endl;
    }
    filterCallback = callback;
    if (callback) {
        std::cout << "🎛️  Filter callback ENABLED - samples will route through filter processing" << std::endl;
    } else {
        std::cout << "🎛️  Filter callback DISABLED - samples play directly" << std::endl;
    }
}

void AudioManager::setRhythmInterpreter(RhythmInterpreter* interpreter) {
    rhythmInterpreter = interpreter;
}

void AudioManager::setFilterMode(bool enabled) {
    std::cout << "🎛️  setFilterMode called with enabled=" << enabled 
              << ", rhythmInterpreter=" << (rhythmInterpreter ? "valid" : "null") << std::endl;
              
    if (enabled && rhythmInterpreter) {
        // FORCE clear any existing callback (e.g., F-key filtered callback) before setting analysis-only callback
        std::cout << "🔧  FORCE CLEARING existing callback and setting ANALYSIS-ONLY mode" << std::endl;
        
        // Directly set the analysis-only callback without using setFilterCallback to avoid the override warning
        filterCallback = [this](const std::vector<float>& audioData) -> std::vector<float> {
            std::cout << "🎛️  ANALYSIS-ONLY Lambda callback executing - analyzing audio through filter bank" << std::endl;
            // Process audio through rhythm interpreter for analysis (updates filter outputs)
            rhythmInterpreter->processAudioFrame(audioData);
            // Return original audio data for playback (not the filtered version)
            std::cout << "🎛️  ANALYSIS-ONLY Lambda callback returning original " << audioData.size() << " samples" << std::endl;
            return audioData; // Return original unfiltered audio
        };
        
        std::cout << "🎛️  ANALYSIS-ONLY Filter Mode FORCIBLY ENABLED - analyzing through RhythmInterpreter, playing original audio" << std::endl;
    } else {
        // Disable filter callback
        setFilterCallback(nullptr);
        if (enabled && !rhythmInterpreter) {
            std::cout << "⚠️  Filter Mode requested but RhythmInterpreter is null!" << std::endl;
        } else {
            std::cout << "🎛️  Filter Mode DISABLED - direct sample playback" << std::endl;
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
        std::cout << "🎛️  Adaptive Filter Mode ENABLED - analyzing through adaptive RhythmInterpreter, playing original audio" << std::endl;
    } else {
        // Disable adaptive filter callback
        setFilterCallback(nullptr);
        std::cout << "🎛️  Adaptive Filter Mode DISABLED - direct sample playback" << std::endl;
    }
}