#ifdef _WIN32

#include "parsec/ParsecCore.h"
#include "platform/IPlatform.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <windows.h>
#include <string>
#include <filesystem> // For path manipulation
// #include "include/mathcore/math.h" // No longer needed for basic tests

// Function to get path relative to executable (adjust if needed)
std::string getModelPath(const std::string& filename) {
    // This assumes the executable is in build/Debug (or similar)
    // and the model is in tests/ relative to the source root.
    // You might need a more robust path finding mechanism.
    std::filesystem::path exe_path = std::filesystem::current_path(); // Path where executable is run (build dir)
    std::filesystem::path model_path = exe_path / ".." / "tests" / filename;
    return model_path.string();
}

class WindowsPlatform : public IPlatform {
public:
    uint64_t getMillis() override {
        return GetTickCount64();
    }
    void log(const std::string& msg) override {
        std::cout << "[WIN] " << msg << std::endl;
    }
};

int main() {
    WindowsPlatform platform;
    ParsecCore core(&platform);

    // --- Load the Test Model ---
    std::string modelFile = getModelPath("test_model.starmodel.json");
    if (!core.loadModel(modelFile)) {
        platform.log("FATAL: Could not load the model file. Exiting.");
        return 1;
    }
    // --- Model Loaded ---

    // Main simulation loop
    platform.log("Starting simulation loop...");
    double logTimer = 0.0;
    const double logInterval = 0.1; // Log state every 0.1 simulated seconds

    while (true) { // Or loop for modelConfig.solver.duration
        core.tick();

        // Log state periodically
        const auto& state = core.getCurrentState();
        if (state.count("time")) {
            double currentTime = state.at("time");
            if (currentTime >= logTimer) {
                 std::string logMsg = "State: time=" + std::to_string(currentTime);
                 if (state.count("x")) logMsg += ", x=" + std::to_string(state.at("x"));
                 if (state.count("v")) logMsg += ", v=" + std::to_string(state.at("v"));
                 platform.log(logMsg);
                 logTimer += logInterval;
            }

            // Optional: Stop after simulation duration
            // if (currentTime >= modelConfig.solver.duration) break; 
        }

        // Add a small sleep to prevent 100% CPU usage in this simple example
        // In a real application, timing would be more sophisticated (e.g., vsync, RTOS timer)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    platform.log("Simulation finished (or loop exited).");
    return 0;
}

#endif 