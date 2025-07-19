#include "parsec/InputManager.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <ctime>

using namespace parsec;

int main() {
    std::cout << "=== Minimal WebSocket Test Client ===" << std::endl;
    std::cout << "This test will:" << std::endl;
    std::cout << "1. Connect to ws://localhost:3000 using InputManager" << std::endl;
    std::cout << "2. Send a test message directly" << std::endl;
    std::cout << "3. Wait 5 seconds" << std::endl;
    std::cout << "4. Disconnect" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Make sure the Stream Handler is running on port 3000!" << std::endl;
    std::cout << "Press Enter to start test..." << std::endl;
    std::cin.get();
    
    // Create InputManager
    InputManager input_manager("minimal_test");
    
    std::cout << "[CLIENT] Initializing InputManager..." << std::endl;
    if (!input_manager.initialize("ws://localhost:3000")) {
        std::cout << "[CLIENT] Failed to initialize!" << std::endl;
        return 1;
    }
    
    std::cout << "[CLIENT] Connecting to Stream Handler..." << std::endl;
    if (!input_manager.connect()) {
        std::cout << "[CLIENT] Failed to connect!" << std::endl;
        return 1;
    }
    
    std::cout << "[CLIENT] Connected successfully!" << std::endl;
    
    // Send test messages
    std::cout << "[CLIENT] Sending test messages..." << std::endl;
    for (int i = 0; i < 3; i++) {
        std::string test_msg = R"({"type":"physics_simulation","action":"test","simulation_id":"minimal_test","test_number":)" + std::to_string(i + 1) + R"(,"message":"Hello from minimal test!","timestamp":)" + std::to_string(std::time(nullptr)) + "}";
        
        bool sent = input_manager.sendTestMessage(test_msg);
        std::cout << "[CLIENT] Test message " << (i + 1) << " sent: " << (sent ? "SUCCESS" : "FAILED") << std::endl;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
    // Wait for responses
    std::cout << "[CLIENT] Waiting 5 seconds for any responses..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // Disconnect
    std::cout << "[CLIENT] Disconnecting..." << std::endl;
    input_manager.disconnect();
    
    std::cout << "[CLIENT] Test finished." << std::endl;
    return 0;
}