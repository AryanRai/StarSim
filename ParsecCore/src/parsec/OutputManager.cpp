#include "parsec/OutputManager.h"
#include "platform/IPlatform.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

namespace parsec {

OutputManager::OutputManager(const ModelConfig& config, IPlatform* platform)
    : modelConfig_(config), platform_(platform) {
    running_ = false;
    lastUpdateTime_ = 0.0;
}

bool OutputManager::loadOutputConfiguration(const std::string& output_config_path) {
    std::ifstream ifs(output_config_path);
    if (!ifs.is_open()) {
        platform_->log("Error: Could not open output config file: " + output_config_path);
        return false;
    }

    nlohmann::json j;
    try {
        ifs >> j;
    } catch (nlohmann::json::parse_error& e) {
        platform_->log("Error: Failed to parse output config JSON: " + std::string(e.what()));
        return false;
    }

    // TODO: Parse output destinations from JSON
    platform_->log("Output configuration loaded successfully (stub implementation)");
    return true;
}

bool OutputManager::addOutputDestination(const OutputDestination& destination) {
    if (!validateOutputDestination(destination)) {
        handleOutputError(destination.name, "Invalid output destination configuration");
        return false;
    }

    // Check for duplicate names
    for (const auto& existing : outputDestinations_) {
        if (existing.name == destination.name) {
            handleOutputError(destination.name, "Output destination with this name already exists");
            return false;
        }
    }

    outputDestinations_.push_back(destination);
    
    // Initialize buffer if needed
    if (destination.destination_type == "file" || destination.destination_type == "network") {
        outputBuffers_[destination.name] = OutputBuffer();
    }
    
    platform_->log("Added output destination: " + destination.name);
    return true;
}

bool OutputManager::removeOutputDestination(const std::string& destination_name) {
    // Close file if open
    closeFile(destination_name);
    
    // Remove from destinations
    auto it = std::remove_if(outputDestinations_.begin(), outputDestinations_.end(),
        [&destination_name](const OutputDestination& dest) {
            return dest.name == destination_name;
        });
    
    if (it != outputDestinations_.end()) {
        outputDestinations_.erase(it, outputDestinations_.end());
        outputBuffers_.erase(destination_name);
        platform_->log("Removed output destination: " + destination_name);
        return true;
    }
    
    return false;
}

void OutputManager::processOutputs(const SimulationState& currentState, double current_time) {
    if (!running_) return;

    for (auto& destination : outputDestinations_) {
        if (!destination.enabled) continue;
        
        if (shouldOutput(destination, current_time)) {
            // Filter variables
            auto filtered_data = filterVariables(currentState, destination.variables);
            
            // Create output data
            OutputData output;
            output.destination_name = destination.name;
            output.data = filtered_data;
            output.timestamp = current_time;
            output.format = destination.format;
            output.valid = true;
            
            // Process output based on destination type
            bool success = false;
            if (destination.destination_type == "file") {
                success = writeToFile(destination, formatData(output, destination.format));
            } else if (destination.destination_type == "stream") {
                success = sendToStream(destination, formatData(output, destination.format));
            } else if (destination.destination_type == "display") {
                success = sendToDisplay(destination, formatData(output, destination.format));
            } else if (destination.destination_type == "network") {
                success = sendToNetwork(destination, formatData(output, destination.format));
            } else if (destination.destination_type == "serial") {
                success = sendToSerial(destination, formatData(output, destination.format));
            }
            
            if (success) {
                updateOutputTimestamp(destination, current_time);
                
                // Fire callback if set
                if (outputCallback_) {
                    outputCallback_(output);
                }
            }
        }
    }
    
    // Process buffered outputs
    for (auto& [name, buffer] : outputBuffers_) {
        if (shouldFlushBuffer(name, current_time)) {
            processBufferedOutput(name);
        }
    }
    
    lastUpdateTime_ = current_time;
}

void OutputManager::forceOutput(const std::string& destination_name) {
    auto* destination = getOutputDestination(destination_name);
    if (!destination) {
        handleOutputError(destination_name, "Output destination not found");
        return;
    }
    
    platform_->log("Force output triggered for: " + destination_name + " (stub implementation)");
}

void OutputManager::forceOutputAll() {
    for (auto& destination : outputDestinations_) {
        if (destination.enabled) {
            forceOutput(destination.name);
        }
    }
}

std::string OutputManager::formatAsCSV(const OutputData& data) const {
    std::stringstream ss;
    
    // Add timestamp if requested
    if (data.timestamp >= 0) {
        ss << std::fixed << std::setprecision(6) << data.timestamp;
        if (!data.data.empty()) ss << ",";
    }
    
    // Add data values
    bool first = true;
    for (const auto& [key, value] : data.data) {
        if (!first) ss << ",";
        ss << std::fixed << std::setprecision(6) << value;
        first = false;
    }
    
    ss << "\n";
    return ss.str();
}

std::string OutputManager::formatAsJSON(const OutputData& data) const {
    nlohmann::json j;
    
    if (data.timestamp >= 0) {
        j["timestamp"] = data.timestamp;
    }
    
    j["data"] = data.data;
    
    return j.dump() + "\n";
}

std::string OutputManager::formatAsXML(const OutputData& data) const {
    std::stringstream ss;
    ss << "<output>";
    
    if (data.timestamp >= 0) {
        ss << "<timestamp>" << std::fixed << std::setprecision(6) << data.timestamp << "</timestamp>";
    }
    
    ss << "<data>";
    for (const auto& [key, value] : data.data) {
        ss << "<" << key << ">" << std::fixed << std::setprecision(6) << value << "</" << key << ">";
    }
    ss << "</data>";
    
    ss << "</output>\n";
    return ss.str();
}

std::vector<uint8_t> OutputManager::formatAsBinary(const OutputData& data) const {
    std::vector<uint8_t> binary;
    
    // Simple binary format: timestamp (8 bytes) + number of values (4 bytes) + values (8 bytes each)
    if (data.timestamp >= 0) {
        const uint8_t* timestamp_bytes = reinterpret_cast<const uint8_t*>(&data.timestamp);
        binary.insert(binary.end(), timestamp_bytes, timestamp_bytes + sizeof(double));
    }
    
    uint32_t count = static_cast<uint32_t>(data.data.size());
    const uint8_t* count_bytes = reinterpret_cast<const uint8_t*>(&count);
    binary.insert(binary.end(), count_bytes, count_bytes + sizeof(uint32_t));
    
    for (const auto& [key, value] : data.data) {
        const uint8_t* value_bytes = reinterpret_cast<const uint8_t*>(&value);
        binary.insert(binary.end(), value_bytes, value_bytes + sizeof(double));
    }
    
    return binary;
}

std::string OutputManager::formatForConsole(const OutputData& data) const {
    std::stringstream ss;
    
    if (data.timestamp >= 0) {
        ss << "[" << std::fixed << std::setprecision(3) << data.timestamp << "] ";
    }
    
    bool first = true;
    for (const auto& [key, value] : data.data) {
        if (!first) ss << ", ";
        ss << key << "=" << std::fixed << std::setprecision(6) << value;
        first = false;
    }
    
    ss << "\n";
    return ss.str();
}

void OutputManager::start() {
    running_ = true;
    platform_->log("Output Manager started");
}

void OutputManager::stop() {
    running_ = false;
    flushAllBuffers();
    closeAllFiles();
    platform_->log("Output Manager stopped");
}

bool OutputManager::isRunning() const {
    return running_;
}

// Stub implementations for private methods

bool OutputManager::writeToFile(const OutputDestination& destination, const std::string& formatted_data) {
    // TODO: Implement actual file writing
    platform_->log("Writing to file " + destination.address + " (stub implementation)");
    return true;
}

bool OutputManager::sendToStream(const OutputDestination& destination, const std::string& formatted_data) {
    // TODO: Implement stream output
    platform_->log("Sending to stream " + destination.address + " (stub implementation)");
    return true;
}

bool OutputManager::sendToDisplay(const OutputDestination& destination, const std::string& formatted_data) {
    // For console display, actually output the data
    std::cout << formatted_data;
    return true;
}

bool OutputManager::sendToNetwork(const OutputDestination& destination, const std::string& formatted_data) {
    // TODO: Implement network output
    platform_->log("Sending to network " + destination.address + " (stub implementation)");
    return true;
}

bool OutputManager::sendToSerial(const OutputDestination& destination, const std::string& formatted_data) {
    // TODO: Implement serial output
    platform_->log("Sending to serial " + destination.address + " (stub implementation)");
    return true;
}

std::map<std::string, double> OutputManager::filterVariables(const SimulationState& state, const std::vector<std::string>& requested_variables) const {
    std::map<std::string, double> filtered;
    
    if (requested_variables.empty()) {
        // Return all variables
        return state;
    }
    
    for (const auto& var_name : requested_variables) {
        auto it = state.find(var_name);
        if (it != state.end()) {
            filtered[var_name] = it->second;
        }
    }
    
    return filtered;
}

bool OutputManager::validateOutputDestination(const OutputDestination& destination) const {
    if (destination.name.empty()) return false;
    if (destination.destination_type.empty()) return false;
    if (destination.format.empty()) return false;
    if (destination.output_frequency <= 0) return false;
    
    return true;
}

void OutputManager::handleOutputError(const std::string& destination_name, const std::string& error) {
    platform_->log("Output error on " + destination_name + ": " + error);
    
    if (errorCallback_) {
        errorCallback_(destination_name, error);
    }
}

bool OutputManager::shouldOutput(const OutputDestination& destination, double current_time) const {
    if (!destination.enabled) return false;
    
    double interval = 1.0 / destination.output_frequency;
    return (current_time - destination.last_output_time) >= interval;
}

void OutputManager::updateOutputTimestamp(OutputDestination& destination, double current_time) {
    destination.last_output_time = current_time;
}

OutputDestination* OutputManager::getOutputDestination(const std::string& destination_name) {
    for (auto& destination : outputDestinations_) {
        if (destination.name == destination_name) {
            return &destination;
        }
    }
    return nullptr;
}

std::string OutputManager::formatData(const OutputData& data, const std::string& format) const {
    if (format == "csv") {
        return formatAsCSV(data);
    } else if (format == "json") {
        return formatAsJSON(data);
    } else if (format == "xml") {
        return formatAsXML(data);
    } else if (format == "console") {
        return formatForConsole(data);
    }
    
    // Default to console format
    return formatForConsole(data);
}

bool OutputManager::shouldFlushBuffer(const std::string& destination_name, double current_time) const {
    auto it = outputBuffers_.find(destination_name);
    if (it == outputBuffers_.end()) return false;
    
    const auto& buffer = it->second;
    return buffer.auto_flush && 
           (current_time - buffer.last_flush_time) >= buffer.flush_interval;
}

void OutputManager::processBufferedOutput(const std::string& destination_name) {
    // TODO: Implement buffered output processing
    platform_->log("Processing buffered output for " + destination_name + " (stub implementation)");
}

void OutputManager::flushAllBuffers() {
    for (auto& [name, buffer] : outputBuffers_) {
        processBufferedOutput(name);
    }
}

bool OutputManager::openFile(const std::string& destination_name) {
    // TODO: Implement file opening
    platform_->log("Opening file for " + destination_name + " (stub implementation)");
    return true;
}

bool OutputManager::closeFile(const std::string& destination_name) {
    auto it = openFiles_.find(destination_name);
    if (it != openFiles_.end()) {
        // File will be closed automatically when unique_ptr is destroyed
        openFiles_.erase(it);
        platform_->log("Closed file for " + destination_name);
        return true;
    }
    return false;
}

void OutputManager::closeAllFiles() {
    for (auto& [name, file] : openFiles_) {
        platform_->log("Closing file for " + name);
    }
    openFiles_.clear();
}

} // namespace parsec 