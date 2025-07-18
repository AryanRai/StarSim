#include "parsec/InputManager.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace parsec {

// Simple implementation without WebSocket for now
class InputManager::WebSocketClient {
public:
    WebSocketClient(InputManager* manager) : manager_(manager), connected_(false) {
        std::cout << "WebSocket client initialized (stub implementation)" << std::endl;
    }
    
    void connect(const std::string& uri) {
        std::cout << "Attempting to connect to: " << uri << std::endl;
        // For now, just simulate connection
        connected_ = true;
        std::cout << "Connected (simulated)" << std::endl;
    }
    
    void run() {
        std::cout << "WebSocket client running..." << std::endl;
        // Simple loop to keep the client alive
        while (connected_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    void close() {
        std::cout << "Closing WebSocket connection" << std::endl;
        connected_ = false;
    }
    
    void send(const std::string& message) {
        std::cout << "Sending message: " << message << std::endl;
        // For now, just log the message
    }
    
    bool is_connected() const {
        return connected_;
    }
    
private:
    InputManager* manager_;
    bool connected_;
};

InputManager::InputManager() : client_(std::make_unique<WebSocketClient>(this)) {
    std::cout << "InputManager initialized" << std::endl;
}

InputManager::~InputManager() {
    if (client_) {
        client_->close();
    }
}

void InputManager::connect(const std::string& uri) {
    if (client_) {
        client_->connect(uri);
    }
}

void InputManager::run() {
    std::cout << "InputManager running..." << std::endl;
    if (client_) {
        client_->run();
    }
}

void InputManager::close() {
    if (client_) {
        client_->close();
    }
}

void InputManager::setVariable(const std::string& name, double value) {
    variables_[name] = value;
    std::cout << "Set variable " << name << " = " << value << std::endl;
}

double InputManager::getVariable(const std::string& name) const {
    auto it = variables_.find(name);
    if (it != variables_.end()) {
        return it->second;
    }
    return 0.0;
}

void InputManager::sendStreamData(const std::string& stream_id, const std::string& name, 
                                 const std::string& datatype, const std::string& unit, 
                                 double value) {
    json stream_data = {
        {"stream_id", stream_id},
        {"name", name},
        {"datatype", datatype},
        {"unit", unit},
        {"value", value},
        {"timestamp", getCurrentTimestamp()}
    };
    
    if (client_) {
        client_->send(stream_data.dump());
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

std::string InputManager::getSimulationId() const {
    return simulation_id_;
}

void InputManager::setSimulationId(const std::string& id) {
    simulation_id_ = id;
}

} // namespace parsec