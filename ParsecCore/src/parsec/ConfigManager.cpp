#include "parsec/ConfigManager.h"
#include <fstream>
#include <iostream> // For error logging
#include <stdexcept> // For exceptions during parsing

namespace parsec {

// Helper to safely get values from JSON or use default
// Usage: safe_get_or(json_obj, "key", default_value);
template<typename T>
T safe_get_or(const nlohmann::json& j, const std::string& key, const T& default_val) {
    return j.contains(key) ? j.at(key).get<T>() : default_val;
}

VariableType ConfigManager::parseVariableType(const std::string& typeStr) {
    if (typeStr == "dynamic") return VariableType::DYNAMIC;
    if (typeStr == "constant") return VariableType::CONSTANT;
    if (typeStr == "input") return VariableType::INPUT;
    // Default or throw error if unknown type encountered
    std::cerr << "Warning: Unknown variable type '" << typeStr << "'. Defaulting to DYNAMIC." << std::endl;
    return VariableType::DYNAMIC;
}

void ConfigManager::parseVariables(const nlohmann::json& jsonVariables, ModelConfig& config) {
    if (!jsonVariables.is_array()) {
        throw std::runtime_error("JSON parsing error: 'variables' field must be an array.");
    }
    for (const auto& item : jsonVariables) {
        Variable var;
        var.name = safe_get_or(item, "name", std::string("unnamed_variable"));
        var.unit = safe_get_or(item, "unit", std::string(""));
        var.type = parseVariableType(safe_get_or(item, "type", std::string("dynamic")));

        // Handle initial/constant value based on type
        if (var.type == VariableType::CONSTANT || var.type == VariableType::DYNAMIC) {
            if (item.contains("initial")) {
                 var.value = item.at("initial").get<double>();
            } else if (item.contains("value")) { // Allow 'value' key for constants
                 var.value = item.at("value").get<double>();
            } else {
                std::cerr << "Warning: Variable '" << var.name << "' of type constant/dynamic missing 'initial' or 'value'. Defaulting to 0.0." << std::endl;
                var.value = 0.0;
            }
        } else if (var.type == VariableType::INPUT) {
             var.value = safe_get_or(item, "placeholder", std::string("")); // Store placeholder if any
        }

        config.variables.push_back(var);
    }
}

void ConfigManager::parseSolverSettings(const nlohmann::json& jsonSolver, ModelConfig& config) {
    if (!jsonSolver.is_object()) {
        // Use defaults if solver section is missing or not an object
        std::cerr << "Warning: 'solver' section missing or invalid in JSON. Using default settings." << std::endl;
        config.solver = SolverSettings{}; // Use default constructor
        return;
    }
    config.solver.method = safe_get_or(jsonSolver, "method", config.solver.method);
    config.solver.dt = safe_get_or(jsonSolver, "dt", config.solver.dt);
    config.solver.duration = safe_get_or(jsonSolver, "duration", config.solver.duration);
}

std::optional<ModelConfig> ConfigManager::loadModel(const std::string& filepath) {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) {
        std::cerr << "Error: Could not open file: " << filepath << std::endl;
        return std::nullopt;
    }

    nlohmann::json j;
    try {
        ifs >> j; // Parse the file stream into the JSON object
    } catch (nlohmann::json::parse_error& e) {
        std::cerr << "Error: Failed to parse JSON file: " << filepath << "\n" << e.what() << std::endl;
        return std::nullopt;
    }

    ModelConfig config;
    try {
        config.model_name = safe_get_or(j, "model_name", std::string("Unnamed Model"));
        config.version = safe_get_or(j, "version", std::string("1.0"));

        // Parse metadata (simple map for now)
        if (j.contains("metadata") && j["metadata"].is_object()) {
            for (auto& [key, val] : j["metadata"].items()) {
                if (val.is_string()) {
                    config.metadata[key] = val.get<std::string>();
                }
            }
        }

        if (j.contains("variables")) {
            parseVariables(j["variables"], config);
        }

        if (j.contains("equations") && j["equations"].is_array()) {
            for (const auto& eq : j["equations"]) {
                if (eq.is_string()) {
                    config.equations.push_back(eq.get<std::string>());
                }
            }
        }

        if (j.contains("solver")) {
            parseSolverSettings(j["solver"], config);
        }

        if (j.contains("outputs") && j["outputs"].is_array()) {
            for (const auto& out : j["outputs"]) {
                if (out.is_string()) {
                    config.outputs.push_back(out.get<std::string>());
                }
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error processing JSON content: " << e.what() << std::endl;
        return std::nullopt;
    }

    return config;
}

} // namespace parsec 