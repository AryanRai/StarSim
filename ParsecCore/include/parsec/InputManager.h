#ifndef PARSEC_INPUT_MANAGER_H
#define PARSEC_INPUT_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>

namespace parsec {

/**
 * @brief Input data structure for storing values received from various sources
 */
struct InputData {
    std::string id;
    std::string name;
    std::string datatype;
    std::string unit;
    double value;
    std::vector<double> vector_value;
    std::map<std::string, std::string> metadata;
    std::string timestamp;
};

/**
 * @brief Callback function type for input data updates
 */
using InputCallback = std::function<void(const InputData&)>;

/**
 * @brief Input Manager for handling data from various sources
 * 
 * The InputManager is responsible for receiving data from external sources
 * (WebSocket, Serial, etc.) and making it available to the simulation.
 */
class InputManager {
public:
    /**
     * @brief Constructor
     * @param simulation_id Unique identifier for the simulation
     */
    InputManager(const std::string& simulation_id);
    
    /**
     * @brief Destructor
     */
    ~InputManager();
    
    /**
     * @brief Initialize the input manager
     * @param ws_url WebSocket URL for the Stream Handler
     * @return True if initialization was successful
     */
    bool initialize(const std::string& ws_url = "ws://localhost:3000");
    
    /**
     * @brief Connect to the Stream Handler
     * @return True if connection was successful
     */
    bool connect();
    
    /**
     * @brief Disconnect from the Stream Handler
     */
    void disconnect();
    
    /**
     * @brief Register a new input stream
     * @param stream_id Unique identifier for the stream
     * @param name Human-readable name for the stream
     * @param datatype Data type (float, vector3, etc.)
     * @param unit Unit of measurement
     * @param metadata Additional metadata
     * @return True if registration was successful
     */
    bool registerStream(const std::string& stream_id, 
                        const std::string& name,
                        const std::string& datatype,
                        const std::string& unit,
                        const std::map<std::string, std::string>& metadata = {});
    
    /**
     * @brief Update a stream value
     * @param stream_id Stream identifier
     * @param value New value
     * @return True if update was successful
     */
    bool updateStreamValue(const std::string& stream_id, double value);
    
    /**
     * @brief Update a vector stream value
     * @param stream_id Stream identifier
     * @param values Vector of values
     * @return True if update was successful
     */
    bool updateStreamVectorValue(const std::string& stream_id, const std::vector<double>& values);
    
    /**
     * @brief Get the latest value for a stream
     * @param stream_id Stream identifier
     * @param default_value Default value if stream not found
     * @return The latest value or default_value if not found
     */
    double getStreamValue(const std::string& stream_id, double default_value = 0.0) const;
    
    /**
     * @brief Get the latest vector value for a stream
     * @param stream_id Stream identifier
     * @return The latest vector value or empty vector if not found
     */
    std::vector<double> getStreamVectorValue(const std::string& stream_id) const;
    
    /**
     * @brief Register a callback for stream updates
     * @param stream_id Stream identifier
     * @param callback Function to call when stream is updated
     * @return True if registration was successful
     */
    bool registerCallback(const std::string& stream_id, InputCallback callback);
    
    /**
     * @brief Send a control command to the simulation
     * @param command Command name
     * @param params Command parameters
     * @return True if command was sent successfully
     */
    bool sendCommand(const std::string& command, const std::map<std::string, std::string>& params = {});
    
    /**
     * @brief Update the simulation status
     * @param status New status
     * @return True if status was updated successfully
     */
    bool updateStatus(const std::string& status);
    
    /**
     * @brief Process incoming messages
     * @param message JSON message string
     */
    void processMessage(const std::string& message);
    
    /**
     * @brief Check if the input manager is connected
     * @return True if connected
     */
    bool isConnected() const { return connected_; }
    
    /**
     * @brief Get the simulation ID
     * @return Simulation ID
     */
    std::string getSimulationId() const { return simulation_id_; }

private:
    // WebSocket client implementation (platform-specific)
    class WebSocketClient;
    std::unique_ptr<WebSocketClient> ws_client_;
    
    // Simulation information
    std::string simulation_id_;
    std::atomic<bool> connected_;
    std::atomic<bool> running_;
    
    // Input data storage
    std::map<std::string, InputData> input_data_;
    std::map<std::string, std::vector<InputCallback>> callbacks_;
    
    // Thread safety
    mutable std::mutex data_mutex_;
    mutable std::mutex callback_mutex_;
    
    // Message processing thread
    std::thread message_thread_;
    
    // Internal methods
    void messageLoop();
    void notifyCallbacks(const std::string& stream_id, const InputData& data);
    std::string getCurrentTimestamp() const;
};

} // namespace parsec

#endif // PARSEC_INPUT_MANAGER_H