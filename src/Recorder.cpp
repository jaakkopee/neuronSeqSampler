#include "Recorder.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <algorithm>

Recorder::Recorder() 
    : isRecordingToFile(false), currentlyRecording(false),
      noiseGateEnabled(true), noiseGateThreshold(0.001f),  // Much lower threshold: 0.1%
      highPassFilterEnabled(true), highPassFrequency(60.0f),  // Lower frequency: 60Hz
      filterPrevInput(0.0f), filterPrevOutput(0.0f)
{
    // Set default recording parameters
    setSampleRate(44100);
    setChannelCount(1); // Mono by default
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

void Recorder::stopRecording() {
    if (!isCurrentlyRecording()) {
        return;
    }
    
    // Stop SFML recording
    sf::SoundRecorder::stop();
    
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
    std::lock_guard<std::mutex> lock(samplesMutex);
    samples.clear();
}

void Recorder::setSampleRate(unsigned int sampleRate) {
    sf::SoundRecorder::setProcessingInterval(sf::milliseconds(10));
    // Note: SFML doesn't allow setting custom sample rates directly
    // The actual sample rate will be determined by the audio device
}

void Recorder::setChannelCount(unsigned int channelCount) {
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
    
    unsigned int sampleRate = sf::SoundRecorder::getSampleRate();
    unsigned int channels = sf::SoundRecorder::getChannelCount();
    
    return static_cast<float>(samples.size()) / (sampleRate * channels);
}

void Recorder::addSamples(const sf::Int16* sampleData, size_t sampleCount) {
    if (!sampleData || sampleCount == 0) return;
    
    std::lock_guard<std::mutex> lock(samplesMutex);
    samples.reserve(samples.size() + sampleCount);
    
    for (size_t i = 0; i < sampleCount; ++i) {
        samples.push_back(sampleData[i]);
    }
}

void Recorder::addSamples(const std::vector<sf::Int16>& newSamples) {
    if (newSamples.empty()) return;
    
    std::lock_guard<std::mutex> lock(samplesMutex);
    samples.reserve(samples.size() + newSamples.size());
    samples.insert(samples.end(), newSamples.begin(), newSamples.end());
}

bool Recorder::onStart() {
    currentlyRecording = true;
    std::cout << "Recording started..." << std::endl;
    std::cout << "Sample rate: " << sf::SoundRecorder::getSampleRate() << " Hz" << std::endl;
    std::cout << "Channels: " << sf::SoundRecorder::getChannelCount() << std::endl;
    return true;
}

bool Recorder::onProcessSamples(const sf::Int16* sampleData, std::size_t sampleCount) {
    // Process and add the new samples to our buffer
    std::vector<sf::Int16> processedSamples(sampleCount);
    
    for (std::size_t i = 0; i < sampleCount; ++i) {
        sf::Int16 sample = sampleData[i];
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
        sf::Uint32 dataSize = static_cast<sf::Uint32>(samples.size() * sizeof(sf::Int16));
        
        // Create WAV header
        WavHeader header = createWavHeader(
            dataSize,
            sf::SoundRecorder::getSampleRate(),
            sf::SoundRecorder::getChannelCount()
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

Recorder::WavHeader Recorder::createWavHeader(sf::Uint32 dataSize, sf::Uint32 sampleRate, sf::Uint16 channels) {
    WavHeader header;
    
    header.chunkSize = 36 + dataSize;
    header.numChannels = channels;
    header.sampleRate = sampleRate;
    header.byteRate = sampleRate * channels * (header.bitsPerSample / 8);
    header.blockAlign = channels * (header.bitsPerSample / 8);
    header.subchunk2Size = dataSize;
    
    return header;
}

sf::Int16 Recorder::applyNoiseGate(sf::Int16 sample) {
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

sf::Int16 Recorder::applyHighPassFilter(sf::Int16 sample) {
    if (!highPassFilterEnabled) return sample;
    
    // Convert to float for processing
    float input = static_cast<float>(sample) / 32767.0f;
    
    // Simple high-pass filter (RC high-pass)
    // cutoff frequency calculation
    float sampleRate = static_cast<float>(sf::SoundRecorder::getSampleRate());
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
    return static_cast<sf::Int16>(clipped * 32767.0f);
}

void Recorder::processSample(sf::Int16& sample) {
    // Apply noise gate first
    sample = applyNoiseGate(sample);
    
    // Then apply high-pass filter
    sample = applyHighPassFilter(sample);
}
