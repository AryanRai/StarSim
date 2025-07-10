#pragma once

#include "parsec/ModelConfig.h"
#include "parsec/SolverCore.h"
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <functional>
#include <optional>

namespace parsec {

// Forward declarations
class EquationManager;
class SolverCore;

// Structure to hold ML model configuration
struct MLModelConfig {
    std::string model_name;
    std::string model_type;        // "pid_optimizer", "system_identifier", "predictor", etc.
    std::string input_features;    // Comma-separated list of input variables
    std::string output_targets;    // Comma-separated list of output variables
    std::map<std::string, double> hyperparameters;
    std::string model_path;        // Path to saved model file (optional)
    bool enabled = true;
};

// Structure to hold training/inference data
struct MLDataPoint {
    double timestamp;
    std::map<std::string, double> inputs;
    std::map<std::string, double> outputs;
    std::map<std::string, double> targets;  // For supervised learning
};

// Structure to hold ML predictions/recommendations
struct MLPrediction {
    std::string model_name;
    std::map<std::string, double> predictions;
    std::map<std::string, double> recommendations;  // e.g., optimal PID values
    double confidence = 0.0;
    std::string status = "success";
};

// Callback function types for ML events
using MLTrainingCallback = std::function<void(const std::string& model_name, int epoch, double loss)>;
using MLPredictionCallback = std::function<void(const MLPrediction& prediction)>;

class MLCore {
public:
    explicit MLCore(const ModelConfig& config);
    
    // Core ML functionality
    bool loadMLConfiguration(const std::string& ml_config_path);
    bool isEnabled() const;
    
    // Data collection and processing
    void collectData(const SimulationState& currentState, double timestamp);
    void processCollectedData();
    
    // Training and inference
    bool trainModel(const std::string& model_name);
    MLPrediction predict(const std::string& model_name, const SimulationState& currentState);
    
    // Model management
    bool saveModel(const std::string& model_name, const std::string& path);
    bool loadModel(const std::string& model_name, const std::string& path);
    std::vector<std::string> getAvailableModels() const;
    
    // PID optimization specific methods
    std::map<std::string, double> optimizePIDParameters(
        const std::string& control_variable,
        const std::string& setpoint_variable,
        const std::vector<MLDataPoint>& training_data
    );
    
    // Data access
    const std::vector<MLDataPoint>& getCollectedData() const;
    void clearCollectedData();
    
    // Callbacks
    void setTrainingCallback(MLTrainingCallback callback);
    void setPredictionCallback(MLPredictionCallback callback);
    
    // Configuration
    void setDataCollectionInterval(double interval_seconds);
    void setMaxDataPoints(size_t max_points);
    
private:
    const ModelConfig& modelConfig_;
    std::vector<MLModelConfig> mlModels_;
    std::vector<MLDataPoint> collectedData_;
    
    // Configuration
    bool enabled_ = false;
    double dataCollectionInterval_ = 0.1;  // seconds
    size_t maxDataPoints_ = 10000;
    double lastCollectionTime_ = 0.0;
    
    // Callbacks
    MLTrainingCallback trainingCallback_;
    MLPredictionCallback predictionCallback_;
    
    // Internal helpers
    std::vector<std::string> parseFeatureList(const std::string& feature_string) const;
    MLDataPoint createDataPoint(const SimulationState& state, double timestamp) const;
    bool validateMLModelConfig(const MLModelConfig& config) const;
    
    // ML algorithm implementations (basic versions)
    std::map<std::string, double> simpleLinearRegression(
        const std::vector<MLDataPoint>& data,
        const std::vector<std::string>& input_features,
        const std::string& output_target
    ) const;
    
    std::map<std::string, double> pidOptimizationAlgorithm(
        const std::vector<MLDataPoint>& data,
        const std::string& control_var,
        const std::string& setpoint_var
    ) const;
    
    // Data preprocessing
    std::vector<MLDataPoint> preprocessData(const std::vector<MLDataPoint>& raw_data) const;
    void normalizeFeatures(std::vector<MLDataPoint>& data) const;
};

} // namespace parsec 