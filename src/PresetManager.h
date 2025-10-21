#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// Forward declarations
class NeuronNetwork;
class RhythmInterpreter;
class Quantizer;

/**
 * Manages saving and loading network presets in JSON format
 * Handles neurons, connections, rhythmogram matrix settings, and tempo configuration
 */
class PresetManager {
public:
    struct PresetInfo {
        std::string name;
        std::string version;
        std::string author;
        std::string description;
        std::string created_date;
        std::string tags;
    };

    // Main preset operations
    static bool savePreset(const NeuronNetwork& network, const std::string& filename, const PresetInfo& info = {});
    static bool loadPreset(NeuronNetwork& network, const std::string& filename);
    
    // Preset discovery and management
    static std::vector<std::string> getAvailablePresets(const std::string& directory = "presets/");
    static std::vector<PresetInfo> getPresetInfos(const std::string& directory = "presets/");
    static PresetInfo getPresetInfo(const std::string& filename);
    
    // Validation and utilities
    static bool validatePreset(const std::string& filename);
    static bool createPresetDirectory();
    static std::string generatePresetPath(const std::string& name);
    
    // Factory presets
    static bool createFactoryPresets();
    static bool loadFactoryPreset(NeuronNetwork& network, const std::string& presetName);
    
    // Preset operations
    static bool deletePreset(const std::string& filename);
    static bool copyPreset(const std::string& sourceFile, const std::string& targetFile);
    
private:
    // JSON serialization helpers
    static nlohmann::json neuronToJson(const class Neuron* neuron, size_t id);
    static nlohmann::json connectionToJson(const class Connection* connection, 
                                         const std::vector<const class Neuron*>& neurons);
    static nlohmann::json rhythmogramMatrixToJson(const RhythmInterpreter* rhythmInterpreter);
    static nlohmann::json quantizationToJson(const Quantizer* quantizer);
    
    // JSON deserialization helpers
    static bool createNeuronFromJson(NeuronNetwork& network, const nlohmann::json& neuronData);
    static bool createConnectionFromJson(NeuronNetwork& network, const nlohmann::json& connectionData);
    static bool applyRhythmogramMatrixFromJson(RhythmInterpreter* rhythmInterpreter, 
                                             const nlohmann::json& matrixData);
    static bool applyQuantizationFromJson(Quantizer* quantizer, const nlohmann::json& quantizationData);
    
    // Utility functions
    static size_t findNeuronIndex(const std::vector<const class Neuron*>& neurons, const class Neuron* target);
    static std::string getCurrentDateTime();
    static bool ensureDirectoryExists(const std::string& path);
};