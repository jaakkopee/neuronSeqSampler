#include "src/Neuron.h"
#include <iostream>
#include <cassert>
#include <string>

int main() {
    std::cout << "🧪 Testing Neuron sample file path storage..." << std::endl;
    
    // Test 1: Create neuron with default (empty) sample file path
    Neuron neuron1(1, 0.0f, 1.0f, 1.0f, 0.0f, ActivationFunction::Linear);
    assert(neuron1.getSampleFilePath().empty());
    std::cout << "✓ Neuron created with empty sample path (default)" << std::endl;
    
    // Test 2: Create neuron with specific sample file path
    std::string testPath = "/home/test/samples/kick.wav";
    Neuron neuron2(2, 0.5f, 1.0f, 1.0f, 0.0f, ActivationFunction::Linear, testPath);
    assert(neuron2.getSampleFilePath() == testPath);
    std::cout << "✓ Neuron created with sample path: " << neuron2.getSampleFilePath() << std::endl;
    
    // Test 3: Test sample path modification
    std::string newPath = "/home/test/samples/snare.wav";
    neuron2.setSampleFilePath(newPath);
    assert(neuron2.getSampleFilePath() == newPath);
    std::cout << "✓ Sample path updated: " << neuron2.getSampleFilePath() << std::endl;
    
    // Test 4: Test setting empty path
    neuron2.setSampleFilePath("");
    assert(neuron2.getSampleFilePath().empty());
    std::cout << "✓ Sample path cleared successfully" << std::endl;
    
    // Test 5: Test copy constructor preserves sample path
    neuron2.setSampleFilePath(testPath);
    Neuron neuron3 = neuron2;
    assert(neuron3.getSampleFilePath() == testPath);
    std::cout << "✓ Sample path preserved in copy: " << neuron3.getSampleFilePath() << std::endl;
    
    std::cout << "🎉 All basic sample file path tests passed!" << std::endl;
    std::cout << "📝 Note: Full preset integration tested separately during build." << std::endl;
    return 0;
}
