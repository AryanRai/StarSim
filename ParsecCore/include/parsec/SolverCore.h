#pragma once

#include "parsec/ModelConfig.h"
#include "parsec/EquationManager.h"
#include <vector>
#include <string>
#include <map>

namespace parsec {

// Represents the current state of the simulation (variable names to values)
typedef std::map<std::string, double> SimulationState;

class SolverCore {
public:
    // Constructor takes references to the config and equation manager
    SolverCore(const ModelConfig& config, EquationManager& eqManager);

    // Performs one simulation step using the specified method (currently only Euler)
    // Updates the provided state in-place.
    void step(SimulationState& currentState);

    // Initializes the simulation state based on the loaded model config
    SimulationState initializeState() const;

    // Gets the configured time step (dt)
    double getTimeStep() const;

private:
    const ModelConfig& modelConfig_; // Reference to configuration
    EquationManager& equationManager_; // Reference to equation manager

    // Helper to identify the derivative name for a dynamic variable (e.g., "x" -> "d(x)/dt")
    std::string getDerivativeName(const std::string& variableName) const;
};

} // namespace parsec 