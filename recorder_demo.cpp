// Example of using the Recorder class
// This file demonstrates how to use the Recorder for streaming audio to a file

#include "Recorder.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>

int main() {
    std::cout << "Audio Recorder Demo" << std::endl;
    std::cout << "===================" << std::endl;
    
    // Check if recording is available
    if (!sf::SoundRecorder::isAvailable()) {
        std::cerr << "Audio recording is not available on this system!" << std::endl;
        return 1;
    }
    
    Recorder recorder;
    
    // Enable gentle noise reduction
    recorder.setNoiseGate(true, 0.001f);  // Very low threshold: 0.1% (only cuts silent background)
    recorder.setHighPassFilter(true, 60.0f);  // Lower frequency: 60Hz (removes electrical hum)
    
    std::cout << "Gentle noise reduction enabled:" << std::endl;
    std::cout << "- Noise gate: 0.1% threshold (very low)" << std::endl;
    std::cout << "- High-pass filter: 60Hz (removes electrical hum)" << std::endl;
    
    std::cout << "\nPress Enter to start recording for 5 seconds..." << std::endl;
    std::cin.get();
    
    // Start recording
    if (recorder.startRecording("demo_recording.wav")) {
        std::cout << "Recording... (5 seconds)" << std::endl;
        
        // Record for 5 seconds
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        // Stop recording
        recorder.stopRecording();
        
        std::cout << "Recording complete!" << std::endl;
        std::cout << "Duration: " << recorder.getRecordingDuration() << " seconds" << std::endl;
        std::cout << "Sample count: " << recorder.getSampleCount() << std::endl;
        
    } else {
        std::cerr << "Failed to start recording!" << std::endl;
        return 1;
    }
    
    // Example of manual audio data processing
    std::cout << "\nManual sample addition example:" << std::endl;
    
    Recorder manualRecorder;
    
    // Create some sample audio data (sine wave)
    std::vector<sf::Int16> sineWave;
    const int sampleRate = 44100;
    const float frequency = 440.0f; // A4 note
    const float duration = 2.0f; // 2 seconds
    
    for (int i = 0; i < static_cast<int>(sampleRate * duration); ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float amplitude = 0.3f; // 30% volume
        sf::Int16 sample = static_cast<sf::Int16>(amplitude * 32767 * sin(2 * M_PI * frequency * t));
        sineWave.push_back(sample);
    }
    
    // Add the generated samples to the recorder
    manualRecorder.addSamples(sineWave);
    
    // Save to file
    if (manualRecorder.saveToFile("generated_tone.wav")) {
        std::cout << "Generated tone saved to generated_tone.wav" << std::endl;
        std::cout << "Duration: " << manualRecorder.getRecordingDuration() << " seconds" << std::endl;
    }
    
    return 0;
}
