#include "Quantizer.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iostream>

Quantizer::Quantizer(float bpm, int sampleRate)
    : currentBPM(bpm), sampleRate(sampleRate), gridResolution(GridResolution::SIXTEENTH_NOTE),
      quantizationAmount(0.8f), swingFactor(0.0f), isEnabled(true) {
    updateTimingCalculations();
    std::cout << "🎵 Quantizer: Constructor - Initial resolution: " << getGridResolutionName() 
              << " (enum: " << static_cast<int>(gridResolution) << ")" << std::endl;
}

void Quantizer::updateTimingCalculations() {
    // Calculate quarter note interval (60 seconds / BPM)
    quarterNoteInterval = 60.0 / currentBPM;
}

bool Quantizer::quantizeEvent(Event& event) {
    if (!isEnabled || quantizationAmount <= 0.0f) {
        return false;
    }
    
    // Get the nearest grid point
    double targetTime = getNearestGridPoint(event.timestamp);
    
    // Apply quantization amount (interpolate between original and quantized)
    double quantizedTime = interpolateQuantization(event.timestamp, targetTime);
    
    // Only update if there's a meaningful change (avoid floating point noise)
    if (std::abs(quantizedTime - event.timestamp) > 0.001) {
        event.timestamp = quantizedTime;
        event.isQuantized = true;
        return true;
    }
    
    return false;
}

size_t Quantizer::quantizeEvents(std::vector<Event>& events) {
    size_t quantizedCount = 0;
    
    for (auto& event : events) {
        if (quantizeEvent(event)) {
            quantizedCount++;
        }
    }
    
    return quantizedCount;
}

double Quantizer::getNextGridPoint(double timestamp) const {
    double gridInterval = getGridInterval();
    
    // Find the next grid boundary
    double gridPosition = timestamp / gridInterval;
    double nextGrid = std::ceil(gridPosition) * gridInterval;
    
    // Apply swing if we have a swing factor
    if (swingFactor != 0.0f) {
        nextGrid = applySwingToGridPoint(nextGrid, gridInterval);
    }
    
    return nextGrid;
}

double Quantizer::getPreviousGridPoint(double timestamp) const {
    double gridInterval = getGridInterval();
    
    // Find the previous grid boundary
    double gridPosition = timestamp / gridInterval;
    double prevGrid = std::floor(gridPosition) * gridInterval;
    
    // Apply swing if we have a swing factor
    if (swingFactor != 0.0f) {
        prevGrid = applySwingToGridPoint(prevGrid, gridInterval);
    }
    
    return std::max(0.0, prevGrid); // Don't go negative
}

double Quantizer::getNearestGridPoint(double timestamp) const {
    double nextGrid = getNextGridPoint(timestamp);
    double prevGrid = getPreviousGridPoint(timestamp);
    
    // Choose the closer grid point
    double nextDistance = std::abs(nextGrid - timestamp);
    double prevDistance = std::abs(timestamp - prevGrid);
    
    return (nextDistance < prevDistance) ? nextGrid : prevGrid;
}

void Quantizer::setGridResolution(GridResolution resolution) {
    std::cout << "🎵 Quantizer: setGridResolution called with: " << static_cast<int>(resolution) << std::endl;
    gridResolution = resolution;
    updateTimingCalculations();
    std::cout << "🎵 Quantizer: Grid interval is now: " << getGridInterval() << " seconds" << std::endl;
}

void Quantizer::setQuantizationAmount(float amount) {
    quantizationAmount = std::max(0.0f, std::min(1.0f, amount));
}

void Quantizer::setSwingFactor(float swing) {
    swingFactor = std::max(-1.0f, std::min(1.0f, swing));
}

void Quantizer::setBPM(float bpm) {
    currentBPM = std::max(60.0f, std::min(200.0f, bpm)); // Reasonable BPM range
    updateTimingCalculations();
}

double Quantizer::getGridInterval() const {
    float subdivisions = getSubdivisionFactor();
    return quarterNoteInterval / subdivisions;
}

std::string Quantizer::getGridResolutionName() const {
    switch (gridResolution) {
        case GridResolution::HALF_NOTE: return "1/2 Note";
        case GridResolution::QUARTER_NOTE: return "1/4 Note";
        case GridResolution::EIGHTH_NOTE: return "1/8 Note";
        case GridResolution::SIXTEENTH_NOTE: return "1/16 Note";
        case GridResolution::THIRTY_SECOND_NOTE: return "1/32 Note";
        case GridResolution::SIXTY_FOURTH_NOTE: return "1/64 Note";
        default: return "Unknown";
    }
}

void Quantizer::reset() {
    updateTimingCalculations();
}

double Quantizer::applySwing(double gridPosition) const {
    // This method is deprecated - use applySwingToGridPoint instead
    return gridPosition;
}

double Quantizer::applySwingToGridPoint(double gridPoint, double gridInterval) const {
    if (swingFactor == 0.0f) {
        return gridPoint;
    }
    
    // Calculate which grid subdivision this is within a pair (0 = on-beat, 1 = off-beat)
    double gridIndex = std::round(gridPoint / gridInterval);
    int gridInPair = static_cast<int>(gridIndex) % 2;
    
    // Only apply swing to off-beats (odd grid positions: 1, 3, 5, etc.)
    if (gridInPair == 1) {
        // For swing:
        // 0% swing = 50% of the way between beats (straight timing)
        // 100% swing = 66.67% of the way between beats (triplet timing)
        double swingAmount = 0.5 + (swingFactor * 0.1667); // 0.5 to 0.6667
        
        // Find the previous on-beat (even grid position)
        double prevOnBeat = (gridIndex - 1) * gridInterval;
        double nextOnBeat = (gridIndex + 1) * gridInterval;
        
        // Place the off-beat at the swing position between on-beats
        double swungGridPoint = prevOnBeat + (nextOnBeat - prevOnBeat) * swingAmount;
        
        std::cout << "🎵 Swing: Moving off-beat from " << gridPoint << "s to " << swungGridPoint 
                  << "s (swing factor: " << swingFactor << ", amount: " << swingAmount << ")" << std::endl;
        
        return swungGridPoint;
    }
    
    // On-beats remain unchanged
    return gridPoint;
}

float Quantizer::getSubdivisionFactor() const {
    switch (gridResolution) {
        case GridResolution::HALF_NOTE: return 0.5f; // Half the subdivision rate (longer intervals)
        case GridResolution::QUARTER_NOTE: return 1.0f;
        case GridResolution::EIGHTH_NOTE: return 2.0f;
        case GridResolution::SIXTEENTH_NOTE: return 4.0f;
        case GridResolution::THIRTY_SECOND_NOTE: return 8.0f;
        case GridResolution::SIXTY_FOURTH_NOTE: return 16.0f;
        default: return 4.0f;
    }
}

double Quantizer::interpolateQuantization(double originalTime, double quantizedTime) const {
    // Linear interpolation based on quantization amount
    return originalTime + (quantizedTime - originalTime) * quantizationAmount;
}