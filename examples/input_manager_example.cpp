#include "parsec/InputManager.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>

/**
 * @brief Example demonstrating the use of InputManager with Stream Handler
 * 
 * This example creates a simple spring-mass system simulation and sends
 * the data to the Stream Handler for visualization.
 */
int main(int argc, char** argv) {
    // Create input manager for spring-mass simulation
    parsec::InputManager input_manager("spring_mass_system");
    
    // Initialize and connect to Stream Handler
    if (!input_manager.initialize("ws://localhost:3000")) {
        std::cerr << "Failed to initialize InputManager" << std::endl;
        return 1;
    }
    
    // Update simulation status
    input_manager.updateStatus("initializing");
    
    // Register streams
    input_manager.registerStream("position", "Position", "float", "m", {
        {"min", "-2.0"},
        {"max", "2.0"},
        {"color", "#FF4500"}
    });
    
    input_manager.registerStream("velocity", "Velocity", "float", "m/s", {
        {"min", "-10.0"},
        {"max", "10.0"},
        {"color", "#4169E1"}
    });
    
    input_manager.registerStream("acceleration", "Acceleration", "float", "m/s²", {
        {"min", "-50.0"},
        {"max", "50.0"},
        {"color", "#32CD32"}
    });
    
    input_manager.registerStream("energy", "Total Energy", "float", "J", {
        {"min", "0.0"},
        {"max", "10.0"},
        {"color", "#FFD700"}
    });
    
    input_manager.registerStream("phase_position", "Phase Space Position", "vector2", "", {
        {"coordinate_system", "phase_space"},
        {"x_label", "Position"},
        {"y_label", "Velocity"}
    });
    
    // Spring-mass system parameters
    double mass = 1.0;
    double spring_constant = 10.0;
    double damping = 0.1;
    double position = 1.0;
    double velocity = 0.0;
    double acceleration = 0.0;
    double time = 0.0;
    double dt = 0.01;
    
    // Update simulation status
    input_manager.updateStatus("running");
    
    // Simulation loop
    std::cout << "Starting simulation..." << std::endl;
    for (int i = 0; i < 3000; ++i) {  // Run for 30 seconds
        // Calculate acceleration (F = -kx - bv)
        acceleration = (-spring_constant * position - damping * velocity) / mass;
        
        // Update velocity and position using Euler integration
        velocity += acceleration * dt;
        position += velocity * dt;
        
        // Calculate energy
        double kinetic_energy = 0.5 * mass * velocity * velocity;
        double potential_energy = 0.5 * spring_constant * position * position;
        double total_energy = kinetic_energy + potential_energy;
        
        // Update streams
        input_manager.updateStreamValue("position", position);
        input_manager.updateStreamValue("velocity", velocity);
        input_manager.updateStreamValue("acceleration", acceleration);
        input_manager.updateStreamValue("energy", total_energy);
        input_manager.updateStreamVectorValue("phase_position", {position, velocity});
        
        // Update time
        time += dt;
        
        // Print status every 100 steps
        if (i % 100 == 0) {
            std::cout << "t=" << time << ", x=" << position << ", v=" << velocity << ", a=" << acceleration << std::endl;
        }
        
        // Sleep to control simulation speed
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(dt * 1000)));
    }
    
    // Update simulation status
    input_manager.updateStatus("completed");
    
    std::cout << "Simulation completed" << std::endl;
    
    // Wait a bit before disconnecting
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    return 0;
}