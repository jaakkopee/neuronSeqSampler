#include "PresetManager.h"
#include "NeuronNetwork.h"
#include "Neuron.h"
#include "Connection.h"
#include "RhythmInterpreter.h"
#include "Quantizer.h"
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
        
        // Serialize quantization settings if available
        auto* quantizer = network.getQuantizer();
        if (quantizer) {
            preset["quantization"] = quantizationToJson(quantizer);
        } else {
            preset["quantization"] = {{"enabled", false}};
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
        
        // Load quantization settings if available
        if (preset.contains("quantization")) {
            auto* quantizer = network.getQuantizer();
            if (quantizer) {
                std::cout << "🔄 Loading quantization settings..." << std::endl;
                if (!applyQuantizationFromJson(quantizer, preset["quantization"])) {
                    std::cerr << "⚠️  Failed to apply quantization settings, continuing with defaults" << std::endl;
                }
            } else {
                std::cout << "⚠️  No quantizer available to apply settings to" << std::endl;
            }
        } else {
            std::cout << "ℹ️  No quantization settings in preset (using current settings)" << std::endl;
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
    
    // Check if factory presets already exist - if any exist, don't recreate all
    std::string drumPresetPath = "presets/factory/drum_pattern.json";
    if (fs::exists(drumPresetPath)) {
        // Factory presets already exist, no need to overwrite
        return true;
    }
    
    std::cout << "🏭 Creating factory presets collection..." << std::endl;
    
    // Create 6 diverse factory presets
    try {
        // Preset 1: Basic Drum Pattern
        json basicDrums = {
            {"preset_info", {
                {"name", "Basic Drum Pattern"},
                {"version", "1.0"},
                {"author", "NeuronSeqSampler"},
                {"description", "Simple 3-neuron drum pattern - great for beginners"},
                {"created_date", getCurrentDateTime()},
                {"tags", "drums,basic,beginner"}
            }},
            {"neurons", json::array({
                {
                    {"id", 0}, {"sample_index", 1}, {"sample_file_path", "samples/kick/kick (ghost).wav"},
                    {"activation", 0.0}, {"threshold", 1.0}, {"decay_rate", 0.8},
                    {"activation_increase_per_iteration", 0.0}, {"activation_function", "Linear"}
                },
                {
                    {"id", 1}, {"sample_index", 2}, {"sample_file_path", "samples/clap/clap (ghost).wav"},
                    {"activation", 0.0}, {"threshold", 1.2}, {"decay_rate", 0.7},
                    {"activation_increase_per_iteration", 0.0}, {"activation_function", "Linear"}
                },
                {
                    {"id", 2}, {"sample_index", 3}, {"sample_file_path", "samples/808/ROBBERY 808 @prodopus.wav"},
                    {"activation", 0.0}, {"threshold", 0.8}, {"decay_rate", 0.6},
                    {"activation_increase_per_iteration", 0.0}, {"activation_function", "Linear"}
                }
            })},
            {"connections", json::array({
                {{"source_id", 0}, {"target_id", 1}, {"weight", 0.5}},
                {{"source_id", 1}, {"target_id", 2}, {"weight", 0.6}},
                {{"source_id", 2}, {"target_id", 0}, {"weight", 0.3}}
            })},
            {"quantization", {{"grid_resolution", "Sixteenth"}, {"quantization_amount", 0.8}, {"swing_factor", 0.0}, {"bpm", 120.0}}},
            {"rhythmogram_matrix", {{"enabled", true}, {"scale", 5.0}, {"filter_gains", {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}}}}
        };

        // Preset 2: Complex Network
        json complexNetwork = {
            {"preset_info", {
                {"name", "Complex Network"},
                {"version", "1.0"},
                {"author", "NeuronSeqSampler"},
                {"description", "5-neuron interconnected network with ReLU activations"},
                {"created_date", getCurrentDateTime()},
                {"tags", "complex,network,relu,advanced"}
            }},
            {"neurons", json::array({
                {
                    {"id", 0}, {"sample_index", 1}, {"sample_file_path", "samples/kick/kick (ghost).wav"},
                    {"activation", 0.0}, {"threshold", 0.9}, {"decay_rate", 0.7},
                    {"activation_increase_per_iteration", 0.1}, {"activation_function", "ReLU"}
                },
                {
                    {"id", 1}, {"sample_index", 2}, {"sample_file_path", "samples/clap/clap (ghost).wav"},
                    {"activation", 0.0}, {"threshold", 1.1}, {"decay_rate", 0.8},
                    {"activation_increase_per_iteration", 0.05}, {"activation_function", "ReLU"}
                },
                {
                    {"id", 2}, {"sample_index", 3}, {"sample_file_path", "samples/808/ROBBERY 808 @prodopus.wav"},
                    {"activation", 0.0}, {"threshold", 0.7}, {"decay_rate", 0.9},
                    {"activation_increase_per_iteration", 0.15}, {"activation_function", "Sigmoid"}
                },
                {
                    {"id", 3}, {"sample_index", 1}, {"sample_file_path", "samples/kick/kick (ghost).wav"},
                    {"activation", 0.0}, {"threshold", 1.3}, {"decay_rate", 0.6},
                    {"activation_increase_per_iteration", 0.08}, {"activation_function", "Tanh"}
                },
                {
                    {"id", 4}, {"sample_index", 2}, {"sample_file_path", "samples/clap/clap (ghost).wav"},
                    {"activation", 0.0}, {"threshold", 0.6}, {"decay_rate", 0.85},
                    {"activation_increase_per_iteration", 0.12}, {"activation_function", "ReLU"}
                }
            })},
            {"connections", json::array({
                {{"source_id", 0}, {"target_id", 1}, {"weight", 0.7}},
                {{"source_id", 1}, {"target_id", 2}, {"weight", 0.8}},
                {{"source_id", 2}, {"target_id", 3}, {"weight", 0.6}},
                {{"source_id", 3}, {"target_id", 4}, {"weight", 0.9}},
                {{"source_id", 4}, {"target_id", 0}, {"weight", 0.4}},
                {{"source_id", 0}, {"target_id", 3}, {"weight", 0.3}},
                {{"source_id", 2}, {"target_id", 4}, {"weight", 0.5}},
                {{"source_id", 1}, {"target_id", 4}, {"weight", 0.2}}
            })},
            {"quantization", {{"grid_resolution", "Eighth"}, {"quantization_amount", 0.9}, {"swing_factor", 0.0}, {"bpm", 128.0}}},
            {"rhythmogram_matrix", {{"enabled", true}, {"scale", 6.0}, {"filter_gains", {1.2, 1.0, 0.8, 1.1, 0.9, 1.0, 0.7, 1.3}}}}
        };

        // Preset 3: Sigmoid Cascade
        json sigmoidCascade = {
            {"preset_info", {
                {"name", "Sigmoid Cascade"},
                {"version", "1.0"},
                {"author", "NeuronSeqSampler"},
                {"description", "4-neuron chain with sigmoid activations for smooth dynamics"},
                {"created_date", getCurrentDateTime()},
                {"tags", "sigmoid,cascade,smooth,dynamics"}
            }},
            {"neurons", json::array({
                {
                    {"id", 0}, {"sample_index", 1}, {"sample_file_path", "samples/kick/kick (ghost).wav"},
                    {"activation", 0.0}, {"threshold", 1.0}, {"decay_rate", 0.95},
                    {"activation_increase_per_iteration", 0.2}, {"activation_function", "Sigmoid"}
                },
                {
                    {"id", 1}, {"sample_index", 2}, {"sample_file_path", "samples/clap/clap (ghost).wav"},
                    {"activation", 0.0}, {"threshold", 0.8}, {"decay_rate", 0.9},
                    {"activation_increase_per_iteration", 0.15}, {"activation_function", "Sigmoid"}
                },
                {
                    {"id", 2}, {"sample_index", 3}, {"sample_file_path", "samples/808/ROBBERY 808 @prodopus.wav"},
                    {"activation", 0.0}, {"threshold", 1.2}, {"decay_rate", 0.85},
                    {"activation_increase_per_iteration", 0.18}, {"activation_function", "Sigmoid"}
                },
                {
                    {"id", 3}, {"sample_index", 1}, {"sample_file_path", "samples/kick/kick (ghost).wav"},
                    {"activation", 0.0}, {"threshold", 0.7}, {"decay_rate", 0.8},
                    {"activation_increase_per_iteration", 0.25}, {"activation_function", "Sigmoid"}
                }
            })},
            {"connections", json::array({
                {{"source_id", 0}, {"target_id", 1}, {"weight", 0.9}},
                {{"source_id", 1}, {"target_id", 2}, {"weight", 0.8}},
                {{"source_id", 2}, {"target_id", 3}, {"weight", 0.7}},
                {{"source_id", 3}, {"target_id", 0}, {"weight", 0.6}}
            })},
            {"quantization", {{"grid_resolution", "Sixteenth"}, {"quantization_amount", 0.6}, {"swing_factor", 0.2}, {"bpm", 110.0}}},
            {"rhythmogram_matrix", {{"enabled", true}, {"scale", 4.0}, {"filter_gains", {0.8, 1.2, 0.9, 1.1, 1.0, 0.7, 1.3, 0.9}}}}
        };

        // Preset 4: High Energy
        json highEnergy = {
            {"preset_info", {
                {"name", "High Energy"},
                {"version", "1.0"},
                {"author", "NeuronSeqSampler"},
                {"description", "Fast-paced preset with low thresholds and rapid activations"},
                {"created_date", getCurrentDateTime()},
                {"tags", "high-energy,fast,rapid,intense"}
            }},
            {"neurons", json::array({
                {
                    {"id", 0}, {"sample_index", 1}, {"sample_file_path", "samples/kick/kick (ghost).wav"},
                    {"activation", 0.0}, {"threshold", 0.4}, {"decay_rate", 0.5},
                    {"activation_increase_per_iteration", 0.3}, {"activation_function", "ReLU"}
                },
                {
                    {"id", 1}, {"sample_index", 2}, {"sample_file_path", "samples/clap/clap (ghost).wav"},
                    {"activation", 0.0}, {"threshold", 0.5}, {"decay_rate", 0.6},
                    {"activation_increase_per_iteration", 0.25}, {"activation_function", "ReLU"}
                },
                {
                    {"id", 2}, {"sample_index", 3}, {"sample_file_path", "samples/808/ROBBERY 808 @prodopus.wav"},
                    {"activation", 0.0}, {"threshold", 0.3}, {"decay_rate", 0.4},
                    {"activation_increase_per_iteration", 0.35}, {"activation_function", "Linear"}
                }
            })},
            {"connections", json::array({
                {{"source_id", 0}, {"target_id", 1}, {"weight", 0.8}},
                {{"source_id", 1}, {"target_id", 2}, {"weight", 0.9}},
                {{"source_id", 2}, {"target_id", 0}, {"weight", 0.7}},
                {{"source_id", 0}, {"target_id", 2}, {"weight", 0.6}}
            })},
            {"quantization", {{"grid_resolution", "ThirtySecond"}, {"quantization_amount", 1.0}, {"swing_factor", 0.0}, {"bpm", 140.0}}},
            {"rhythmogram_matrix", {{"enabled", true}, {"scale", 8.0}, {"filter_gains", {1.5, 1.3, 1.4, 1.2, 1.1, 1.0, 1.6, 1.7}}}}
        };

        // Preset 5: Ambient Flow
        json ambientFlow = {
            {"preset_info", {
                {"name", "Ambient Flow"},
                {"version", "1.0"},
                {"author", "NeuronSeqSampler"},
                {"description", "Slow, flowing preset with Tanh activations for organic rhythms"},
                {"created_date", getCurrentDateTime()},
                {"tags", "ambient,slow,organic,tanh,chill"}
            }},
            {"neurons", json::array({
                {
                    {"id", 0}, {"sample_index", 1}, {"sample_file_path", "samples/kick/kick (ghost).wav"},
                    {"activation", 0.0}, {"threshold", 1.5}, {"decay_rate", 0.98},
                    {"activation_increase_per_iteration", 0.05}, {"activation_function", "Tanh"}
                },
                {
                    {"id", 1}, {"sample_index", 2}, {"sample_file_path", "samples/clap/clap (ghost).wav"},
                    {"activation", 0.0}, {"threshold", 1.8}, {"decay_rate", 0.96},
                    {"activation_increase_per_iteration", 0.03}, {"activation_function", "Tanh"}
                },
                {
                    {"id", 2}, {"sample_index", 3}, {"sample_file_path", "samples/808/ROBBERY 808 @prodopus.wav"},
                    {"activation", 0.0}, {"threshold", 2.0}, {"decay_rate", 0.94},
                    {"activation_increase_per_iteration", 0.08}, {"activation_function", "Tanh"}
                }
            })},
            {"connections", json::array({
                {{"source_id", 0}, {"target_id", 1}, {"weight", 0.3}},
                {{"source_id", 1}, {"target_id", 2}, {"weight", 0.4}},
                {{"source_id", 2}, {"target_id", 0}, {"weight", 0.2}}
            })},
            {"quantization", {{"grid_resolution", "Eighth"}, {"quantization_amount", 0.3}, {"swing_factor", 0.4}, {"bpm", 85.0}}},
            {"rhythmogram_matrix", {{"enabled", true}, {"scale", 2.0}, {"filter_gains", {0.5, 0.7, 0.6, 0.8, 0.9, 0.4, 0.6, 0.5}}}}
        };

        // Preset 6: Experimental
        json experimental = {
            {"preset_info", {
                {"name", "Experimental"},
                {"version", "1.0"},
                {"author", "NeuronSeqSampler"},
                {"description", "Unusual preset with mixed activation functions and complex connections"},
                {"created_date", getCurrentDateTime()},
                {"tags", "experimental,mixed,complex,unusual"}
            }},
            {"neurons", json::array({
                {
                    {"id", 0}, {"sample_index", 1}, {"sample_file_path", "samples/kick/kick (ghost).wav"},
                    {"activation", 0.0}, {"threshold", 0.9}, {"decay_rate", 0.3},
                    {"activation_increase_per_iteration", 0.4}, {"activation_function", "Linear"}
                },
                {
                    {"id", 1}, {"sample_index", 2}, {"sample_file_path", "samples/clap/clap (ghost).wav"},
                    {"activation", 0.0}, {"threshold", 1.7}, {"decay_rate", 0.1},
                    {"activation_increase_per_iteration", 0.6}, {"activation_function", "Sigmoid"}
                },
                {
                    {"id", 2}, {"sample_index", 3}, {"sample_file_path", "samples/808/ROBBERY 808 @prodopus.wav"},
                    {"activation", 0.0}, {"threshold", 0.2}, {"decay_rate", 0.9},
                    {"activation_increase_per_iteration", 0.1}, {"activation_function", "ReLU"}
                },
                {
                    {"id", 3}, {"sample_index", 1}, {"sample_file_path", "samples/kick/kick (ghost).wav"},
                    {"activation", 0.0}, {"threshold", 2.5}, {"decay_rate", 0.05},
                    {"activation_increase_per_iteration", 0.8}, {"activation_function", "Tanh"}
                }
            })},
            {"connections", json::array({
                {{"source_id", 0}, {"target_id", 2}, {"weight", 1.2}},
                {{"source_id", 1}, {"target_id", 0}, {"weight", 0.1}},
                {{"source_id", 2}, {"target_id", 3}, {"weight", 0.9}},
                {{"source_id", 3}, {"target_id", 1}, {"weight", 0.05}},
                {{"source_id", 0}, {"target_id", 1}, {"weight", 0.8}},
                {{"source_id", 2}, {"target_id", 0}, {"weight", 1.1}}
            })},
            {"quantization", {{"grid_resolution", "Sixteenth"}, {"quantization_amount", 0.5}, {"swing_factor", 0.3}, {"bpm", 95.0}}},
            {"rhythmogram_matrix", {{"enabled", true}, {"scale", 10.0}, {"filter_gains", {2.0, 0.2, 1.8, 0.3, 1.5, 0.1, 2.2, 0.4}}}}
        };

        // Save all presets
        std::vector<std::pair<std::string, json>> presets = {
            {"drum_pattern", basicDrums},
            {"complex_network", complexNetwork}, 
            {"sigmoid_cascade", sigmoidCascade},
            {"high_energy", highEnergy},
            {"ambient_flow", ambientFlow},
            {"experimental", experimental}
        };

        int created = 0;
        for (const auto& preset : presets) {
            std::string filename = "presets/factory/" + preset.first + ".json";
            std::ofstream file(filename);
            if (file.is_open()) {
                file << preset.second.dump(2);
                file.close();
                std::cout << "✅ Created factory preset: " << preset.first << ".json" << std::endl;
                created++;
            } else {
                std::cerr << "❌ Failed to create: " << preset.first << ".json" << std::endl;
            }
        }
        
        std::cout << "🎉 Factory preset creation complete: " << created << "/6 presets created" << std::endl;
        return created > 0;
        
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
        {"sample_file_path", neuron->getSampleFilePath()},
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

json PresetManager::quantizationToJson(const Quantizer* quantizer) {
    if (!quantizer) {
        return json{{"enabled", false}};
    }
    
    // Convert grid resolution enum to string for JSON
    std::string gridResolutionName;
    int gridResolutionValue = static_cast<int>(quantizer->getGridResolution());
    switch (quantizer->getGridResolution()) {
        case Quantizer::GridResolution::HALF_NOTE:
            gridResolutionName = "half_note";
            break;
        case Quantizer::GridResolution::QUARTER_NOTE:
            gridResolutionName = "quarter_note";
            break;
        case Quantizer::GridResolution::EIGHTH_NOTE:
            gridResolutionName = "eighth_note";
            break;
        case Quantizer::GridResolution::SIXTEENTH_NOTE:
            gridResolutionName = "sixteenth_note";
            break;
        case Quantizer::GridResolution::THIRTY_SECOND_NOTE:
            gridResolutionName = "thirty_second_note";
            break;
        case Quantizer::GridResolution::SIXTY_FOURTH_NOTE:
            gridResolutionName = "sixty_fourth_note";
            break;
        default:
            gridResolutionName = "quarter_note";
            gridResolutionValue = 1;
            break;
    }
    
    return json{
        {"enabled", quantizer->isQuantizationEnabled()},
        {"grid_resolution", gridResolutionName},
        {"grid_resolution_value", gridResolutionValue},
        {"quantization_amount", quantizer->getQuantizationAmount()},
        {"swing_factor", quantizer->getSwingFactor()},
        {"bpm", quantizer->getBPM()}
    };
}

bool PresetManager::createNeuronFromJson(NeuronNetwork& network, const json& neuronData) {
    try {
        int sampleIndex = neuronData.value("sample_index", 1);
        float activation = neuronData.value("activation", 0.0f);
        float threshold = neuronData.value("threshold", 1.0f);
        float decayRate = neuronData.value("decay_rate", 1.0f);
        float activationIncrease = neuronData.value("activation_increase_per_iteration", 0.0f);
        
        // Validate sample index
        if (sampleIndex <= 0 || sampleIndex > 100) { // Reasonable range for sample indices
            std::cerr << "❌ Invalid sample index: " << sampleIndex << ", using default (1)" << std::endl;
            sampleIndex = 1;
        }
        
        // Validate float values to prevent NaN or extreme values
        if (!std::isfinite(activation) || !std::isfinite(threshold) || 
            !std::isfinite(decayRate) || !std::isfinite(activationIncrease)) {
            std::cerr << "❌ Invalid float values in neuron data, using defaults" << std::endl;
            activation = 0.0f;
            threshold = 1.0f;
            decayRate = 1.0f;
            activationIncrease = 0.0f;
        }
        
        ActivationFunction func = ActivationFunction::Linear;
        std::string funcName = neuronData.value("activation_function", "Linear");
        if (funcName == "Sigmoid") func = ActivationFunction::Sigmoid;
        else if (funcName == "ReLU") func = ActivationFunction::ReLU;
        else if (funcName == "Tanh") func = ActivationFunction::Tanh;
        
        // Extract sample file path (try both field names for compatibility)
        std::string sampleFilePath = neuronData.value("sample_file_path", "");
        if (sampleFilePath.empty()) {
            sampleFilePath = neuronData.value("sample_file", "");
        }
        
        Neuron* neuron = network.addNeuron(sampleIndex, activation, threshold, decayRate, activationIncrease, func, sampleFilePath);
        return neuron != nullptr;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error creating neuron from JSON: " << e.what() << std::endl;
        return false;
    }
}

bool PresetManager::createConnectionFromJson(NeuronNetwork& network, const json& connectionData) {
    try {
        // Get the raw values first to validate them
        auto sourceIdValue = connectionData.value("source_id", 0);
        auto targetIdValue = connectionData.value("target_id", 0);
        float weight = connectionData.value("weight", 1.0f);
        
        // Convert to size_t with safety checks
        if (sourceIdValue < 0 || targetIdValue < 0) {
            std::cerr << "❌ Negative neuron indices in connection: " << sourceIdValue << " -> " << targetIdValue << std::endl;
            return false;
        }
        
        const size_t MAX_NEURON_ID = 1000; // Reasonable upper limit
        if (sourceIdValue >= MAX_NEURON_ID || targetIdValue >= MAX_NEURON_ID) {
            std::cerr << "❌ Neuron indices too large in connection: " << sourceIdValue << " -> " << targetIdValue << std::endl;
            return false;
        }
        
        size_t sourceId = static_cast<size_t>(sourceIdValue);
        size_t targetId = static_cast<size_t>(targetIdValue);
        
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

bool PresetManager::applyQuantizationFromJson(Quantizer* quantizer, const json& quantizationData) {
    if (!quantizer) {
        std::cerr << "❌ No quantizer provided to apply settings to" << std::endl;
        return false;
    }
    
    try {
        // Apply enabled state
        if (quantizationData.contains("enabled")) {
            bool enabled = quantizationData["enabled"];
            quantizer->setEnabled(enabled);
            std::cout << "🎵 Quantization " << (enabled ? "enabled" : "disabled") << std::endl;
        }
        
        // Apply grid resolution
        if (quantizationData.contains("grid_resolution")) {
            std::string gridResName = quantizationData["grid_resolution"];
            Quantizer::GridResolution resolution = Quantizer::GridResolution::QUARTER_NOTE; // default
            
            if (gridResName == "half_note") {
                resolution = Quantizer::GridResolution::HALF_NOTE;
            } else if (gridResName == "quarter_note") {
                resolution = Quantizer::GridResolution::QUARTER_NOTE;
            } else if (gridResName == "eighth_note") {
                resolution = Quantizer::GridResolution::EIGHTH_NOTE;
            } else if (gridResName == "sixteenth_note") {
                resolution = Quantizer::GridResolution::SIXTEENTH_NOTE;
            } else if (gridResName == "thirty_second_note") {
                resolution = Quantizer::GridResolution::THIRTY_SECOND_NOTE;
            } else if (gridResName == "sixty_fourth_note") {
                resolution = Quantizer::GridResolution::SIXTY_FOURTH_NOTE;
            }
            
            quantizer->setGridResolution(resolution);
            std::cout << "🎵 Grid resolution set to: " << gridResName << std::endl;
        }
        
        // Apply quantization amount
        if (quantizationData.contains("quantization_amount")) {
            float amount = quantizationData["quantization_amount"];
            // Clamp amount to valid range to prevent issues
            amount = std::max(0.0f, std::min(1.0f, amount));
            quantizer->setQuantizationAmount(amount);
            std::cout << "🎵 Quantization amount: " << (amount * 100.0f) << "%" << std::endl;
        }
        
        // Apply swing factor
        if (quantizationData.contains("swing_factor")) {
            float swing = quantizationData["swing_factor"];
            // Clamp swing to valid range to prevent issues  
            swing = std::max(-1.0f, std::min(1.0f, swing));
            quantizer->setSwingFactor(swing);
            std::cout << "🎵 Swing factor: " << (swing * 100.0f) << "%" << std::endl;
        }
        
        // Apply BPM (though this is usually managed by the global tempo)
        if (quantizationData.contains("bpm")) {
            float bpm = quantizationData["bpm"];
            // Ensure BPM is in reasonable range to prevent issues
            if (bpm > 0.0f && bpm <= 300.0f) {
                quantizer->setBPM(bpm);
                std::cout << "🎵 Quantizer BPM: " << bpm << std::endl;
            } else {
                std::cerr << "⚠️  Invalid BPM value in preset: " << bpm << ", skipping" << std::endl;
            }
        }
        
        std::cout << "✅ Quantization settings applied successfully" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error applying quantization settings: " << e.what() << std::endl;
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