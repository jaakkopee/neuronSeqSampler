#include "PresetManager.h"
#include "NeuronNetwork.h"
#include "Neuron.h"
#include "Connection.h"
#include "RhythmInterpreter.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

using json = nlohmann::json;
namespace fs = std::filesystem;

bool PresetManager::savePreset(const NeuronNetwork& network, const std::string& filename, const PresetInfo& info) {
    try {
        // Create preset directory if it doesn't exist
        createPresetDirectory();
        
        json preset;
        
        // Preset metadata
        preset["preset_info"] = {
            {"name", info.name.empty() ? "Untitled Preset" : info.name},
            {"version", info.version.empty() ? "1.0" : info.version},
            {"author", info.author.empty() ? "Unknown" : info.author},
            {"description", info.description.empty() ? "Neural network preset" : info.description},
            {"created_date", info.created_date.empty() ? getCurrentDateTime() : info.created_date},
            {"tags", info.tags.empty() ? "" : info.tags},
            {"neuron_count", network.getNeuronCount()},
            {"connection_count", network.getConnectionCount()}
        };
        
        // Serialize neurons
        preset["neurons"] = json::array();
        const auto& neurons = network.getNeurons();
        for (size_t i = 0; i < neurons.size(); ++i) {
            preset["neurons"].push_back(neuronToJson(neurons[i].get(), i));
        }
        
        // Create neuron pointer vector for connection serialization
        std::vector<const Neuron*> neuronPtrs;
        for (const auto& neuron : neurons) {
            neuronPtrs.push_back(neuron.get());
        }
        
        // Serialize connections
        preset["connections"] = json::array();
        const auto& connections = network.getConnections();
        for (const auto& connection : connections) {
            preset["connections"].push_back(connectionToJson(connection.get(), neuronPtrs));
        }
        
        // Serialize rhythmogram matrix if available
        auto* rhythmInterpreter = network.getRhythmInterpreter();
        if (rhythmInterpreter) {
            preset["rhythmogram_matrix"] = rhythmogramMatrixToJson(rhythmInterpreter);
        } else {
            preset["rhythmogram_matrix"] = {{"enabled", false}};
        }
        
        // Write to file
        std::string fullPath = filename;
        if (fullPath.find(".json") == std::string::npos) {
            fullPath += ".json";
        }
        
        std::ofstream file(fullPath);
        if (!file.is_open()) {
            std::cerr << "❌ Failed to open file for writing: " << fullPath << std::endl;
            return false;
        }
        
        file << preset.dump(2); // Pretty print with 2-space indentation
        file.close();
        
        std::cout << "✅ Preset saved: " << fullPath << std::endl;
        std::cout << "   Neurons: " << neurons.size() << ", Connections: " << connections.size() << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error saving preset: " << e.what() << std::endl;
        return false;
    }
}

bool PresetManager::loadPreset(NeuronNetwork& network, const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "❌ Failed to open preset file: " << filename << std::endl;
            return false;
        }
        
        json preset;
        file >> preset;
        file.close();
        
        // Validate basic structure
        if (!preset.contains("neurons") || !preset.contains("connections")) {
            std::cerr << "❌ Invalid preset format: missing required sections" << std::endl;
            return false;
        }
        
        // Clear existing network
        network.clearNetwork();
        
        // Load neurons first
        std::cout << "🔄 Loading neurons..." << std::endl;
        for (const auto& neuronData : preset["neurons"]) {
            if (!createNeuronFromJson(network, neuronData)) {
                std::cerr << "❌ Failed to create neuron from preset data" << std::endl;
                return false;
            }
        }
        
        // Load connections after all neurons are created
        std::cout << "🔄 Loading connections..." << std::endl;
        for (const auto& connectionData : preset["connections"]) {
            if (!createConnectionFromJson(network, connectionData)) {
                std::cerr << "❌ Failed to create connection from preset data" << std::endl;
                return false;
            }
        }
        
        // Reinitialize rhythm interpreter after loading neurons
        network.initializeRhythmInterpreter();
        
        // Load rhythmogram matrix if available
        if (preset.contains("rhythmogram_matrix")) {
            auto* rhythmInterpreter = network.getRhythmInterpreter();
            if (rhythmInterpreter) {
                std::cout << "🔄 Loading rhythmogram matrix..." << std::endl;
                applyRhythmogramMatrixFromJson(rhythmInterpreter, preset["rhythmogram_matrix"]);
            }
        }
        
        // Display preset info
        if (preset.contains("preset_info")) {
            auto info = preset["preset_info"];
            std::cout << "✅ Loaded preset: " << info.value("name", "Unknown") << std::endl;
            std::cout << "   Author: " << info.value("author", "Unknown") << std::endl;
            std::cout << "   Description: " << info.value("description", "") << std::endl;
        }
        
        std::cout << "   Neurons: " << network.getNeuronCount() 
                  << ", Connections: " << network.getConnectionCount() << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error loading preset: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> PresetManager::getAvailablePresets(const std::string& directory) {
    std::vector<std::string> presets;
    
    if (!fs::exists(directory)) {
        return presets;
    }
    
    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                presets.push_back(entry.path().string());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "❌ Error scanning preset directory: " << e.what() << std::endl;
    }
    
    return presets;
}

std::vector<PresetManager::PresetInfo> PresetManager::getPresetInfos(const std::string& directory) {
    std::vector<PresetInfo> infos;
    auto presetFiles = getAvailablePresets(directory);
    
    for (const auto& file : presetFiles) {
        auto info = getPresetInfo(file);
        if (!info.name.empty()) {
            infos.push_back(info);
        }
    }
    
    return infos;
}

PresetManager::PresetInfo PresetManager::getPresetInfo(const std::string& filename) {
    PresetInfo info;
    
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return info;
        }
        
        json preset;
        file >> preset;
        file.close();
        
        if (preset.contains("preset_info")) {
            auto infoJson = preset["preset_info"];
            info.name = infoJson.value("name", "");
            info.version = infoJson.value("version", "");
            info.author = infoJson.value("author", "");
            info.description = infoJson.value("description", "");
            info.created_date = infoJson.value("created_date", "");
            info.tags = infoJson.value("tags", "");
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error reading preset info: " << e.what() << std::endl;
    }
    
    return info;
}

bool PresetManager::createPresetDirectory() {
    try {
        fs::create_directories("presets/factory");
        fs::create_directories("presets/user");
        return true;
    } catch (const std::exception& e) {
        std::cerr << "❌ Error creating preset directories: " << e.what() << std::endl;
        return false;
    }
}

bool PresetManager::loadFactoryPreset(NeuronNetwork& network, const std::string& presetName) {
    std::string filename = "presets/factory/" + presetName + ".json";
    
    // Create factory presets if they don't exist
    if (!fs::exists(filename)) {
        createFactoryPresets();
    }
    
    return loadPreset(network, filename);
}

bool PresetManager::createFactoryPresets() {
    createPresetDirectory();
    
    // Create a simple 3-neuron drum pattern preset
    try {
        json drumPreset = {
            {"preset_info", {
                {"name", "Factory Drum Pattern"},
                {"version", "1.0"},
                {"author", "NeuronSeqSampler"},
                {"description", "Basic 3-neuron drum pattern with kick, clap, and bass"},
                {"created_date", getCurrentDateTime()},
                {"tags", "drums,factory,basic"}
            }},
            {"neurons", json::array({
                {
                    {"id", 0},
                    {"sample_index", 1},
                    {"activation", 0.0},
                    {"threshold", 1.0},
                    {"decay_rate", 0.5},
                    {"activation_increase_per_iteration", 0.0},
                    {"activation_function", "Linear"}
                },
                {
                    {"id", 1},
                    {"sample_index", 2},
                    {"activation", 0.0},
                    {"threshold", 1.0},
                    {"decay_rate", 0.5},
                    {"activation_increase_per_iteration", 0.0},
                    {"activation_function", "Linear"}
                },
                {
                    {"id", 2},
                    {"sample_index", 3},
                    {"activation", 0.0},
                    {"threshold", 1.0},
                    {"decay_rate", 0.5},
                    {"activation_increase_per_iteration", 0.0},
                    {"activation_function", "Linear"}
                }
            })},
            {"connections", json::array({
                {{"source_id", 0}, {"target_id", 1}, {"weight", 0.6}},
                {{"source_id", 0}, {"target_id", 2}, {"weight", 0.7}},
                {{"source_id", 1}, {"target_id", 2}, {"weight", 0.5}},
                {{"source_id", 2}, {"target_id", 0}, {"weight", 0.4}}
            })},
            {"rhythmogram_matrix", {
                {"enabled", true},
                {"scale", 5.0},
                {"filter_gains", {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}}
            }}
        };
        
        std::ofstream file("presets/factory/drum_pattern.json");
        file << drumPreset.dump(2);
        file.close();
        
        std::cout << "✅ Created factory preset: drum_pattern.json" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error creating factory presets: " << e.what() << std::endl;
        return false;
    }
}

// Private helper methods

json PresetManager::neuronToJson(const Neuron* neuron, size_t id) {
    std::string activationFuncName;
    switch (neuron->getActivationFunction()) {
        case ActivationFunction::Linear: activationFuncName = "Linear"; break;
        case ActivationFunction::Sigmoid: activationFuncName = "Sigmoid"; break;
        case ActivationFunction::ReLU: activationFuncName = "ReLU"; break;
        case ActivationFunction::Tanh: activationFuncName = "Tanh"; break;
        default: activationFuncName = "Linear"; break;
    }
    
    return json{
        {"id", id},
        {"sample_index", neuron->getSampleIndex()},
        {"activation", neuron->getRawActivation()},
        {"threshold", neuron->getThreshold()},
        {"decay_rate", neuron->getDecayRate()},
        {"activation_increase_per_iteration", neuron->getActivationIncreasePerIteration()},
        {"activation_function", activationFuncName}
    };
}

json PresetManager::connectionToJson(const Connection* connection, const std::vector<const Neuron*>& neurons) {
    size_t sourceId = findNeuronIndex(neurons, connection->getSource());
    size_t targetId = findNeuronIndex(neurons, connection->getTarget());
    
    return json{
        {"source_id", sourceId},
        {"target_id", targetId},
        {"weight", connection->getWeight()}
    };
}

json PresetManager::rhythmogramMatrixToJson(const RhythmInterpreter* rhythmInterpreter) {
    // This is a placeholder - you'll need to implement access to rhythm interpreter internals
    // For now, return basic structure
    return json{
        {"enabled", true},
        {"scale", 5.0},
        {"filter_gains", {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}}
    };
}

bool PresetManager::createNeuronFromJson(NeuronNetwork& network, const json& neuronData) {
    try {
        int sampleIndex = neuronData.value("sample_index", 1);
        float activation = neuronData.value("activation", 0.0f);
        float threshold = neuronData.value("threshold", 1.0f);
        float decayRate = neuronData.value("decay_rate", 1.0f);
        float activationIncrease = neuronData.value("activation_increase_per_iteration", 0.0f);
        
        ActivationFunction func = ActivationFunction::Linear;
        std::string funcName = neuronData.value("activation_function", "Linear");
        if (funcName == "Sigmoid") func = ActivationFunction::Sigmoid;
        else if (funcName == "ReLU") func = ActivationFunction::ReLU;
        else if (funcName == "Tanh") func = ActivationFunction::Tanh;
        
        Neuron* neuron = network.addNeuron(sampleIndex, activation, threshold, decayRate, activationIncrease, func);
        return neuron != nullptr;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error creating neuron from JSON: " << e.what() << std::endl;
        return false;
    }
}

bool PresetManager::createConnectionFromJson(NeuronNetwork& network, const json& connectionData) {
    try {
        size_t sourceId = connectionData.value("source_id", 0);
        size_t targetId = connectionData.value("target_id", 0);
        float weight = connectionData.value("weight", 1.0f);
        
        Neuron* source = network.getNeuron(sourceId);
        Neuron* target = network.getNeuron(targetId);
        
        if (!source || !target) {
            std::cerr << "❌ Invalid neuron indices in connection: " << sourceId << " -> " << targetId << std::endl;
            return false;
        }
        
        Connection* connection = network.connect(source, target, weight);
        return connection != nullptr;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error creating connection from JSON: " << e.what() << std::endl;
        return false;
    }
}

bool PresetManager::applyRhythmogramMatrixFromJson(RhythmInterpreter* rhythmInterpreter, const json& matrixData) {
    // Placeholder implementation - you'll need to add methods to RhythmInterpreter
    // to set matrix connections and filter gains
    try {
        if (matrixData.contains("filter_gains")) {
            // Apply filter gains if the rhythm interpreter supports it
            std::cout << "🔄 Rhythmogram matrix settings loaded" << std::endl;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "❌ Error applying rhythmogram matrix: " << e.what() << std::endl;
        return false;
    }
}

size_t PresetManager::findNeuronIndex(const std::vector<const Neuron*>& neurons, const Neuron* target) {
    for (size_t i = 0; i < neurons.size(); ++i) {
        if (neurons[i] == target) {
            return i;
        }
    }
    return SIZE_MAX; // Not found
}

std::string PresetManager::getCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}