#include "parsec/InputManager.h"
#include "platform/IPlatform.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <nlohmann/json.hpp>

namespace parsec {

InputManager::InputManager(const ModelConfig& config, IPlatform* platform)
    : modelConfig_(config), platform_(platform) {
    // Initialize input manager
    running_ = false;
    lastUpdateTime_ = 0.0;
}

bool InputManager::loadInputConfiguration(const std::string& input_config_path) {
    std::ifstream ifs(input_config_path);
    if (!ifs.is_open()) {
        platform_->log("Error: Could not open input config file: " + input_config_path);
        return false;
    }

    nlohmann::json j;
    try {
        ifs >> j;
    } catch (nlohmann::json::parse_error& e) {
        platform_->log("Error: Failed to parse input config JSON: " + std::string(e.what()));
        return false;
    }

    // TODO: Parse input sources from JSON
    // For now, return true to indicate basic structure is working
    platform_->log("Input configuration loaded successfully (stub implementation)");
    return true;
}

bool InputManager::addInputSource(const InputSource& source) {
    if (!validateInputSource(source)) {
        handleInputError(source.name, "Invalid input source configuration");
        return false;
    }

    // Check for duplicate names
    for (const auto& existing : inputSources_) {
        if (existing.name == source.name) {
            handleInputError(source.name, "Input source with this name already exists");
            return false;
        }
    }

    inputSources_.push_back(source);
    platform_->log("Added input source: " + source.name);
    return true;
}

bool InputManager::removeInputSource(const std::string& source_name) {
    auto it = std::remove_if(inputSources_.begin(), inputSources_.end(),
        [&source_name](const InputSource& source) {
            return source.name == source_name;
        });
    
    if (it != inputSources_.end()) {
        inputSources_.erase(it, inputSources_.end());
        platform_->log("Removed input source: " + source_name);
        return true;
    }
    
    return false;
}

void InputManager::updateInputs(SimulationState& currentState, double current_time) {
    if (!running_) return;

    for (auto& source : inputSources_) {
        if (!source.enabled) continue;
        
        if (shouldUpdate(source, current_time)) {
            // TODO: Implement actual input reading based on source type
            bool success = false;
            
            if (source.source_type == "sensor") {
                success = readSensorInput(source, current_time);
            } else if (source.source_type == "control") {
                success = readControlInput(source, current_time);
            } else if (source.source_type == "simulation") {
                success = readSimulationInput(source, current_time);
            } else if (source.source_type == "stream") {
                success = readStreamInput(source, current_time);
            }
            
            if (success) {
                // Update simulation state with new input value
                currentState[source.name] = source.current_value;
                updateSourceTimestamp(source, current_time);
                
                // Fire callback if set
                if (inputCallback_) {
                    InputData data;
                    data.source_name = source.name;
                    data.value = source.current_value;
                    data.timestamp = current_time;
                    data.valid = true;
                    inputCallback_(data);
                }
            }
        }
    }
    
    lastUpdateTime_ = current_time;
}

void InputManager::forceReadInput(const std::string& source_name) {
    auto* source = getInputSource(source_name);
    if (!source) {
        handleInputError(source_name, "Input source not found");
        return;
    }
    
    double current_time = platform_->getMillis() / 1000.0;
    
    // Force read regardless of timing
    if (source->source_type == "sensor") {
        readSensorInput(*source, current_time);
    } else if (source->source_type == "control") {
        readControlInput(*source, current_time);
    } else if (source->source_type == "simulation") {
        readSimulationInput(*source, current_time);
    } else if (source->source_type == "stream") {
        readStreamInput(*source, current_time);
    }
}

void InputManager::forceReadAllInputs() {
    for (auto& source : inputSources_) {
        if (source.enabled) {
            forceReadInput(source.name);
        }
    }
}

void InputManager::injectInput(const std::string& source_name, double value, double timestamp) {
    auto* source = getInputSource(source_name);
    if (!source) {
        handleInputError(source_name, "Input source not found for injection");
        return;
    }
    
    source->current_value = value;
    if (timestamp < 0) {
        timestamp = platform_->getMillis() / 1000.0;
    }
    source->last_update_time = timestamp;
    
    platform_->log("Injected input " + source_name + " = " + std::to_string(value));
}

std::vector<InputSource> InputManager::getInputSources() const {
    return inputSources_;
}

InputSource* InputManager::getInputSource(const std::string& source_name) {
    for (auto& source : inputSources_) {
        if (source.name == source_name) {
            return &source;
        }
    }
    return nullptr;
}

bool InputManager::enableInput(const std::string& source_name, bool enable) {
    auto* source = getInputSource(source_name);
    if (!source) return false;
    
    source->enabled = enable;
    platform_->log("Input source " + source_name + (enable ? " enabled" : " disabled"));
    return true;
}

void InputManager::setInputCallback(InputCallback callback) {
    inputCallback_ = callback;
}

void InputManager::setErrorCallback(InputErrorCallback callback) {
    errorCallback_ = callback;
}

std::map<std::string, double> InputManager::getInputStatistics() const {
    std::map<std::string, double> stats;
    
    for (const auto& source : inputSources_) {
        stats[source.name + "_value"] = source.current_value;
        stats[source.name + "_last_update"] = source.last_update_time;
        stats[source.name + "_enabled"] = source.enabled ? 1.0 : 0.0;
    }
    
    return stats;
}

bool InputManager::isInputHealthy(const std::string& source_name) const {
    const auto* source = const_cast<InputManager*>(this)->getInputSource(source_name);
    if (!source) return false;
    
    double current_time = platform_->getMillis() / 1000.0;
    double time_since_update = current_time - source->last_update_time;
    
    // Consider healthy if updated within 2x the expected interval
    return time_since_update < (2.0 / source->update_frequency);
}

std::vector<std::string> InputManager::getFailedInputs() const {
    std::vector<std::string> failed;
    
    for (const auto& source : inputSources_) {
        if (source.enabled && !isInputHealthy(source.name)) {
            failed.push_back(source.name);
        }
    }
    
    return failed;
}

void InputManager::start() {
    running_ = true;
    platform_->log("Input Manager started");
}

void InputManager::stop() {
    running_ = false;
    platform_->log("Input Manager stopped");
}

bool InputManager::isRunning() const {
    return running_;
}

// Private helper methods (stub implementations)

bool InputManager::readSensorInput(InputSource& source, double current_time) {
    // TODO: Implement actual sensor reading based on interface type
    if (source.interface == "analog") {
        return readAnalogInput(source);
    } else if (source.interface == "digital") {
        return readDigitalInput(source);
    } else if (source.interface == "serial") {
        return readSerialInput(source);
    } else if (source.interface == "can") {
        return readCANInput(source);
    }
    
    // Stub: Generate simulated sensor data
    source.current_value = 0.5; // Placeholder value
    platform_->log("Read sensor input " + source.name + " (stub implementation)");
    return true;
}

bool InputManager::readControlInput(InputSource& source, double current_time) {
    // TODO: Implement control input reading
    platform_->log("Read control input " + source.name + " (stub implementation)");
    return true;
}

bool InputManager::readSimulationInput(InputSource& source, double current_time) {
    // TODO: Implement simulation parameter input reading
    platform_->log("Read simulation input " + source.name + " (stub implementation)");
    return true;
}

bool InputManager::readStreamInput(InputSource& source, double current_time) {
    // TODO: Implement stream input reading
    if (source.interface == "websocket") {
        return readWebSocketInput(source);
    }
    
    platform_->log("Read stream input " + source.name + " (stub implementation)");
    return true;
}

bool InputManager::readSerialInput(InputSource& source) {
    // TODO: Implement serial input reading
    platform_->log("Read serial input " + source.name + " (stub implementation)");
    return true;
}

bool InputManager::readAnalogInput(InputSource& source) {
    // TODO: Implement analog input reading
    platform_->log("Read analog input " + source.name + " (stub implementation)");
    return true;
}

bool InputManager::readDigitalInput(InputSource& source) {
    // TODO: Implement digital input reading
    platform_->log("Read digital input " + source.name + " (stub implementation)");
    return true;
}

bool InputManager::readCANInput(InputSource& source) {
    // TODO: Implement CAN input reading
    platform_->log("Read CAN input " + source.name + " (stub implementation)");
    return true;
}

bool InputManager::readWebSocketInput(InputSource& source) {
    // TODO: Implement WebSocket input reading
    platform_->log("Read WebSocket input " + source.name + " (stub implementation)");
    return true;
}

bool InputManager::validateInputSource(const InputSource& source) const {
    if (source.name.empty()) return false;
    if (source.source_type.empty()) return false;
    if (source.interface.empty()) return false;
    if (source.update_frequency <= 0) return false;
    
    return true;
}

void InputManager::handleInputError(const std::string& source_name, const std::string& error) {
    platform_->log("Input error on " + source_name + ": " + error);
    
    if (errorCallback_) {
        errorCallback_(source_name, error);
    }
}

bool InputManager::shouldUpdate(const InputSource& source, double current_time) const {
    if (!source.enabled) return false;
    
    double interval = 1.0 / source.update_frequency;
    return (current_time - source.last_update_time) >= interval;
}

void InputManager::updateSourceTimestamp(InputSource& source, double current_time) {
    source.last_update_time = current_time;
}

} // namespace parsec 