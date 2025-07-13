#pragma once

#include "parsec/ModelConfig.h"
#include "parsec/SolverCore.h"
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <memory>
#include <fstream>
#include <queue>

namespace parsec {

// Forward declarations
class IPlatform;

// Structure to represent an output destination
struct OutputDestination {
    std::string name;                    // Unique identifier for this output
    std::string destination_type;        // "file", "stream", "display", "network", "serial"
    std::string format;                  // "csv", "json", "binary", "xml", "console"
    std::string address;                 // File path, URL, port, etc.
    std::vector<std::string> variables;  // Which variables to output
    double output_frequency;             // Hz - how often to output
    double last_output_time = 0.0;       // Internal tracking
    bool enabled = true;
    bool include_timestamp = true;
    std::map<std::string, std::string> parameters; // Format-specific parameters
};

// Structure for output data with metadata
struct OutputData {
    std::string destination_name;
    std::map<std::string, double> data;
    double timestamp;
    std::string format;
    bool valid = true;
    std::string error_message;
};

// Structure for buffered output (for high-frequency data)
struct OutputBuffer {
    std::queue<OutputData> buffer;
    size_t max_buffer_size = 1000;
    bool auto_flush = true;
    double flush_interval = 1.0; // seconds
    double last_flush_time = 0.0;
};

// Callback function types for output events
using OutputCallback = std::function<void(const OutputData& data)>;
using OutputErrorCallback = std::function<void(const std::string& destination_name, const std::string& error)>;

class OutputManager {
public:
    explicit OutputManager(const ModelConfig& config, IPlatform* platform);
    
    // Configuration and setup
    bool loadOutputConfiguration(const std::string& output_config_path);
    bool addOutputDestination(const OutputDestination& destination);
    bool removeOutputDestination(const std::string& destination_name);
    
    // Output processing
    void processOutputs(const SimulationState& currentState, double current_time);
    void forceOutput(const std::string& destination_name);
    void forceOutputAll();
    
    // Manual output injection (for logging/debugging)
    void injectOutput(const std::string& destination_name, const std::map<std::string, double>& data, double timestamp = -1);
    
    // Output destination management
    std::vector<OutputDestination> getOutputDestinations() const;
    OutputDestination* getOutputDestination(const std::string& destination_name);
    bool enableOutput(const std::string& destination_name, bool enable = true);
    
    // Data formatting
    std::string formatAsCSV(const OutputData& data) const;
    std::string formatAsJSON(const OutputData& data) const;
    std::string formatAsXML(const OutputData& data) const;
    std::vector<uint8_t> formatAsBinary(const OutputData& data) const;
    std::string formatForConsole(const OutputData& data) const;
    
    // Buffering management
    void enableBuffering(const std::string& destination_name, bool enable = true);
    void setBufferSize(const std::string& destination_name, size_t buffer_size);
    void flushBuffer(const std::string& destination_name);
    void flushAllBuffers();
    
    // File management
    bool openFile(const std::string& destination_name);
    bool closeFile(const std::string& destination_name);
    void closeAllFiles();
    
    // Callbacks
    void setOutputCallback(OutputCallback callback);
    void setErrorCallback(OutputErrorCallback callback);
    
    // Statistics and monitoring
    std::map<std::string, size_t> getOutputStatistics() const; // Output counts, buffer sizes, etc.
    bool isOutputHealthy(const std::string& destination_name) const;
    std::vector<std::string> getFailedOutputs() const;
    
    // Control
    void start();
    void stop();
    bool isRunning() const;
    
private:
    const ModelConfig& modelConfig_;
    IPlatform* platform_;
    std::vector<OutputDestination> outputDestinations_;
    std::map<std::string, OutputBuffer> outputBuffers_;
    std::map<std::string, std::unique_ptr<std::ofstream>> openFiles_;
    
    // State tracking
    bool running_ = false;
    double lastUpdateTime_ = 0.0;
    
    // Callbacks
    OutputCallback outputCallback_;
    OutputErrorCallback errorCallback_;
    
    // Internal helpers
    bool writeToFile(const OutputDestination& destination, const std::string& formatted_data);
    bool sendToStream(const OutputDestination& destination, const std::string& formatted_data);
    bool sendToDisplay(const OutputDestination& destination, const std::string& formatted_data);
    bool sendToNetwork(const OutputDestination& destination, const std::string& formatted_data);
    bool sendToSerial(const OutputDestination& destination, const std::string& formatted_data);
    
    // Format-specific writers
    bool writeCSV(const OutputDestination& destination, const OutputData& data);
    bool writeJSON(const OutputDestination& destination, const OutputData& data);
    bool writeBinary(const OutputDestination& destination, const OutputData& data);
    bool writeXML(const OutputDestination& destination, const OutputData& data);
    bool writeConsole(const OutputDestination& destination, const OutputData& data);
    
    // Buffer management helpers
    void addToBuffer(const std::string& destination_name, const OutputData& data);
    bool shouldFlushBuffer(const std::string& destination_name, double current_time) const;
    void processBufferedOutput(const std::string& destination_name);
    
    // Validation and error handling
    bool validateOutputDestination(const OutputDestination& destination) const;
    void handleOutputError(const std::string& destination_name, const std::string& error);
    
    // Timing helpers
    bool shouldOutput(const OutputDestination& destination, double current_time) const;
    void updateOutputTimestamp(OutputDestination& destination, double current_time);
    
    // Data filtering and selection
    std::map<std::string, double> filterVariables(const SimulationState& state, const std::vector<std::string>& requested_variables) const;
    
    // Header management for files
    void writeHeader(const OutputDestination& destination);
    std::string generateCSVHeader(const std::vector<std::string>& variables, bool include_timestamp) const;
    std::string generateJSONHeader() const;
    std::string generateXMLHeader() const;
};

} // namespace parsec 