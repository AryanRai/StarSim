#include "parsec/ParsecCore.h"
#include "platform/IPlatform.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>

// Simple Windows platform implementation for the example
class WindowsPlatform : public IPlatform {
public:
    void log(const std::string& message) override {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::cout << "[WIN] " << message << std::endl;
    }

    uint64_t getMillis() override {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }
};

int main() {
    // Create platform and core
    WindowsPlatform platform;
    ParsecCore core(&platform);

    // Load the physics model
    std::string model_path = "../tests/test_model.starmodel.json";
    if (!core.loadModel(model_path)) {
        std::cerr << "Failed to load model: " << model_path << std::endl;
        return 1;
    }

    // Load ML configuration
    std::string ml_config_path = "../tests/ml_config_example.json";
    if (core.loadMLConfiguration(ml_config_path)) {
        std::cout << "ML configuration loaded successfully!" << std::endl;
        
        // Enable ML data collection
        core.enableMLDataCollection(true);
        
        // Set up ML callbacks (optional)
        if (core.getMLCore()) {
            core.getMLCore()->setTrainingCallback([](const std::string& model_name, int epoch, double loss) {
                std::cout << "Training " << model_name << " - Epoch: " << epoch << ", Loss: " << loss << std::endl;
            });
            
            core.getMLCore()->setPredictionCallback([](const parsec::MLPrediction& prediction) {
                std::cout << "Prediction from " << prediction.model_name << " (confidence: " << prediction.confidence << ")" << std::endl;
                for (const auto& rec : prediction.recommendations) {
                    std::cout << "  " << rec.first << ": " << rec.second << std::endl;
                }
            });
        }
    } else {
        std::cout << "ML configuration not loaded, continuing without ML features" << std::endl;
    }

    // Run simulation for data collection
    std::cout << "Starting simulation with ML data collection..." << std::endl;
    std::cout << "Collecting data for 5 seconds..." << std::endl;
    
    auto start_time = std::chrono::steady_clock::now();
    auto last_log_time = start_time;
    
    while (true) {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);
        
        // Run simulation step
        core.tick();
        
        // Log state every second
        if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_log_time).count() >= 1000) {
            auto state = core.getCurrentState();
            std::cout << "State: time=" << std::fixed << std::setprecision(3) << state.at("time");
            if (state.find("x") != state.end()) {
                std::cout << ", x=" << state.at("x");
            }
            if (state.find("v") != state.end()) {
                std::cout << ", v=" << state.at("v");
            }
            std::cout << std::endl;
            last_log_time = current_time;
        }
        
        // After 5 seconds, train and test ML models
        if (elapsed.count() >= 5) {
            break;
        }
        
        // Small delay to prevent overwhelming the system
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    
    if (core.isMLEnabled()) {
        std::cout << "\n--- ML Training and Prediction Demo ---" << std::endl;
        
        // Get collected data info
        auto* mlCore = core.getMLCore();
        if (mlCore) {
            std::cout << "Collected " << mlCore->getCollectedData().size() << " data points" << std::endl;
            
            // Process collected data
            mlCore->processCollectedData();
            
            // Train the PID optimizer model
            std::cout << "\nTraining PID optimizer..." << std::endl;
            if (core.trainMLModel("spring_mass_pid_optimizer")) {
                std::cout << "PID optimizer training completed!" << std::endl;
                
                // Get PID recommendations
                auto prediction = core.runMLPrediction("spring_mass_pid_optimizer");
                if (prediction.status == "success") {
                    std::cout << "PID Recommendations:" << std::endl;
                    for (const auto& rec : prediction.recommendations) {
                        std::cout << "  " << rec.first << ": " << rec.second << std::endl;
                    }
                }
            }
            
            // Train the system predictor
            std::cout << "\nTraining system predictor..." << std::endl;
            if (core.trainMLModel("system_predictor")) {
                std::cout << "System predictor training completed!" << std::endl;
                
                // Get system predictions
                auto prediction = core.runMLPrediction("system_predictor");
                if (prediction.status == "success") {
                    std::cout << "System Predictions:" << std::endl;
                    for (const auto& pred : prediction.predictions) {
                        std::cout << "  " << pred.first << ": " << pred.second << std::endl;
                    }
                }
            }
            
            // Demonstrate PID optimization function
            std::cout << "\nDemonstrating PID parameter optimization..." << std::endl;
            auto pid_params = mlCore->optimizePIDParameters("x", "v", mlCore->getCollectedData());
            std::cout << "Optimized PID parameters:" << std::endl;
            for (const auto& param : pid_params) {
                std::cout << "  " << param.first << ": " << param.second << std::endl;
            }
        }
    }
    
    std::cout << "\n--- ML Demo Complete ---" << std::endl;
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();
    
    return 0;
} 