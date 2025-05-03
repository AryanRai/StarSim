#include "ParsecCore.h"
#include "IPlatform.h"
#include <string> // Needed for std::to_string
#include <vector>
#include <iostream> // For logging errors

ParsecCore::ParsecCore(IPlatform* platform)
    : platform_(platform),
      modelLoaded_(false) // Explicitly initialize modelLoaded_
      // configManager_ is default constructed
      // modelConfig_ is std::nullopt by default
      // unique_ptrs are nullptr by default
{
    if (!platform_) {
        // Handle error: platform cannot be null
        // For now, maybe just log or throw, depending on desired robustness
        std::cerr << "FATAL: ParsecCore created with null platform!" << std::endl;
        // Potentially throw std::invalid_argument("Platform cannot be null");
    }
}

void ParsecCore::clearModel() {
    modelConfig_.reset(); // Reset optional
    equationManager_.reset(); // Reset unique_ptr
    solverCore_.reset();    // Reset unique_ptr
    simulationState_.clear();
    modelLoaded_ = false;
}

bool ParsecCore::loadModel(const std::string& filepath) {
    platform_->log("Attempting to load model: " + filepath);
    clearModel(); // Clear any previously loaded model

    modelConfig_ = configManager_.loadModel(filepath);

    if (!modelConfig_) {
        platform_->log("Error: Failed to load model configuration from " + filepath);
        return false;
    }

    platform_->log("Model configuration loaded successfully: " + modelConfig_.value().model_name);

    // Create managers based on the loaded config
    try {
        equationManager_ = std::make_unique<parsec::EquationManager>(modelConfig_.value());
        solverCore_ = std::make_unique<parsec::SolverCore>(modelConfig_.value(), *equationManager_);

        // Initialize the simulation state
        simulationState_ = solverCore_->initializeState();
        // Add time variable if not present?
        if (simulationState_.find("time") == simulationState_.end()) {
             simulationState_["time"] = 0.0;
        }

        modelLoaded_ = true;
        platform_->log("Model loaded and initialized successfully.");
        return true;

    } catch (const std::exception& e) {
        platform_->log("Error initializing managers or state: " + std::string(e.what()));
        clearModel(); // Ensure clean state on error
        return false;
    }
}

void ParsecCore::tick() {
    if (!modelLoaded_ || !solverCore_) {
        // platform_->log("Tick: No model loaded.");
        return; // Or handle error/warning
    }

    // Perform one solver step
    solverCore_->step(simulationState_);

    // Update time
    simulationState_["time"] += solverCore_->getTimeStep();

    // Optional: Log some state variables
    // Example: Log x and v if they exist
    // if (simulationState_.count("x") && simulationState_.count("v")) {
    //     platform_->log("Tick: time=" + std::to_string(simulationState_["time"]) +
    //                    ", x=" + std::to_string(simulationState_["x"]) +
    //                    ", v=" + std::to_string(simulationState_["v"]));
    // }
}

const parsec::SimulationState& ParsecCore::getCurrentState() const {
    return simulationState_;
}
