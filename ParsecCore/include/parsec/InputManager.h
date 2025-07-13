#pragma once

#include "parsec/ModelConfig.h"
#include "parsec/SolverCore.h"
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <memory>
#include <chrono>

namespace parsec {

// Forward declarations
class IPlatform;

// Structure to represent an input source
struct InputSource {
    std::string name;               // Variable name to update in simulation
    std::string source_type;        // "sensor", "control", "simulation", "stream"
    std::string interface;          // "serial", "analog", "digital", "can", "websocket", etc.
    std::string address;            // Port, pin, URL, or identifier
    double update_frequency;        // Hz - how often to read this input
    double last_update_time = 0.0;  // Internal tracking
    double current_value = 0.0;     // Current cached value
    bool enabled = true;
    std::map<std::string, std::string> parameters; // Interface-specific parameters
};

// Structure for input data with metadata
struct InputData {
    std::string source_name;
    double value;
    double timestamp;
    std::string units;
    bool valid = true;
    std::string error_message;
};

// Callback function types for input events
using InputCallback = std::function<void(const InputData& data)>;
using InputErrorCallback = std::function<void(const std::string& source_name, const std::string& error)>;

class InputManager {
public:
    explicit InputManager(const ModelConfig& config, IPlatform* platform);
    
    // Configuration and setup
    bool loadInputConfiguration(const std::string& input_config_path);
    bool addInputSource(const InputSource& source);
    bool removeInputSource(const std::string& source_name);
    
    // Input processing
    void updateInputs(SimulationState& currentState, double current_time);
    void forceReadInput(const std::string& source_name);
    void forceReadAllInputs();
    
    // Manual input injection (for testing/debugging)
    void injectInput(const std::string& source_name, double value, double timestamp = -1);
    
    // Input source management
    std::vector<InputSource> getInputSources() const;
    InputSource* getInputSource(const std::string& source_name);
    bool enableInput(const std::string& source_name, bool enable = true);
    
    // Callbacks
    void setInputCallback(InputCallback callback);
    void setErrorCallback(InputErrorCallback callback);
    
    // Statistics and monitoring
    std::map<std::string, double> getInputStatistics() const; // Last values, update frequencies, etc.
    bool isInputHealthy(const std::string& source_name) const;
    std::vector<std::string> getFailedInputs() const;
    
    // Control
    void start();
    void stop();
    bool isRunning() const;
    
private:
    const ModelConfig& modelConfig_;
    IPlatform* platform_;
    std::vector<InputSource> inputSources_;
    
    // State tracking
    bool running_ = false;
    double lastUpdateTime_ = 0.0;
    
    // Callbacks
    InputCallback inputCallback_;
    InputErrorCallback errorCallback_;
    
    // Internal helpers
    bool readSensorInput(InputSource& source, double current_time);
    bool readControlInput(InputSource& source, double current_time);
    bool readSimulationInput(InputSource& source, double current_time);
    bool readStreamInput(InputSource& source, double current_time);
    
    // Interface-specific readers
    bool readSerialInput(InputSource& source);
    bool readAnalogInput(InputSource& source);
    bool readDigitalInput(InputSource& source);
    bool readCANInput(InputSource& source);
    bool readWebSocketInput(InputSource& source);
    
    // Validation and error handling
    bool validateInputSource(const InputSource& source) const;
    void handleInputError(const std::string& source_name, const std::string& error);
    
    // Timing helpers
    bool shouldUpdate(const InputSource& source, double current_time) const;
    void updateSourceTimestamp(InputSource& source, double current_time);
};

} // namespace parsec 