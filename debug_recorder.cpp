// Debug version of recorder to identify audio issues
#include "src/Recorder.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Debug Audio Recorder" << std::endl;
    std::cout << "===================" << std::endl;
    
    // Check if recording is available
    if (!sf::SoundRecorder::isAvailable()) {
        std::cerr << "Audio recording is not available on this system!" << std::endl;
        return 1;
    }
    
    std::cout << "Audio recording is available" << std::endl;
    
    // Get available devices
    auto devices = sf::SoundRecorder::getAvailableDevices();
    std::cout << "Available recording devices:" << std::endl;
    for (size_t i = 0; i < devices.size(); ++i) {
        std::cout << "  " << i << ": " << devices[i] << std::endl;
    }
    
    std::cout << "Default device: " << sf::SoundRecorder::getDefaultDevice() << std::endl;
    
    Recorder recorder;
    
    std::cout << "\nPress Enter to start recording for 3 seconds..." << std::endl;
    std::cin.get();
    
    // Start recording
    if (recorder.startRecording("debug_recording.wav")) {
        std::cout << "Recording started successfully..." << std::endl;
        
        // Record for 3 seconds, checking sample count periodically
        for (int i = 0; i < 30; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "Sample count: " << recorder.getSampleCount() << std::endl;
        }
        
        // Stop recording
        recorder.stopRecording();
        
        std::cout << "Recording complete!" << std::endl;
        std::cout << "Final sample count: " << recorder.getSampleCount() << std::endl;
        std::cout << "Duration: " << recorder.getRecordingDuration() << " seconds" << std::endl;
        
    } else {
        std::cerr << "Failed to start recording!" << std::endl;
        return 1;
    }
    
    return 0;
}
