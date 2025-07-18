#include "parsec/InputManager.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace parsec {

// Simple WebSocket client stub implementation
class InputManager::WebSocketClient {
public:
    WebSocketClient(const std::string& simulation_id) : simulation_id_(simulation_id), connected_(false) {
        std::cout << "WebSocket client initialized (stub implementation) for simulation: " << simulation_id << std::endl;
    }
    
    ~WebSocketClient() {
        disconnect();
    }
    
    bool connect(const std::string& uri) {
        std::cout << "Attempting to connect to: " << uri << std::endl;
        // For now, just simulate connection
        connected_ = true;
        std::cout << "Connected (simulated) - WebSocket client ready" << std::endl;
        return true;
    }
    
    void disconnect() {
        if (connected_) {
            std::cout << "Disconnecting WebSocket client" << std::endl;
            connected_ = false;
        }
    }
    
    bool send(const std::string& message) {
        if (!connected_) {
            return false;
        }
        std::cout << "Sending message: " << message << std::endl;
        return true;
    }
    
    bool isConnected() const {
        return connected_;
    }
    
private:
    std::string simulation_id_;
    std::atomic<bool> connected_;
};

InputManager::InputManager(const std::string& simulation_id) 
    : simulation_id_(simulation_id), connected_(false), running_(false) {
    std::cout << "InputManager initialized for simulation: " << simulation_id << std::endl;
}

InputManager::~InputManager() {
    disconnect();
}

bool InputManager::initialize(const std::string& ws_url) {
    ws_client_ = std::make_unique<WebSocketClient>(simulation_id_);
    std::cout << "InputManager initialized with WebSocket URL: " << ws_url << std::endl;
    return true;
}

bool InputManager::connect() {
    if (!ws_client_) {
        std::cout << "WebSocket client not initialized" << std::endl;
        return false;
    }
    
    bool success = ws_client_->connect("ws://localhost:3000");
    if (success) {
        connected_ = true;
        running_ = true;
        std::cout << "InputManager connected successfully" << std::endl;
    }
    return success;
}

void InputManager::disconnect() {
    if (ws_client_) {
        ws_client_->disconnect();
    }
    connected_ = false;
    running_ = false;
    std::cout << "InputManager disconnected" << std::endl;
}

bool InputManager::registerStream(const std::string& stream_id, 
                                 const std::string& name,
                                 const std::string& datatype,
                                 const std::string& unit,
                                 const std::map<std::string, std::string>& metadata) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    InputData data;
    data.id = stream_id;
    data.name = name;
    data.datatype = datatype;
    data.unit = unit;
    data.value = 0.0;
    data.metadata = metadata;
    data.timestamp = getCurrentTimestamp();
    
    input_data_[stream_id] = data;
    
    std::cout << "Registered stream: " << stream_id << " (" << name << ")" << std::endl;
    return true;
}

bool InputManager::updateStreamValue(const std::string& stream_id, double value) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    auto it = input_data_.find(stream_id);
    if (it != input_data_.end()) {
        it->second.value = value;
        it->second.timestamp = getCurrentTimestamp();
        
        // Notify callbacks
        notifyCallbacks(stream_id, it->second);
        
        std::cout << "Updated stream " << stream_id << " = " << value << std::endl;
        return true;
    }
    
    std::cout << "Stream not found: " << stream_id << std::endl;
    return false;
}

bool InputManager::updateStreamVectorValue(const std::string& stream_id, const std::vector<double>& values) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    auto it = input_data_.find(stream_id);
    if (it != input_data_.end()) {
        it->second.vector_value = values;
        it->second.timestamp = getCurrentTimestamp();
        
        // Notify callbacks
        notifyCallbacks(stream_id, it->second);
        
        std::cout << "Updated vector stream " << stream_id << " with " << values.size() << " values" << std::endl;
        return true;
    }
    
    std::cout << "Stream not found: " << stream_id << std::endl;
    return false;
}

double InputManager::getStreamValue(const std::string& stream_id, double default_value) const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    auto it = input_data_.find(stream_id);
    if (it != input_data_.end()) {
        return it->second.value;
    }
    
    return default_value;
}

std::vector<double> InputManager::getStreamVectorValue(const std::string& stream_id) const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    auto it = input_data_.find(stream_id);
    if (it != input_data_.end()) {
        return it->second.vector_value;
    }
    
    return std::vector<double>();
}

bool InputManager::registerCallback(const std::string& stream_id, InputCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    
    callbacks_[stream_id].push_back(callback);
    std::cout << "Registered callback for stream: " << stream_id << std::endl;
    return true;
}

bool InputManager::sendCommand(const std::string& command, const std::map<std::string, std::string>& params) {
    if (!ws_client_ || !connected_) {
        std::cout << "WebSocket not connected, cannot send command" << std::endl;
        return false;
    }
    
    json message = {
        {"type", "command"},
        {"command", command},
        {"params", params},
        {"simulation_id", simulation_id_}
    };
    
    return ws_client_->send(message.dump());
}

bool InputManager::updateStatus(const std::string& status) {
    if (!ws_client_ || !connected_) {
        std::cout << "WebSocket not connected, cannot update status" << std::endl;
        return false;
    }
    
    json message = {
        {"type", "status"},
        {"status", status},
        {"simulation_id", simulation_id_}
    };
    
    return ws_client_->send(message.dump());
}

void InputManager::processMessage(const std::string& message) {
    try {
        json msg = json::parse(message);
        
        if (msg.contains("type") && msg["type"] == "stream_update") {
            std::string stream_id = msg["stream_id"];
            double value = msg["value"];
            updateStreamValue(stream_id, value);
        }
        
        std::cout << "Processed message: " << message << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error processing message: " << e.what() << std::endl;
    }
}

void InputManager::messageLoop() {
    std::cout << "Message loop started" << std::endl;
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // In a real implementation, this would listen for incoming messages
    }
    std::cout << "Message loop stopped" << std::endl;
}

void InputManager::notifyCallbacks(const std::string& stream_id, const InputData& data) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    
    auto it = callbacks_.find(stream_id);
    if (it != callbacks_.end()) {
        for (const auto& callback : it->second) {
            callback(data);
        }
    }
}

std::string InputManager::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

} // namespace parsec