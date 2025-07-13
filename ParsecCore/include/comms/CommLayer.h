#pragma once

#include <vector>
#include <string>
#include <map>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>

namespace parsec {

// Forward declarations
class IPlatform;

// Communication message structure
struct CommMessage {
    std::string interface_name;     // Which interface this message came from/goes to
    std::string message_type;       // "data", "command", "status", "error", "heartbeat"
    std::vector<uint8_t> payload;   // Raw message data
    std::string source_address;     // Source IP, MAC, etc.
    std::string destination_address; // Destination address
    double timestamp;               // When message was received/sent
    size_t message_id;              // Unique message identifier
    std::map<std::string, std::string> metadata; // Protocol-specific metadata
};

// Communication interface configuration
struct CommInterface {
    std::string name;               // Unique interface identifier
    std::string protocol;           // "websocket", "serial", "can", "bluetooth", "lora", "udp", "tcp"
    std::string address;            // URL, port, device path, etc.
    bool is_server = false;         // Server vs client mode (where applicable)
    bool enabled = true;
    double heartbeat_interval = 5.0; // seconds
    double connection_timeout = 10.0; // seconds
    size_t max_message_size = 4096;  // bytes
    std::map<std::string, std::string> parameters; // Protocol-specific parameters
};

// Connection status tracking
struct ConnectionStatus {
    std::string interface_name;
    bool connected = false;
    double last_activity = 0.0;
    size_t messages_sent = 0;
    size_t messages_received = 0;
    size_t bytes_sent = 0;
    size_t bytes_received = 0;
    std::string last_error;
    double connection_start_time = 0.0;
};

// Callback function types
using MessageReceivedCallback = std::function<void(const CommMessage& message)>;
using MessageSentCallback = std::function<void(const CommMessage& message, bool success)>;
using ConnectionCallback = std::function<void(const std::string& interface_name, bool connected)>;
using ErrorCallback = std::function<void(const std::string& interface_name, const std::string& error)>;

// Base interface class for different communication protocols
class ICommProtocol {
public:
    virtual ~ICommProtocol() = default;
    
    virtual bool initialize(const CommInterface& config) = 0;
    virtual bool connect() = 0;
    virtual bool disconnect() = 0;
    virtual bool isConnected() const = 0;
    
    virtual bool sendMessage(const CommMessage& message) = 0;
    virtual std::vector<CommMessage> receiveMessages() = 0;
    
    virtual ConnectionStatus getStatus() const = 0;
    virtual void updateStatus() = 0;
    
    virtual std::string getProtocolName() const = 0;
};

// Specific protocol implementations
class WebSocketProtocol : public ICommProtocol {
public:
    bool initialize(const CommInterface& config) override;
    bool connect() override;
    bool disconnect() override;
    bool isConnected() const override;
    bool sendMessage(const CommMessage& message) override;
    std::vector<CommMessage> receiveMessages() override;
    ConnectionStatus getStatus() const override;
    void updateStatus() override;
    std::string getProtocolName() const override { return "WebSocket"; }
    
private:
    CommInterface config_;
    ConnectionStatus status_;
    // Platform-specific WebSocket implementation details
    void* websocket_handle_ = nullptr;
};

class SerialProtocol : public ICommProtocol {
public:
    bool initialize(const CommInterface& config) override;
    bool connect() override;
    bool disconnect() override;
    bool isConnected() const override;
    bool sendMessage(const CommMessage& message) override;
    std::vector<CommMessage> receiveMessages() override;
    ConnectionStatus getStatus() const override;
    void updateStatus() override;
    std::string getProtocolName() const override { return "Serial"; }
    
private:
    CommInterface config_;
    ConnectionStatus status_;
    // Platform-specific serial port handle
    void* serial_handle_ = nullptr;
    std::thread read_thread_;
    std::atomic<bool> reading_ = false;
    std::queue<CommMessage> incoming_messages_;
    std::mutex message_mutex_;
};

class CANProtocol : public ICommProtocol {
public:
    bool initialize(const CommInterface& config) override;
    bool connect() override;
    bool disconnect() override;
    bool isConnected() const override;
    bool sendMessage(const CommMessage& message) override;
    std::vector<CommMessage> receiveMessages() override;
    ConnectionStatus getStatus() const override;
    void updateStatus() override;
    std::string getProtocolName() const override { return "CAN"; }
    
private:
    CommInterface config_;
    ConnectionStatus status_;
    // CAN-specific implementation
    void* can_handle_ = nullptr;
};

class BluetoothProtocol : public ICommProtocol {
public:
    bool initialize(const CommInterface& config) override;
    bool connect() override;
    bool disconnect() override;
    bool isConnected() const override;
    bool sendMessage(const CommMessage& message) override;
    std::vector<CommMessage> receiveMessages() override;
    ConnectionStatus getStatus() const override;
    void updateStatus() override;
    std::string getProtocolName() const override { return "Bluetooth"; }
    
private:
    CommInterface config_;
    ConnectionStatus status_;
    // Bluetooth-specific implementation
    void* bluetooth_handle_ = nullptr;
};

class LoRaProtocol : public ICommProtocol {
public:
    bool initialize(const CommInterface& config) override;
    bool connect() override;
    bool disconnect() override;
    bool isConnected() const override;
    bool sendMessage(const CommMessage& message) override;
    std::vector<CommMessage> receiveMessages() override;
    ConnectionStatus getStatus() const override;
    void updateStatus() override;
    std::string getProtocolName() const override { return "LoRa"; }
    
private:
    CommInterface config_;
    ConnectionStatus status_;
    // LoRa-specific implementation
    void* lora_handle_ = nullptr;
};

class UDPProtocol : public ICommProtocol {
public:
    bool initialize(const CommInterface& config) override;
    bool connect() override;
    bool disconnect() override;
    bool isConnected() const override;
    bool sendMessage(const CommMessage& message) override;
    std::vector<CommMessage> receiveMessages() override;
    ConnectionStatus getStatus() const override;
    void updateStatus() override;
    std::string getProtocolName() const override { return "UDP"; }
    
private:
    CommInterface config_;
    ConnectionStatus status_;
    // UDP socket implementation
    int socket_fd_ = -1;
    std::thread receive_thread_;
    std::atomic<bool> receiving_ = false;
    std::queue<CommMessage> incoming_messages_;
    std::mutex message_mutex_;
};

// Main Communication Layer Manager
class CommLayer {
public:
    explicit CommLayer(IPlatform* platform);
    ~CommLayer();
    
    // Configuration and setup
    bool loadCommConfiguration(const std::string& comm_config_path);
    bool addInterface(const CommInterface& interface);
    bool removeInterface(const std::string& interface_name);
    
    // Connection management
    bool connectInterface(const std::string& interface_name);
    bool disconnectInterface(const std::string& interface_name);
    void connectAll();
    void disconnectAll();
    
    // Message handling
    bool sendMessage(const std::string& interface_name, const CommMessage& message);
    bool broadcastMessage(const CommMessage& message); // Send to all connected interfaces
    std::vector<CommMessage> receiveMessages(); // Get all pending messages
    std::vector<CommMessage> receiveMessages(const std::string& interface_name); // From specific interface
    
    // Interface management
    std::vector<CommInterface> getInterfaces() const;
    CommInterface* getInterface(const std::string& interface_name);
    bool enableInterface(const std::string& interface_name, bool enable = true);
    
    // Status and monitoring
    std::vector<ConnectionStatus> getAllConnectionStatus() const;
    ConnectionStatus getConnectionStatus(const std::string& interface_name) const;
    bool isInterfaceConnected(const std::string& interface_name) const;
    
    // Callbacks
    void setMessageReceivedCallback(MessageReceivedCallback callback);
    void setMessageSentCallback(MessageSentCallback callback);
    void setConnectionCallback(ConnectionCallback callback);
    void setErrorCallback(ErrorCallback callback);
    
    // Message utilities
    CommMessage createDataMessage(const std::string& data, const std::string& destination = "");
    CommMessage createCommandMessage(const std::string& command, const std::string& destination = "");
    CommMessage createStatusMessage(const std::string& status, const std::string& destination = "");
    CommMessage createHeartbeatMessage(const std::string& destination = "");
    
    // Control
    void start();
    void stop();
    bool isRunning() const;
    
private:
    IPlatform* platform_;
    std::map<std::string, std::unique_ptr<ICommProtocol>> protocols_;
    std::map<std::string, CommInterface> interfaces_;
    
    // Threading and state
    bool running_ = false;
    std::thread worker_thread_;
    std::atomic<bool> worker_running_ = false;
    
    // Message queues
    std::queue<CommMessage> outgoing_messages_;
    std::queue<CommMessage> incoming_messages_;
    std::mutex outgoing_mutex_;
    std::mutex incoming_mutex_;
    
    // Callbacks
    MessageReceivedCallback messageReceivedCallback_;
    MessageSentCallback messageSentCallback_;
    ConnectionCallback connectionCallback_;
    ErrorCallback errorCallback_;
    
    // Internal workers
    void workerThread();
    void processOutgoingMessages();
    void processIncomingMessages();
    void sendHeartbeats();
    void checkConnections();
    
    // Protocol factory
    std::unique_ptr<ICommProtocol> createProtocol(const std::string& protocol_name);
    
    // Validation
    bool validateInterface(const CommInterface& interface) const;
    
    // Error handling
    void handleError(const std::string& interface_name, const std::string& error);
    
    // Utilities
    size_t generateMessageId();
    double getCurrentTime() const;
};

} // namespace parsec 