#include "Recorder.h"
#include <iostream>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <algorithm>

Recorder::Recorder() 
    : isRecordingToFile(false), currentlyRecording(false), isInternalRecording(false),
      recordingSampleRate(44100), recordingChannelCount(2), // Default to 44.1kHz stereo
      noiseGateEnabled(false), noiseGateThreshold(0.001f),  // Disabled by default
      highPassFilterEnabled(false), highPassFrequency(60.0f),  // Disabled by default
      filterPrevInput(0.0f), filterPrevOutput(0.0f)
{
    // Set default recording parameters
    setSampleRate(44100);
    setChannelCount(2); // Stereo by default for better quality
}

Recorder::~Recorder() {
    if (isCurrentlyRecording()) {
        stopRecording();
    }
}

bool Recorder::startRecording(const std::string& filename) {
    if (isCurrentlyRecording()) {
        std::cerr << "Already recording. Stop current recording first." << std::endl;
        return false;
    }
    
    // Check available devices and warn about dummy devices
    auto devices = sf::SoundRecorder::getAvailableDevices();
    std::string defaultDevice = sf::SoundRecorder::getDefaultDevice();
    
    if (defaultDevice.find("Dummy") != std::string::npos || 
        defaultDevice.find("Monitor") != std::string::npos) {
        std::cerr << "Warning: Default recording device appears to be a dummy/monitor device: " 
                  << defaultDevice << std::endl;
        std::cerr << "This usually means JACK is running and blocking microphone access." << std::endl;
        std::cerr << "Try stopping JACK first: ./manage_jack.sh stop" << std::endl;
    }
    
    // Clear previous samples
    clearBuffer();
    
    if (!filename.empty()) {
        outputFilename = filename;
        isRecordingToFile = true;
    } else {
        // Generate default filename with timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        std::stringstream ss;
        ss << "recording_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".wav";
        outputFilename = ss.str();
        isRecordingToFile = true;
    }
    
    // Start the SFML sound recorder
    if (sf::SoundRecorder::start()) {
        std::cout << "Started recording to: " << outputFilename << std::endl;
        return true;
    } else {
        std::cerr << "Failed to start recording. Make sure a microphone is available." << std::endl;
        isRecordingToFile = false;
        return false;
    }
}

bool Recorder::startInternalRecording(const std::string& filename) {
    if (isCurrentlyRecording()) {
        std::cerr << "Already recording. Stop current recording first." << std::endl;
        return false;
    }
    
    // Clear previous samples
    clearBuffer();
    
    if (!filename.empty()) {
        outputFilename = filename;
        isRecordingToFile = true;
    } else {
        // Generate default filename with timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        std::stringstream ss;
        ss << "internal_recording_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".wav";
        outputFilename = ss.str();
        isRecordingToFile = true;
    }
    
    // Mark as internal recording (no microphone)
    isInternalRecording = true;
    currentlyRecording = true;
    
    // Initialize timing for real-time recording
    recordingStartTime = std::chrono::steady_clock::now();
    
    // Clear and prepare the real-time buffer
    {
        std::lock_guard<std::mutex> bufferLock(realtimeBufferMutex);
        std::lock_guard<std::mutex> activeLock(activeSamplesMutex);
        realtimeBuffer.clear();
        activeSamples.clear(); // Clear any previous active sample tracking
    }
    
    std::cout << "Started internal recording to: " << outputFilename << std::endl;
    std::cout << "Capturing AudioManager output..." << std::endl;
    
    return true;
}

void Recorder::stopRecording() {
    if (!isCurrentlyRecording()) {
        return;
    }
    
    // Stop SFML recording only if not internal recording
    if (!isInternalRecording) {
        sf::SoundRecorder::stop();
    } else {
        // For internal recording, finalize the real-time buffer
        finalizeRealtimeBuffer();
    }
    
    if (isRecordingToFile && !outputFilename.empty()) {
        if (saveToFile(outputFilename)) {
            std::cout << "Recording saved to: " << outputFilename << std::endl;
            std::cout << "Duration: " << getRecordingDuration() << " seconds" << std::endl;
            std::cout << "Sample count: " << getSampleCount() << std::endl;
        } else {
            std::cerr << "Failed to save recording to: " << outputFilename << std::endl;
        }
    }
    
    isRecordingToFile = false;
    isInternalRecording = false;
    currentlyRecording = false;
}

bool Recorder::isCurrentlyRecording() const {
    return currentlyRecording;
}

bool Recorder::saveToFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(samplesMutex);
    
    if (samples.empty()) {
        std::cerr << "No audio data to save." << std::endl;
        return false;
    }
    
    return writeWavFile(filename);
}

void Recorder::clearBuffer() {
    std::lock_guard<std::mutex> samplesLock(samplesMutex);
    std::lock_guard<std::mutex> bufferLock(realtimeBufferMutex);
    std::lock_guard<std::mutex> activeLock(activeSamplesMutex);
    
    samples.clear();
    realtimeBuffer.clear();
    activeSamples.clear();
}

void Recorder::setSampleRate(unsigned int sampleRate) {
    recordingSampleRate = sampleRate; // Store for internal recording

    // Note: SFML doesn't allow setting custom sample rates directly
    // The actual sample rate will be determined by the audio device
}

void Recorder::setChannelCount(unsigned int channelCount) {
    recordingChannelCount = channelCount; // Store for internal recording
    sf::SoundRecorder::setChannelCount(channelCount);
}

void Recorder::setNoiseGate(bool enabled, float threshold) {
    noiseGateEnabled = enabled;
    noiseGateThreshold = threshold;
}

void Recorder::setHighPassFilter(bool enabled, float frequency) {
    highPassFilterEnabled = enabled;
    highPassFrequency = frequency;
    // Reset filter state
    filterPrevInput = 0.0f;
    filterPrevOutput = 0.0f;
}

size_t Recorder::getSampleCount() const {
    std::lock_guard<std::mutex> lock(samplesMutex);
    return samples.size();
}

float Recorder::getRecordingDuration() const {
    std::lock_guard<std::mutex> lock(samplesMutex);
    if (samples.empty()) return 0.0f;
    
    unsigned int sampleRate = getEffectiveSampleRate();
    unsigned int channels = getEffectiveChannelCount();
    
    return static_cast<float>(samples.size()) / (sampleRate * channels);
}

void Recorder::addSamples(const std::int16_t* sampleData, size_t sampleCount) {
    if (!sampleData || sampleCount == 0) return;
    
    std::lock_guard<std::mutex> lock(samplesMutex);
    samples.reserve(samples.size() + sampleCount);
    
    for (size_t i = 0; i < sampleCount; ++i) {
        samples.push_back(sampleData[i]);
    }
}

void Recorder::addSamples(const std::vector<std::int16_t>& newSamples) {
    if (newSamples.empty()) return;
    
    std::lock_guard<std::mutex> lock(samplesMutex);
    samples.reserve(samples.size() + newSamples.size());
    samples.insert(samples.end(), newSamples.begin(), newSamples.end());
}

void Recorder::addSampleAtTime(const std::int16_t* sampleData, size_t sampleCount, int sampleIndex) {
    if (!sampleData || sampleCount == 0 || !isInternalRecording) return;
    
    std::lock_guard<std::mutex> bufferLock(realtimeBufferMutex);
    std::lock_guard<std::mutex> activeLock(activeSamplesMutex);
    
    // Calculate current time position in samples since recording started
    auto now = std::chrono::steady_clock::now();
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - recordingStartTime).count();
    
    // Convert elapsed time to sample position
    size_t bufferPosition = (elapsedMs * recordingSampleRate * recordingChannelCount) / 1000;
    
    // Ensure buffer is large enough
    size_t requiredSize = bufferPosition + sampleCount;
    if (realtimeBuffer.size() < requiredSize) {
        realtimeBuffer.resize(requiredSize, 0.0f); // Fill with silence
    }
    
    // Create normalized sample data for tracking
    std::vector<float> normalizedData;
    normalizedData.reserve(sampleCount);
    for (size_t i = 0; i < sampleCount; ++i) {
        normalizedData.push_back(static_cast<float>(sampleData[i]) / 32767.0f);
    }
    
    // Track this as an active sample
    ActiveSample activeSample;
    activeSample.sampleIndex = sampleIndex;
    activeSample.startPosition = bufferPosition;
    activeSample.sampleLength = sampleCount;
    activeSample.sampleData = std::move(normalizedData);
    activeSample.active = true;
    activeSamples.push_back(activeSample);
    
    // Mix the new sample data into the buffer at the correct time position
    mixSampleIntoBuffer(sampleData, sampleCount, bufferPosition);
    
    std::cout << "Mixed sample " << sampleIndex << " (" << sampleCount << " samples) at position " << bufferPosition 
              << " (elapsed: " << elapsedMs << "ms)" << std::endl;
}

bool Recorder::onStart() {
    currentlyRecording = true;
    std::cout << "Recording started..." << std::endl;
    std::cout << "Sample rate: " << getEffectiveSampleRate() << " Hz" << std::endl;
    std::cout << "Channels: " << getEffectiveChannelCount() << std::endl;
    return true;
}

bool Recorder::onProcessSamples(const std::int16_t* sampleData, std::size_t sampleCount) {
    // Process and add the new samples to our buffer
    std::vector<std::int16_t> processedSamples(sampleCount);
    
    for (std::size_t i = 0; i < sampleCount; ++i) {
    std::int16_t sample = sampleData[i];
        processSample(sample);
    processedSamples[i] = sample;
    }
    
    addSamples(processedSamples.data(), sampleCount);
    
    // Continue recording
    return true;
}

void Recorder::onStop() {
    currentlyRecording = false;
    std::cout << "Recording stopped. Total samples: " << getSampleCount() << std::endl;
}

bool Recorder::writeWavFile(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }
    
    try {
        // Calculate data size
    std::uint32_t dataSize = static_cast<std::uint32_t>(samples.size() * sizeof(std::int16_t));
        
        // Create WAV header
        WavHeader header = createWavHeader(
            dataSize,
            getEffectiveSampleRate(),
            getEffectiveChannelCount()
        );
        
        // Write header
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));
        
        // Write audio data
        file.write(reinterpret_cast<const char*>(samples.data()), dataSize);
        
        file.close();
        
        std::cout << "Successfully wrote WAV file: " << filename << std::endl;
        std::cout << "File size: " << (sizeof(header) + dataSize) << " bytes" << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error writing WAV file: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

void Recorder::finalizeRealtimeBuffer() {
    std::lock_guard<std::mutex> bufferLock(realtimeBufferMutex);
    std::lock_guard<std::mutex> samplesLock(samplesMutex);
    
    // Convert float buffer to Int16 samples
    samples.clear();
    samples.reserve(realtimeBuffer.size());
    
    for (float sample : realtimeBuffer) {
        // Clamp and convert to Int16
        sample = std::max(-1.0f, std::min(1.0f, sample));
    samples.push_back(static_cast<std::int16_t>(sample * 32767.0f));
    }
    
    std::cout << "Finalized real-time buffer: " << realtimeBuffer.size() 
              << " samples converted to Int16" << std::endl;
    
    // Clear the real-time buffer
    realtimeBuffer.clear();
}

void Recorder::mixSampleIntoBuffer(const std::int16_t* sampleData, size_t sampleCount, size_t bufferOffset) {
    // Mix sample data into the real-time buffer with proper audio mixing
    for (size_t i = 0; i < sampleCount && (bufferOffset + i) < realtimeBuffer.size(); ++i) {
        float newSample = static_cast<float>(sampleData[i]) / 32767.0f;
        
        // Simple audio mixing: add samples together and clamp
        realtimeBuffer[bufferOffset + i] += newSample;
        
        // Clamp to prevent overflow
        realtimeBuffer[bufferOffset + i] = std::max(-1.0f, std::min(1.0f, realtimeBuffer[bufferOffset + i]));
    }
}

size_t Recorder::getRealtimeBufferSize() const {
    std::lock_guard<std::mutex> lock(realtimeBufferMutex);
    return realtimeBuffer.size();
}

std::vector<float> Recorder::getRealtimeAudioBuffer(size_t maxSamples) const {
    std::lock_guard<std::mutex> lock(realtimeBufferMutex);
    
    std::vector<float> audioBuffer;
    if (realtimeBuffer.empty()) {
        // Return silent buffer if no data available
        audioBuffer.resize(maxSamples, 0.0f);
        return audioBuffer;
    }
    
    // Get the most recent samples (up to maxSamples)
    size_t availableSamples = realtimeBuffer.size();
    size_t samplesToCopy = std::min(maxSamples, availableSamples);
    
    audioBuffer.resize(samplesToCopy);
    
    // Copy from the end of the buffer (most recent samples)
    std::copy(realtimeBuffer.end() - samplesToCopy, 
              realtimeBuffer.end(), 
              audioBuffer.begin());
    
    return audioBuffer;
}

unsigned int Recorder::getEffectiveSampleRate() const {
    if (isInternalRecording) {
        // For internal recording, use our stored sample rate
        return recordingSampleRate;
    } else {
        // For external recording, use SFML's sample rate
        unsigned int sampleRate = sf::SoundRecorder::getSampleRate();
        return (sampleRate > 0) ? sampleRate : recordingSampleRate; // Fallback to stored rate
    }
}

unsigned int Recorder::getEffectiveChannelCount() const {
    if (isInternalRecording) {
        // For internal recording, use our stored channel count
        return recordingChannelCount;
    } else {
        // For external recording, use SFML's channel count
        return sf::SoundRecorder::getChannelCount();
    }
}

Recorder::WavHeader Recorder::createWavHeader(std::uint32_t dataSize, std::uint32_t sampleRate, std::uint16_t channels) {
    WavHeader header;
    
    header.chunkSize = 36 + dataSize;
    header.numChannels = channels;
    header.sampleRate = sampleRate;
    header.byteRate = sampleRate * channels * (header.bitsPerSample / 8);
    header.blockAlign = channels * (header.bitsPerSample / 8);
    header.subchunk2Size = dataSize;
    
    return header;
}

std::int16_t Recorder::applyNoiseGate(std::int16_t sample) {
    if (!noiseGateEnabled) return sample;
    
    // Convert to float for processing
    float floatSample = static_cast<float>(sample) / 32767.0f;
    float absSample = std::abs(floatSample);
    
    // Apply noise gate with a much lower threshold for background noise only
    // Threshold should be around 0.001 (0.1%) to only cut very quiet background noise
    if (absSample < noiseGateThreshold) {
        return 0; // Silence below threshold
    }
    
    return sample;
}

std::int16_t Recorder::applyHighPassFilter(std::int16_t sample) {
    if (!highPassFilterEnabled) return sample;
    
    // Convert to float for processing
    float input = static_cast<float>(sample) / 32767.0f;
    
    // Simple high-pass filter (RC high-pass)
    // cutoff frequency calculation
    float sampleRate = static_cast<float>(getEffectiveSampleRate());
    float RC = 1.0f / (2.0f * M_PI * highPassFrequency);
    float dt = 1.0f / sampleRate;
    float alpha = RC / (RC + dt);
    
    // Apply filter
    float output = alpha * (filterPrevOutput + input - filterPrevInput);
    
    // Update state
    filterPrevInput = input;
    filterPrevOutput = output;
    
    // Convert back to Int16 with clipping
    float clipped = std::max(-1.0f, std::min(1.0f, output));
    return static_cast<std::int16_t>(clipped * 32767.0f);
}

void Recorder::processSample(std::int16_t& sample) {
    // Filters disabled - pass samples through unchanged
    // Apply noise gate first
    // sample = applyNoiseGate(sample);
    
    // Then apply high-pass filter  
    // sample = applyHighPassFilter(sample);
}

void Recorder::stopSampleAtTime(int sampleIndex) {
    if (!isInternalRecording) return;
    
    std::lock_guard<std::mutex> bufferLock(realtimeBufferMutex);
    std::lock_guard<std::mutex> activeLock(activeSamplesMutex);
    
    // Calculate current time position
    auto now = std::chrono::steady_clock::now();
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - recordingStartTime).count();
    size_t currentPosition = (elapsedMs * recordingSampleRate * recordingChannelCount) / 1000;
    
    stopActiveSample(sampleIndex, currentPosition);
}

void Recorder::stopActiveSample(int sampleIndex, size_t currentPosition) {
    // Find and stop any active instance of this sample
    for (auto& activeSample : activeSamples) {
        if (activeSample.sampleIndex == sampleIndex && activeSample.active) {
            size_t sampleEndPosition = activeSample.startPosition + activeSample.sampleLength;
            
            // Only process if the sample extends beyond current position
            if (currentPosition < sampleEndPosition && currentPosition >= activeSample.startPosition) {
                // Subtract this sample's contribution from the buffer
                size_t sampleOffset = currentPosition - activeSample.startPosition;
                
                for (size_t i = currentPosition; i < sampleEndPosition && i < realtimeBuffer.size(); ++i) {
                    size_t dataIndex = i - activeSample.startPosition;
                    if (dataIndex < activeSample.sampleData.size()) {
                        // Subtract this sample's contribution (proper audio mixing)
                        realtimeBuffer[i] -= activeSample.sampleData[dataIndex];
                    }
                }
                
                std::cout << "Stopped sample " << sampleIndex << " at position " << currentPosition 
                          << " (subtracted " << (sampleEndPosition - currentPosition) << " samples)" << std::endl;
            }
            
            activeSample.active = false;
            break; // Only stop one instance per call
        }
    }
    
    // Clean up inactive samples that are no longer needed
    activeSamples.erase(
        std::remove_if(activeSamples.begin(), activeSamples.end(),
            [currentPosition](const ActiveSample& sample) {
                return !sample.active && (currentPosition > sample.startPosition + sample.sampleLength);
            }),
        activeSamples.end()
    );
}

void Recorder::rebuildRealtimeBuffer(size_t currentPosition) {
    // Calculate the required buffer size based on all active samples
    size_t maxRequiredSize = currentPosition;
    for (const auto& activeSample : activeSamples) {
        if (activeSample.active) {
            size_t sampleEndPosition = activeSample.startPosition + activeSample.sampleLength;
            maxRequiredSize = std::max(maxRequiredSize, sampleEndPosition);
        }
    }
    
    // Resize buffer if needed and clear it
    if (realtimeBuffer.size() < maxRequiredSize) {
        realtimeBuffer.resize(maxRequiredSize, 0.0f);
    } else {
        // Clear the buffer (fill with silence)
        std::fill(realtimeBuffer.begin(), realtimeBuffer.end(), 0.0f);
    }
    
    // Rebuild the buffer from all active samples
    for (const auto& activeSample : activeSamples) {
        if (activeSample.active) {
            // Mix the entire active sample into the buffer
            size_t sampleEndPosition = activeSample.startPosition + activeSample.sampleLength;
            
            // Make sure we don't exceed buffer bounds
            size_t samplesToMix = std::min(activeSample.sampleLength, 
                                         realtimeBuffer.size() - activeSample.startPosition);
            
            // Mix this sample into the buffer
            for (size_t i = 0; i < samplesToMix && i < activeSample.sampleData.size(); ++i) {
                size_t bufferIndex = activeSample.startPosition + i;
                if (bufferIndex < realtimeBuffer.size()) {
                    realtimeBuffer[bufferIndex] += activeSample.sampleData[i];
                    // Clamp to prevent overflow
                    realtimeBuffer[bufferIndex] = std::max(-1.0f, std::min(1.0f, realtimeBuffer[bufferIndex]));
                }
            }
        }
    }
}
