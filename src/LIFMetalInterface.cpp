#include "LIFMetalInterface.h"

#include <algorithm>

namespace {
float clamp01(float v) {
    return std::max(0.0f, std::min(1.0f, v));
}
}

bool LIFMetalInterface::initialize(bool preferMetalBackend) {
    initialized = true;
#if defined(__APPLE__)
    metalAvailable = true;
#else
    metalAvailable = false;
#endif
    useMetalBackend = preferMetalBackend && metalAvailable;
    return true;
}

void LIFMetalInterface::setPreferredMetalBackend(bool enabled) {
    if (!initialized) {
        initialize(enabled);
        return;
    }
    useMetalBackend = enabled && metalAvailable;
}

std::string LIFMetalInterface::backendName() const {
    if (useMetalBackend) return "Metal";
    return "CPU";
}

float LIFMetalInterface::adaptLearningRate(float baseRate, float tempoBpm, float rhythmMatch) const {
    float tempoNorm = clamp01(tempoBpm / 180.0f);
    float rhythmNorm = clamp01(rhythmMatch);
    float tempoGain = 0.65f + 0.35f * tempoNorm;
    float rhythmGain = 0.55f + 0.45f * rhythmNorm;
    float backendGain = useMetalBackend ? 1.10f : 1.0f;
    return baseRate * tempoGain * rhythmGain * backendGain;
}

float LIFMetalInterface::adaptPatternBlend(float baseBlend, float tempoBpm, float rhythmMatch) const {
    float blend = clamp01(baseBlend);
    float tempoNorm = clamp01(tempoBpm / 180.0f);
    float rhythmNorm = clamp01(rhythmMatch);
    float adaptiveBlend = blend * (0.75f + 0.25f * tempoNorm) + 0.15f * rhythmNorm;
    return clamp01(adaptiveBlend);
}
