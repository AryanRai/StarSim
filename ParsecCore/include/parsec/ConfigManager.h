#pragma once

#include "parsec/ModelConfig.h"
#include <string>
#include <optional>
#include <nlohmann/json.hpp> // Include the JSON library header

namespace parsec {

class ConfigManager {
public:
    // Attempts to load a model configuration from the specified JSON file.
    // Returns the loaded config on success, or std::nullopt on failure (e.g., file not found, parse error).
    std::optional<ModelConfig> loadModel(const std::string& filepath);

private:
    // Helper functions to parse specific parts of the JSON
    VariableType parseVariableType(const std::string& typeStr);
    void parseVariables(const nlohmann::json& jsonVariables, ModelConfig& config);
    void parseSolverSettings(const nlohmann::json& jsonSolver, ModelConfig& config);
};

} // namespace parsec 