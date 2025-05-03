#include "parsec/SolverCore.h"
#include <stdexcept> // For std::runtime_error
#include <iostream>  // For warnings/errors

namespace parsec {

SolverCore::SolverCore(const ModelConfig& config, EquationManager& eqManager)
    : modelConfig_(config), equationManager_(eqManager) {
    // TODO: Select solver based on config.solver.method if supporting multiple solvers
    if (config.solver.method != "Euler") {
        // For now, only Euler is implemented via the hardcoded EquationManager
        std::cerr << "Warning: Solver method '" << config.solver.method << "' requested, but only basic Euler is implemented via EquationManager. Proceeding with Euler." << std::endl;
    }
}

SimulationState SolverCore::initializeState() const {
    SimulationState state;
    for (const auto& var : modelConfig_.variables) {
        if (var.type == VariableType::DYNAMIC || var.type == VariableType::CONSTANT) {
            if (std::holds_alternative<double>(var.value)) {
                state[var.name] = std::get<double>(var.value);
            } else {
                // Should not happen if ConfigManager validation is correct, but handle defensively
                std::cerr << "Warning: Variable '" << var.name << "' has non-double value despite being dynamic/constant. Defaulting to 0.0." << std::endl;
                state[var.name] = 0.0;
            }
        } else if (var.type == VariableType::INPUT) {
             // Initialize inputs to 0.0 by default. They should be updated externally.
             state[var.name] = 0.0; 
        }
    }
    return state;
}

double SolverCore::getTimeStep() const {
    return modelConfig_.solver.dt;
}

// Basic Forward Euler implementation
void SolverCore::step(SimulationState& currentState) {
    double dt = modelConfig_.solver.dt;

    // 1. Evaluate all derivatives based on the *current* state
    // Note: EquationManager::evaluateDerivatives currently uses hardcoded logic!
    std::map<std::string, double> derivatives = equationManager_.evaluateDerivatives(currentState);

    // 2. Update dynamic variables using the calculated derivatives
    for (const auto& var : modelConfig_.variables) {
        if (var.type == VariableType::DYNAMIC) {
            std::string derivName = getDerivativeName(var.name);
            auto it = derivatives.find(derivName);

            if (it != derivatives.end()) {
                // Euler step: y_new = y_old + dt * dy/dt
                currentState[var.name] += dt * it->second;
            } else {
                // This might happen if an equation for a dynamic variable's derivative is missing
                std::cerr << "Warning: Derivative '" << derivName << "' not found for dynamic variable '" << var.name << "'. Variable not updated." << std::endl;
            }
        }
        // Constants and Inputs are not updated by the solver step itself
    }

    // TODO: Add time update (e.g., currentState["time"] += dt;)
}

// Simple helper to construct the expected derivative name string.
// Assumes the convention "d(variableName)/dt".
std::string SolverCore::getDerivativeName(const std::string& variableName) const {
    return "d(" + variableName + ")/dt";
}


} // namespace parsec 