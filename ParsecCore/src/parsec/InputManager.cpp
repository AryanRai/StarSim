#include "parsec/InputManager.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>

// Include platform-specific WebSocket implementation
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

// Include JSON library (using nlohmann/json)
#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace parsec {

// WebSocket client implementation
class InputManager::WebSocketClient {
public:
    WebSocketClient(InputManager* manager) : manager_(manager), socket_(-1) {}
    
    ~WebSocketClient() {
        disconnect();
    }
    
    bool connect(const std::string& url) {
        // Parse URL (simple parsing, assumes format "ws://host:port")
        std::string host;
        int port = 80;
        
        size_t protocol_end = url.find("://");
        if (protocol_end != std::string::npos) {
            size_t host_start = protocol_end + 3;
            size_t port_start = url.find(":", host_start);
            
            if (port_start != std::string::npos) {
                host = url.substr(host_start, port_start - host_start);
                port = std::stoi(url.substr(port_start + 1));
            } else {
                host = url.substr(host_start);
            }
        } else {
            host = url;
        }
        
        // Initialize socket library on Windows
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "Failed to initialize Winsock" << std::endl;
            return false;
        }
#endif
        
        // Create socket
        socket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_ < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }
        
        // Connect to server
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        
#ifdef _WIN32
        InetPton(AF_INET, host.c_str(), &server_addr.sin_addr);
        if (::connect(socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
#else
        inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);
        if (::connect(socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
#endif
            std::cerr << "Connection failed" << std::endl;
            return false;
        }
        
        // Perform WebSocket handshake
        std::string handshake = 
            "GET / HTTP/1.1\r\n"
            "Host: " + host + ":" + std::to_string(port) + "\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
            "Sec-WebSocket-Version: 13\r\n"
            "\r\n";
        
#ifdef _WIN32
        if (send(socket_, handshake.c_str(), handshake.length(), 0) < 0) {
#else
        if (send(socket_, handshake.c_str(), handshake.length(), 0) < 0) {
#endif
            std::cerr << "Failed to send handshake" << std::endl;
            return false;
        }
        
        // Receive handshake response
        char buffer[1024] = {0};
#ifdef _WIN32
        if (recv(socket_, buffer, sizeof(buffer), 0) < 0) {
#else
        if (recv(socket_, buffer, sizeof(buffer), 0) < 0) {
#endif
            std::cerr << "Failed to receive handshake response" << std::endl;
            return false;
        }
        
        // Check if handshake was successful
        if (std::string(buffer).find("HTTP/1.1 101") == std::string::npos) {
            std::cerr << "WebSocket handshake failed" << std::endl;
            return false;
        }
        
        std::cout << "WebSocket connection established" << std::endl;
        return true;
    }
    
    void disconnect() {
        if (socket_ >= 0) {
#ifdef _WIN32
            closesocket(socket_);
            WSACleanup();
#else
            close(socket_);
#endif
            socket_ = -1;
        }
    }
    
    bool send(const std::string& message) {
        // Simple WebSocket frame encoding (no masking)
        std::vector<uint8_t> frame;
        
        // Add FIN and opcode (0x81 = FIN + text)
        frame.push_back(0x81);
        
        // Add payload length
        if (message.length() < 126) {
            frame.push_back(message.length());
        } else if (message.length() < 65536) {
            frame.push_back(126);
            frame.push_back((message.length() >> 8) & 0xFF);
            frame.push_back(message.length() & 0xFF);
        } else {
            frame.push_back(127);
            for (int i = 7; i >= 0; --i) {
                frame.push_back((message.length() >> (i * 8)) & 0xFF);
            }
        }
        
        // Add payload
        for (char c : message) {
            frame.push_back(c);
        }
        
#ifdef _WIN32
        if (send(socket_, (const char*)frame.data(), frame.size(), 0) < 0) {
#else
        if (send(socket_, frame.data(), frame.size(), 0) < 0) {
#endif
            std::cerr << "Failed to send message" << std::endl;
            return false;
        }
        
        return true;
    }
    
    bool receive(std::string& message) {
        // Receive WebSocket frame header
        uint8_t header[2];
#ifdef _WIN32
        if (recv(socket_, (char*)header, 2, 0) != 2) {
#else
        if (recv(socket_, header, 2, 0) != 2) {
#endif
            return false;
        }
        
        // Parse header
        bool fin = (header[0] & 0x80) != 0;
        uint8_t opcode = header[0] & 0x0F;
        bool masked = (header[1] & 0x80) != 0;
        uint64_t payload_length = header[1] & 0x7F;
        
        // Handle extended payload length
        if (payload_length == 126) {
            uint8_t length_bytes[2];
#ifdef _WIN32
            if (recv(socket_, (char*)length_bytes, 2, 0) != 2) {
#else
            if (recv(socket_, length_bytes, 2, 0) != 2) {
#endif
                return false;
            }
            payload_length = (length_bytes[0] << 8) | length_bytes[1];
        } else if (payload_length == 127) {
            uint8_t length_bytes[8];
#ifdef _WIN32
            if (recv(socket_, (char*)length_bytes, 8, 0) != 8) {
#else
            if (recv(socket_, length_bytes, 8, 0) != 8) {
#endif
                return false;
            }
            payload_length = 0;
            for (int i = 0; i < 8; ++i) {
                payload_length = (payload_length << 8) | length_bytes[i];
            }
        }
        
        // Read masking key if present
        uint8_t masking_key[4] = {0};
        if (masked) {
#ifdef _WIN32
            if (recv(socket_, (char*)masking_key, 4, 0) != 4) {
#else
            if (recv(socket_, masking_key, 4, 0) != 4) {
#endif
                return false;
            }
        }
        
        // Read payload
        message.resize(payload_length);
        size_t bytes_read = 0;
        while (bytes_read < payload_length) {
            int result;
#ifdef _WIN32
            result = recv(socket_, (char*)&message[bytes_read], payload_length - bytes_read, 0);
#else
            result = recv(socket_, &message[bytes_read], payload_length - bytes_read, 0);
#endif
            if (result <= 0) {
                return false;
            }
            bytes_read += result;
        }
        
        // Unmask payload if necessary
        if (masked) {
            for (size_t i = 0; i < payload_length; ++i) {
                message[i] ^= masking_key[i % 4];
            }
        }
        
        // Handle control frames
        if (opcode == 0x8) {
            // Close frame
            disconnect();
            return false;
        } else if (opcode == 0x9) {
            // Ping frame, respond with pong
            std::string pong_payload = message;
            message.clear();
            
            // Send pong frame
            std::vector<uint8_t> pong_frame;
            pong_frame.push_back(0x8A);  // FIN + pong opcode
            pong_frame.push_back(pong_payload.length());
            for (char c : pong_payload) {
                pong_frame.push_back(c);
            }
            
#ifdef _WIN32
            send(socket_, (const char*)pong_frame.data(), pong_frame.size(), 0);
#else
            send(socket_, pong_frame.data(), pong_frame.size(), 0);
#endif
            
            // Continue receiving
            return receive(message);
        } else if (opcode == 0xA) {
            // Pong frame, ignore
            message.clear();
            return receive(message);
        }
        
        // Process fragmented messages
        if (!fin) {
            std::string fragment;
            if (!receive(fragment)) {
                return false;
            }
            message += fragment;
        }
        
        return true;
    }
    
private:
    InputManager* manager_;
    int socket_;
};

// InputManager implementation
InputManager::InputManager(const std::string& simulation_id)
    : simulation_id_(simulation_id), connected_(false), running_(false) {
    ws_client_ = std::make_unique<WebSocketClient>(this);
}

InputManager::~InputManager() {
    disconnect();
}

bool InputManager::initialize(const std::string& ws_url) {
    std::cout << "Initializing InputManager for simulation " << simulation_id_ << std::endl;
    return connect();
}

bool InputManager::connect() {
    if (connected_) {
        return true;
    }
    
    if (!ws_client_->connect("ws://localhost:3000")) {
        std::cerr << "Failed to connect to Stream Handler" << std::endl;
        return false;
    }
    
    connected_ = true;
    running_ = true;
    
    // Start message processing thread
    message_thread_ = std::thread(&InputManager::messageLoop, this);
    
    // Register simulation with Stream Handler
    json registration = {
        {"type", "physics_simulation"},
        {"action", "register"},
        {"simulation_id", simulation_id_},
        {"config", {
            {"solver", "RK4"},
            {"dt", 0.001},
            {"max_iterations", 1000000}
        }},
        {"msg-sent-timestamp", getCurrentTimestamp()}
    };
    
    return ws_client_->send(registration.dump());
}

void InputManager::disconnect() {
    if (!connected_) {
        return;
    }
    
    running_ = false;
    
    if (message_thread_.joinable()) {
        message_thread_.join();
    }
    
    ws_client_->disconnect();
    connected_ = false;
}

bool InputManager::registerStream(const std::string& stream_id, 
                                 const std::string& name,
                                 const std::string& datatype,
                                 const std::string& unit,
                                 const std::map<std::string, std::string>& metadata) {
    if (!connected_) {
        std::cerr << "Not connected to Stream Handler" << std::endl;
        return false;
    }
    
    // Create input data entry
    InputData data;
    data.id = stream_id;
    data.name = name;
    data.datatype = datatype;
    data.unit = unit;
    data.value = 0.0;
    data.metadata = metadata;
    data.timestamp = getCurrentTimestamp();
    
    // Store locally
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        input_data_[stream_id] = data;
    }
    
    // Send registration to Stream Handler
    json metadata_json = json::object();
    for (const auto& [key, value] : metadata) {
        metadata_json[key] = value;
    }
    
    json registration = {
        {"type", "physics_simulation"},
        {"action", "update"},
        {"simulation_id", simulation_id_},
        {"stream_id", stream_id},
        {"data", {
            {"name", name},
            {"datatype", datatype},
            {"unit", unit},
            {"value", 0.0},
            {"metadata", metadata_json},
            {"timestamp", data.timestamp}
        }},
        {"msg-sent-timestamp", getCurrentTimestamp()}
    };
    
    return ws_client_->send(registration.dump());
}

bool InputManager::updateStreamValue(const std::string& stream_id, double value) {
    if (!connected_) {
        std::cerr << "Not connected to Stream Handler" << std::endl;
        return false;
    }
    
    // Update local data
    InputData data;
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        auto it = input_data_.find(stream_id);
        if (it == input_data_.end()) {
            std::cerr << "Stream " << stream_id << " not registered" << std::endl;
            return false;
        }
        
        it->second.value = value;
        it->second.timestamp = getCurrentTimestamp();
        data = it->second;
    }
    
    // Notify callbacks
    notifyCallbacks(stream_id, data);
    
    // Send update to Stream Handler
    json update = {
        {"type", "physics_simulation"},
        {"action", "update"},
        {"simulation_id", simulation_id_},
        {"stream_id", stream_id},
        {"data", {
            {"value", value},
            {"timestamp", data.timestamp}
        }},
        {"msg-sent-timestamp", getCurrentTimestamp()}
    };
    
    return ws_client_->send(update.dump());
}

bool InputManager::updateStreamVectorValue(const std::string& stream_id, const std::vector<double>& values) {
    if (!connected_) {
        std::cerr << "Not connected to Stream Handler" << std::endl;
        return false;
    }
    
    // Update local data
    InputData data;
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        auto it = input_data_.find(stream_id);
        if (it == input_data_.end()) {
            std::cerr << "Stream " << stream_id << " not registered" << std::endl;
            return false;
        }
        
        it->second.vector_value = values;
        it->second.timestamp = getCurrentTimestamp();
        data = it->second;
    }
    
    // Notify callbacks
    notifyCallbacks(stream_id, data);
    
    // Send update to Stream Handler
    json update = {
        {"type", "physics_simulation"},
        {"action", "update"},
        {"simulation_id", simulation_id_},
        {"stream_id", stream_id},
        {"data", {
            {"value", values},
            {"timestamp", data.timestamp}
        }},
        {"msg-sent-timestamp", getCurrentTimestamp()}
    };
    
    return ws_client_->send(update.dump());
}

double InputManager::getStreamValue(const std::string& stream_id, double default_value) const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    auto it = input_data_.find(stream_id);
    if (it == input_data_.end()) {
        return default_value;
    }
    
    return it->second.value;
}

std::vector<double> InputManager::getStreamVectorValue(const std::string& stream_id) const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    auto it = input_data_.find(stream_id);
    if (it == input_data_.end()) {
        return {};
    }
    
    return it->second.vector_value;
}

bool InputManager::registerCallback(const std::string& stream_id, InputCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    callbacks_[stream_id].push_back(callback);
    return true;
}

bool InputManager::sendCommand(const std::string& command, const std::map<std::string, std::string>& params) {
    if (!connected_) {
        std::cerr << "Not connected to Stream Handler" << std::endl;
        return false;
    }
    
    json params_json = json::object();
    for (const auto& [key, value] : params) {
        params_json[key] = value;
    }
    
    json command_json = {
        {"type", "physics_simulation"},
        {"action", "control"},
        {"simulation_id", simulation_id_},
        {"command", command},
        {"params", params_json},
        {"msg-sent-timestamp", getCurrentTimestamp()}
    };
    
    return ws_client_->send(command_json.dump());
}

bool InputManager::updateStatus(const std::string& status) {
    if (!connected_) {
        std::cerr << "Not connected to Stream Handler" << std::endl;
        return false;
    }
    
    json status_json = {
        {"type", "physics_simulation"},
        {"action", "status"},
        {"simulation_id", simulation_id_},
        {"status", status},
        {"msg-sent-timestamp", getCurrentTimestamp()}
    };
    
    return ws_client_->send(status_json.dump());
}

void InputManager::processMessage(const std::string& message) {
    try {
        json data = json::parse(message);
        
        // Handle physics simulation messages
        if (data["type"] == "physics_simulation") {
            std::string action = data["action"];
            
            if (action == "control") {
                // Handle control commands
                std::string command = data["command"];
                json params = data["params"];
                
                std::cout << "Received command: " << command << std::endl;
                
                // Process command (to be implemented by derived classes)
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error processing message: " << e.what() << std::endl;
    }
}

void InputManager::messageLoop() {
    while (running_) {
        if (!connected_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        std::string message;
        if (ws_client_->receive(message)) {
            processMessage(message);
        } else {
            connected_ = false;
            std::cerr << "Connection to Stream Handler lost" << std::endl;
            
            // Try to reconnect
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (connect()) {
                std::cout << "Reconnected to Stream Handler" << std::endl;
            }
        }
    }
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

// Helper function to get current timestamp
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
    
    return ss.str();
}

} // namespace parsec