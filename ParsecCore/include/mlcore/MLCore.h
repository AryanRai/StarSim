#pragma once

#include "modules/DataCollection.h"
#include "modules/LinearRegression.h"
#include "modules/PIDOptimizer.h"
#include "MLDataStructures.h"
#include "parsec/ModelConfig.h"
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

namespace parsec {

// Forward declarations
using SimulationState = std::map<std::string, double>;

// Forward declarations for callback types from MLDataStructures.h

class MLCore {
public:
    explicit MLCore(const ModelConfig& config);

    // Configuration
    bool loadMLConfiguration(const std::string& ml_config_path);
    bool isEnabled() const;

    // Data collection
    void collectData(const SimulationState& currentState, double timestamp);
    void processCollectedData();
    const std::vector<MLDataPoint>& getCollectedData() const;
    void clearCollectedData();

    // Model training and prediction
    bool trainModel(const std::string& model_name);
    MLPrediction predict(const std::string& model_name, const SimulationState& currentState);

    // Model management
    bool saveModel(const std::string& model_name, const std::string& path);
    bool loadModel(const std::string& model_name, const std::string& path);
    std::vector<std::string> getAvailableModels() const;

    // PID optimization
    std::map<std::string, double> optimizePIDParameters(
        const std::string& control_variable,
        const std::string& setpoint_variable,
        const std::vector<MLDataPoint>& training_data);

    // Callbacks
    void setTrainingCallback(std::function<void(const std::string&, int, double)> callback);
    void setPredictionCallback(MLPredictionCallback callback);

    // Settings
    void setDataCollectionInterval(double interval_seconds);
    void setMaxDataPoints(size_t max_points);

private:
    // Configuration
    ModelConfig modelConfig_;
    std::vector<MLModelConfig> mlModels_;
    bool enabled_;
    double dataCollectionInterval_;
    size_t maxDataPoints_;

    // Data storage
    std::vector<MLDataPoint> collectedData_;
    double lastCollectionTime_;

    // Callbacks  
    std::function<void(const std::string&, int, double)> trainingCallback_;
    MLPredictionCallback predictionCallback_;

    // Helper methods
    std::vector<std::string> parseFeatureList(const std::string& feature_string) const;
    MLDataPoint createDataPoint(const SimulationState& state, double timestamp) const;
    bool validateMLModelConfig(const MLModelConfig& config) const;

    // ML algorithms
    std::map<std::string, double> simpleLinearRegression(
        const std::vector<MLDataPoint>& data,
        const std::vector<std::string>& input_features,
        const std::string& output_target) const;

    std::map<std::string, double> pidOptimizationAlgorithm(
        const std::vector<MLDataPoint>& data,
        const std::string& control_var,
        const std::string& setpoint_var) const;

    // Data preprocessing
    std::vector<MLDataPoint> preprocessData(const std::vector<MLDataPoint>& raw_data) const;
    void normalizeFeatures(std::vector<MLDataPoint>& data) const;
};

} // namespace parsec