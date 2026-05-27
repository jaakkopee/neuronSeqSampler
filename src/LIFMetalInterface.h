#pragma once

#include <string>

class LIFMetalInterface {
public:
    bool initialize(bool preferMetalBackend = true);
    void setPreferredMetalBackend(bool enabled);

    bool isMetalAvailable() const { return metalAvailable; }
    bool isUsingMetal() const { return useMetalBackend; }
    std::string backendName() const;

    float adaptLearningRate(float baseRate, float tempoBpm, float rhythmMatch) const;
    float adaptPatternBlend(float baseBlend, float tempoBpm, float rhythmMatch) const;

private:
    bool initialized = false;
    bool metalAvailable = false;
    bool useMetalBackend = false;
};
