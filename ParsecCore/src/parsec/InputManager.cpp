#include "parsec/InputManager.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <thread>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace parsec {

// WebSocket client implementation using websocketpp
class InputManager::WebSocketClient {
public:
    using client = websocketpp::client<websocketpp::config::asio_client>;
    using message_ptr = client::message_ptr;
    using connection_hdl = websocketpp::connection_hdl;
    
    WebSocketClient(InputManager* manager) : manager_(manager), connected_(false) {
        // Initialize websocket client
        client_.clear_access_channels(websocketpp::log::alevel::all);
        client_.clear_error_channels(websocketpp::log::elevel::all);
        
        client_.init_asio();
        client_.set_reuse_addr(true);
        
        // Set handlers
        client_.set_open_handler([this](connection_hdl hdl) {
            std::cout << "WebSocket connection opened" << std::endl;
            connected_ = true;
            connection_hdl_ = hdl;
            
            // Subscribe to physics simulation channel
            json subscribe_msg = {
                {"type", "subscribe"},
                {"channel", "physics"},
                {"simulation_id", manager_->getSimulationId()}
            };
            
            client_.send(hdl, subscribe_msg.dump(), websocketpp::frame::opcode::text);
            
            // Notify the manager that connection is ready
            if (manager_) {
                manager_->onWebSocketConnected();
            }
        });
        
        client_.set_close_handler([this](connection_hdl hdl) {
            std::cout << "WebSocket connection closed" << std::endl;
            connected_ = false;
        });
        
        client_.set_message_handler([this](connection_hdl hdl, message_ptr msg) {
            if (manager_) {
                manager_->processMessage(msg->get_payload());
            }
        });
        
        client_.set_fail_handler([this](connection_hdl hdl) {
            std::cout << "WebSocket connection failed" << std::endl;
            connected_ = false;
        });
    }
    
    ~WebSocketClient() {
        disconnect();
    }
    
    bool connect(const std::string& url) {
        try {
            websocketpp::lib::error_code ec;
            client::connection_ptr con = client_.get_connection(url, ec);
            
            if (ec) {
                std::cout << "Connection error: " << ec.message() << std::endl;
                return false;
            }
            
            client_.connect(con);
            
            // Start the client thread
            client_thread_ = std::thread([this]() {
                client_.run();
            });
            
            // Wait for connection to be established
            int wait_attempts = 0;
            while (!connected_ && wait_attempts < 50) {  // Wait up to 5 seconds
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                wait_attempts++;
            }
            
            if (connected_) {
                std::cout << "WebSocket connection established successfully!" << std::endl;
            } else {
                std::cout << "WebSocket connection timeout - unable to establish connection" << std::endl;
            }
            
            return connected_;
        } catch (const std::exception& e) {
            std::cout << "WebSocket connection exception: " << e.what() << std::endl;
            return false;
        }
    }
    
    void disconnect() {
        if (connected_) {
            client_.close(connection_hdl_, websocketpp::close::status::normal, "Disconnecting");
            connected_ = false;
        }
        
        client_.stop();
        
        if (client_thread_.joinable()) {
            client_thread_.join();
        }
    }
    
    bool send(const std::string& message) {
        if (!connected_) {
            return false;
        }
        
        try {
            client_.send(connection_hdl_, message, websocketpp::frame::opcode::text);
            return true;
        } catch (const std::exception& e) {
            std::cout << "WebSocket send error: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool isConnected() const {
        return connected_;
    }
    
private:
    InputManager* manager_;
    client client_;
    connection_hdl connection_hdl_;
    std::atomic<bool> connected_;
    std::thread client_thread_;
};

InputManager::InputManager(const std::string& simulation_id) 
    : simulation_id_(simulation_id), connected_(false), running_(false) {
    ws_client_ = std::make_unique<WebSocketClient>(this);
}

InputManager::~InputManager() {
    disconnect();
}

bool InputManager::initialize(const std::string& ws_url) {
    if (connected_) {
        std::cout << "InputManager already initialized" << std::endl;
        return true;
    }
    
    std::cout << "Initializing InputManager for simulation: " << simulation_id_ << std::endl;
    std::cout << "WebSocket URL: " << ws_url << std::endl;
    
    return ws_client_->connect(ws_url);
}

bool InputManager::connect() {
    if (!ws_client_) {
        std::cout << "WebSocket client not initialized" << std::endl;
        return false;
    }
    
    // The WebSocket connection should already be established by initialize()
    // Just verify and set our connection status
    connected_ = ws_client_->isConnected();
    std::cout << "[DEBUG] InputManager::connect() - ws_client_->isConnected()=" << ws_client_->isConnected() << ", connected_=" << connected_ << std::endl;
    
    if (connected_) {
        std::cout << "InputManager connected successfully!" << std::endl;
        
        // Register this simulation with the stream handler
        json register_msg = {
            {"type", "physics_simulation"},
            {"action", "register"},
            {"simulation_id", simulation_id_},
            {"config", {
                {"name", "ParsecCore Simulation"},
                {"status", "initializing"}
            }}
        };
        
        bool sent = ws_client_->send(register_msg.dump());
        std::cout << "[DEBUG] Simulation registration sent: " << sent << std::endl;
        
        // Start message processing
        running_ = true;
        message_thread_ = std::thread(&InputManager::messageLoop, this);
    } else {
        std::cout << "InputManager connection failed - WebSocket not connected" << std::endl;
    }
    
    return connected_;
}

void InputManager::disconnect() {
    running_ = false;
    
    if (message_thread_.joinable()) {
        message_thread_.join();
    }
    
    if (ws_client_) {
        ws_client_->disconnect();
    }
    
    connected_ = false;
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
    
    // Generate timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    data.timestamp = oss.str();
    
    input_data_[stream_id] = data;
    
    // Send stream registration to stream handler
    bool ws_connected = (ws_client_ && ws_client_->isConnected());
    std::cout << "[DEBUG] Attempting to send stream registration. connected_=" << connected_ << ", ws_connected=" << ws_connected << std::endl;
    if (connected_ && ws_connected) {
        json stream_msg = {
            {"type", "physics_simulation"},
            {"action", "register_stream"},
            {"simulation_id", simulation_id_},
            {"stream_id", stream_id},
            {"stream_data", {
                {"name", name},
                {"datatype", datatype},
                {"unit", unit},
                {"value", 0.0},
                {"status", "active"},
                {"timestamp", data.timestamp}
            }}
        };
        
        std::cout << "[DEBUG] Sending stream registration: " << stream_msg.dump() << std::endl;
        bool sent = ws_client_->send(stream_msg.dump());
        std::cout << "[DEBUG] Stream registration sent: " << sent << std::endl;
    } else {
        std::cout << "[DEBUG] Not connected, skipping stream registration" << std::endl;
    }
    
    std::cout << "Registered stream: " << stream_id << " (" << name << ")" << std::endl;
    return true;
}

bool InputManager::updateStreamValue(const std::string& stream_id, double value) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    auto it = input_data_.find(stream_id);
    if (it == input_data_.end()) {
        std::cout << "Stream not found: " << stream_id << std::endl;
        return false;
    }
    
    it->second.value = value;
    
    // Generate timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    it->second.timestamp = oss.str();
    
    // Send update to stream handler
    bool ws_connected = (ws_client_ && ws_client_->isConnected());
    if (connected_ && ws_connected) {
        json update_msg = {
            {"type", "physics_simulation"},
            {"action", "update"},
            {"simulation_id", simulation_id_},
            {"stream_id", stream_id},
            {"data", {
                {"value", value},
                {"timestamp", it->second.timestamp}
            }}
        };
        
        std::cout << "[DEBUG] Sending update: " << stream_id << " = " << value << std::endl;
        ws_client_->send(update_msg.dump());
    }
    
    // Notify callbacks
    notifyCallbacks(stream_id, it->second);
    
    return true;
}

bool InputManager::updateStreamVectorValue(const std::string& stream_id, const std::vector<double>& values) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    auto it = input_data_.find(stream_id);
    if (it == input_data_.end()) {
        std::cout << "Stream not found: " << stream_id << std::endl;
        return false;
    }
    
    it->second.vector_value = values;
    
    // Generate timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    it->second.timestamp = oss.str();
    
    // Send update to stream handler
    bool ws_connected = (ws_client_ && ws_client_->isConnected());
    if (connected_ && ws_connected) {
        json update_msg = {
            {"type", "physics_simulation"},
            {"action", "update"},
            {"simulation_id", simulation_id_},
            {"stream_id", stream_id},
            {"data", {
                {"vector_value", values},
                {"timestamp", it->second.timestamp}
            }}
        };
        
        ws_client_->send(update_msg.dump());
    }
    
    // Notify callbacks
    notifyCallbacks(stream_id, it->second);
    
    return true;
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
    
    return {};
}

bool InputManager::registerCallback(const std::string& stream_id, InputCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    
    callbacks_[stream_id].push_back(callback);
    
    std::cout << "Registered callback for stream: " << stream_id << std::endl;
    return true;
}

bool InputManager::sendCommand(const std::string& command, const std::map<std::string, std::string>& params) {
    if (!connected_) {
        std::cout << "Not connected to stream handler" << std::endl;
        return false;
    }
    
    json command_msg = {
        {"type", "physics_simulation"},
        {"action", "control"},
        {"simulation_id", simulation_id_},
        {"command", command},
        {"params", params}
    };
    
    return ws_client_->send(command_msg.dump());
}

bool InputManager::updateStatus(const std::string& status) {
    if (!connected_) {
        std::cout << "Not connected to stream handler" << std::endl;
        return false;
    }
    
    json status_msg = {
        {"type", "physics_simulation"},
        {"action", "status"},
        {"simulation_id", simulation_id_},
        {"status", status}
    };
    
    return ws_client_->send(status_msg.dump());
}

bool InputManager::sendTestMessage(const std::string& message) {
    bool ws_connected = (ws_client_ && ws_client_->isConnected());
    std::cout << "[DEBUG] sendTestMessage called. ws_connected=" << ws_connected << std::endl;
    
    if (ws_connected) {
        std::cout << "[DEBUG] Sending test message: " << message << std::endl;
        bool sent = ws_client_->send(message);
        std::cout << "[DEBUG] Test message sent: " << sent << std::endl;
        return sent;
    } else {
        std::cout << "[DEBUG] Not connected, cannot send test message" << std::endl;
        return false;
    }
}

void InputManager::processMessage(const std::string& message) {
    try {
        json msg = json::parse(message);
        
        std::string type = msg.value("type", "");
        
        if (type == "physics_simulation") {
            std::string action = msg.value("action", "");
            std::string sim_id = msg.value("simulation_id", "");
            
            // Only process messages for our simulation
            if (sim_id != simulation_id_) {
                return;
            }
            
            if (action == "control") {
                std::string command = msg.value("command", "");
                std::cout << "Received command: " << command << std::endl;
                
                // Handle simulation control commands
                if (command == "start") {
                    updateStatus("running");
                } else if (command == "pause") {
                    updateStatus("paused");
                } else if (command == "stop") {
                    updateStatus("stopped");
                } else if (command == "reset") {
                    updateStatus("reset");
                }
            } else if (action == "parameter_update") {
                // Handle parameter updates from UI
                std::string stream_id = msg.value("stream_id", "");
                if (msg.contains("data") && msg["data"].contains("value")) {
                    double value = msg["data"]["value"];
                    
                    std::lock_guard<std::mutex> lock(data_mutex_);
                    auto it = input_data_.find(stream_id);
                    if (it != input_data_.end()) {
                        it->second.value = value;
                        notifyCallbacks(stream_id, it->second);
                        std::cout << "Updated parameter " << stream_id << " = " << value << std::endl;
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cout << "Error processing message: " << e.what() << std::endl;
    }
}

void InputManager::messageLoop() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        // Message processing is handled by websocketpp callbacks
    }
}

void InputManager::onWebSocketConnected() {
    std::cout << "[DEBUG] InputManager::onWebSocketConnected() called" << std::endl;
    connected_ = true;
}

void InputManager::notifyCallbacks(const std::string& stream_id, const InputData& data) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    
    auto it = callbacks_.find(stream_id);
    if (it != callbacks_.end()) {
        for (const auto& callback : it->second) {
            try {
                callback(data);
            } catch (const std::exception& e) {
                std::cout << "Error in callback for stream " << stream_id << ": " << e.what() << std::endl;
            }
        }
    }
}

} // namespace parsec