#include "parsec/InputManager.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <ctime>
#include <nlohmann/json.hpp>

using namespace parsec;
using json = nlohmann::json;

int main() {
    std::cout << "StarSim Physics Simulation Demo" << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << std::endl;
    
    // Create InputManager for spring-mass system
    InputManager input_manager("spring_mass_system");
    
    // Initialize with WebSocket URL
    input_manager.initialize("ws://localhost:3000");
    
    // Try to connect to Stream Handler
    std::cout << "Attempting to connect to Stream Handler..." << std::endl;
    if (input_manager.connect()) {
        std::cout << "Successfully connected to Stream Handler!" << std::endl;
    } else {
        std::cout << "Failed to connect to Stream Handler. Continuing in offline mode..." << std::endl;
    }
    
    // Send test messages to verify Stream Handler is receiving them
    std::cout << "\n=== Testing WebSocket Communication ===" << std::endl;
    for (int i = 0; i < 5; i++) {
        json test_msg = {
            {"type", "physics_simulation"},
            {"action", "test"},
            {"simulation_id", "spring_mass_system"},
            {"test_number", i + 1},
            {"message", "Hello from StarSim test!"},
            {"timestamp", std::time(nullptr)}
        };
        
        bool sent = input_manager.sendTestMessage(test_msg.dump());
        std::cout << "Test message " << (i + 1) << " sent: " << (sent ? "SUCCESS" : "FAILED") << std::endl;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << "=== Test Messages Complete ===\n" << std::endl;
    
    // Register physics streams
    input_manager.registerStream("position", "Position", "float", "m");
    input_manager.registerStream("velocity", "Velocity", "float", "m/s");
    input_manager.registerStream("acceleration", "Acceleration", "float", "m/s²");
    input_manager.registerStream("energy", "Total Energy", "float", "J");
    input_manager.registerStream("phase_position", "Phase Space Position", "vector2", "m,m/s");
    
    // Send initial status
    input_manager.updateStatus("starting");
    
    // Spring-mass system parameters
    double mass = 1.0;      // kg
    double spring_k = 10.0; // N/m
    double damping = 0.1;   // N·s/m
    
    // Initial conditions
    double x = 1.0;         // m (initial displacement)
    double v = 0.0;         // m/s (initial velocity)
    double dt = 0.01;       // s (time step)
    
    std::cout << "Starting simulation..." << std::endl;
    input_manager.updateStatus("running");
    
    // Run simulation for 100 time steps
    for (int i = 0; i < 100; ++i) {
        // Calculate acceleration: F = ma, F = -kx - cv
        double a = (-spring_k * x - damping * v) / mass;
        
        // Update velocity and position (simple Euler integration)
        v += a * dt;
        x += v * dt;
        
        // Calculate total energy (kinetic + potential)
        double kinetic_energy = 0.5 * mass * v * v;
        double potential_energy = 0.5 * spring_k * x * x;
        double total_energy = kinetic_energy + potential_energy;
        
        // Update streams
        input_manager.updateStreamValue("position", x);
        input_manager.updateStreamValue("velocity", v);
        input_manager.updateStreamValue("acceleration", a);
        input_manager.updateStreamValue("energy", total_energy);
        
        // Update phase space position (x, v)
        std::vector<double> phase_pos = {x, v};
        input_manager.updateStreamVectorValue("phase_position", phase_pos);
        
        // Print progress
        std::cout << "t=" << (i + 1) * dt << ", x=" << x << ", v=" << v << ", a=" << a << std::endl;
        
        // Small delay to make it visible
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    std::cout << "Simulation completed!" << std::endl;
    input_manager.updateStatus("completed");
    
    // Disconnect
    input_manager.disconnect();
    
    return 0;
}