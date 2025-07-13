#pragma once

// Modular ML Core - Import all ML algorithm domains
#include "mlcore/MLDataStructures.h"
#include "mlcore/DataCollection.h"
#include "mlcore/PIDOptimizer.h"
#include "mlcore/LinearRegression.h"

// Forward declarations for core dependencies
#include "parsec/ModelConfig.h"
#include "parsec/SolverCore.h"

// This is the main MLCore header that provides access to all ML domains.
// You can now import specific ML concepts individually:
//
//   #include "mlcore/MLDataStructures.h"  // For common ML data structures and utilities
//   #include "mlcore/DataCollection.h"    // For data collection and preprocessing
//   #include "mlcore/PIDOptimizer.h"      // For PID parameter optimization
//   #include "mlcore/LinearRegression.h"  // For linear regression algorithms
//
// Or import everything at once:
//
//   #include "mlcore/MLCore.h"            // Imports all ML domains

namespace parsec {

// Forward declarations
class EquationManager;
class SolverCore;

// Unified MLCore class that integrates all ML functionality
class MLCore {
public:
    explicit MLCore(const ModelConfig& config);
    
    // Core ML functionality
    bool loadMLConfiguration(const std::string& ml_config_path);
    bool isEnabled() const;
    
    // Data collection and processing (delegated to DataCollector)
    void collectData(const SimulationState& currentState, double timestamp);
    void processCollectedData();
    
    // Training and inference
    bool trainModel(const std::string& model_name);
    MLPrediction predict(const std::string& model_name, const SimulationState& currentState);
    
    // Model management
    bool saveModel(const std::string& model_name, const std::string& path);
    bool loadModel(const std::string& model_name, const std::string& path);
    std::vector<std::string> getAvailableModels() const;
    
    // PID optimization specific methods (delegated to PIDOptimizer)
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
    void setValidationCallback(MLValidationCallback callback);
    void setFeatureCallback(MLFeatureCallback callback);
    
    // Configuration
    void setDataCollectionInterval(double interval_seconds);
    void setMaxDataPoints(size_t max_points);
    
    // Advanced ML functionality access
    MLAlgorithms::DataCollector& getDataCollector() { return *dataCollector_; }
    MLAlgorithms::PIDOptimizer& getPIDOptimizer() { return *pidOptimizer_; }
    MLAlgorithms::LinearRegression& getLinearRegression() { return *linearRegression_; }
    
    // Convenience methods for common tasks
    MLAlgorithms::PIDOptimizer::PIDParameters optimizePID(
        const std::string& control_variable,
        const std::string& setpoint_variable,
        const std::string& method = "genetic_algorithm"
    );
    
    MLAlgorithms::LinearRegression::RegressionResult trainRegression(
        const std::vector<std::string>& input_features,
        const std::string& target_variable,
        const std::string& algorithm = "ordinary_least_squares"
    );
    
private:
    const ModelConfig& modelConfig_;
    std::vector<MLModelConfig> mlModels_;
    
    // ML Algorithm components
    std::unique_ptr<MLAlgorithms::DataCollector> dataCollector_;
    std::unique_ptr<MLAlgorithms::PIDOptimizer> pidOptimizer_;
    std::unique_ptr<MLAlgorithms::LinearRegression> linearRegression_;
    
    // Configuration
    bool enabled_ = false;
    double lastCollectionTime_ = 0.0;
    
    // Callbacks
    MLTrainingCallback trainingCallback_;
    MLPredictionCallback predictionCallback_;
    MLValidationCallback validationCallback_;
    MLFeatureCallback featureCallback_;
    
    // Internal helpers
    std::vector<std::string> parseFeatureList(const std::string& feature_string) const;
    bool validateMLModelConfig(const MLModelConfig& config) const;
    
    // Model registry
    std::map<std::string, std::string> modelRegistry_;  // model_name -> model_type
    
    // Legacy compatibility methods
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
};

} // namespace parsec

// Example usage:
//
// #include "mlcore/PIDOptimizer.h"
// #include "mlcore/DataCollection.h"
//
// parsec::MLCore mlCore(config);
// auto& pidOptimizer = mlCore.getPIDOptimizer();
// auto pidParams = pidOptimizer.optimizeGeneticAlgorithm(data, "control", "setpoint");
//
// parsec::MLAlgorithms::DataCollector collector(config);
// collector.enableFeatureEngineering(true, true, 10); 