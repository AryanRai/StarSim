#pragma once

#include "IPlatform.h"
#include "ConfigManager.h"          // Include ConfigManager
#include "parsec/EquationManager.h" // Update path
#include "parsec/SolverCore.h"      // Update path
#include "parsec/ModelConfig.h"   // Include ModelConfig
#include <string>
#include <memory>                 // For std::unique_ptr
#include <map>                    // For SimulationState
#include <optional>               // For optional ModelConfig

class ParsecCore {
public:
    explicit ParsecCore(IPlatform* platform);

    // Loads a simulation model from a file path.
    // Returns true on success, false on failure.
    bool loadModel(const std::string& filepath);

    // Executes one simulation time step if a model is loaded.
    void tick();

    // Gets the current simulation state (read-only)
    const parsec::SimulationState& getCurrentState() const;

private:
    IPlatform* platform_; // Non-owning pointer to the platform implementation

    // Managers and configuration data (owned by ParsecCore)
    parsec::ConfigManager configManager_;
    std::optional<parsec::ModelConfig> modelConfig_; // Store loaded config
    std::unique_ptr<parsec::EquationManager> equationManager_;
    std::unique_ptr<parsec::SolverCore> solverCore_;

    // Simulation state
    parsec::SimulationState simulationState_;
    bool modelLoaded_ = false;

    // Internal helper to clear the current model state
    void clearModel();
};
