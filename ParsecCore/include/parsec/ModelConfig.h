#pragma once

#include "parsec/Variable.h"
#include <string>
#include <vector>
#include <map>

namespace parsec {

struct SolverSettings {
    std::string method = "RK4"; // Default solver
    double dt = 0.01;           // Default time step
    double duration = 10.0;     // Default duration
};

struct ModelConfig {
    std::string model_name;
    std::string version;
    std::map<std::string, std::string> metadata; // Simple key-value for now
    std::vector<Variable> variables;
    std::vector<std::string> equations; // Store as strings for now
    SolverSettings solver;
    std::vector<std::string> outputs; // Names of variables to output

    // We'll add methods to load this from JSON later via ConfigManager
};

} // namespace parsec 