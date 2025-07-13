#pragma once

#include <vector>
#include <string>
#include <map>
#include <functional>

namespace parsec {

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
    
    // Utility constructors
    MLDataPoint() = default;
    MLDataPoint(double t) : timestamp(t) {}
    MLDataPoint(double t, const std::map<std::string, double>& in, const std::map<std::string, double>& out)
        : timestamp(t), inputs(in), outputs(out) {}
};

// Structure to hold ML predictions/recommendations
struct MLPrediction {
    std::string model_name;
    std::map<std::string, double> predictions;
    std::map<std::string, double> recommendations;  // e.g., optimal PID values
    double confidence = 0.0;
    std::string status = "success";
    double prediction_timestamp = 0.0;
    
    // Utility methods
    bool isValid() const { return status == "success"; }
    bool hasRecommendation(const std::string& key) const { 
        return recommendations.find(key) != recommendations.end(); 
    }
    bool hasPrediction(const std::string& key) const { 
        return predictions.find(key) != predictions.end(); 
    }
};

// Training progress information
struct MLTrainingProgress {
    std::string model_name;
    int current_epoch;
    int total_epochs;
    double current_loss;
    double validation_loss;
    double learning_rate;
    std::vector<double> loss_history;
    std::vector<double> validation_loss_history;
    bool converged = false;
    double training_time_seconds = 0.0;
};

// Model performance metrics
struct MLModelMetrics {
    std::string model_name;
    std::string metric_type;  // "regression", "classification", "optimization"
    
    // Regression metrics
    double mean_squared_error = 0.0;
    double root_mean_squared_error = 0.0;
    double mean_absolute_error = 0.0;
    double r_squared = 0.0;
    double adjusted_r_squared = 0.0;
    
    // Classification metrics (if applicable)
    double accuracy = 0.0;
    double precision = 0.0;
    double recall = 0.0;
    double f1_score = 0.0;
    
    // Custom metrics
    std::map<std::string, double> custom_metrics;
};

// Feature importance information
struct MLFeatureImportance {
    std::string feature_name;
    double importance_score;
    double coefficient = 0.0;  // For linear models
    double p_value = 1.0;      // Statistical significance
    std::string importance_type; // "coefficient", "permutation", "gain", etc.
};

// Data preprocessing configuration
struct MLPreprocessingConfig {
    bool normalize_features = false;
    bool standardize_features = true;
    bool remove_outliers = false;
    double outlier_threshold = 3.0;  // z-score threshold
    bool handle_missing_values = true;
    std::string missing_value_strategy = "mean";  // "mean", "median", "zero", "drop"
    
    // Feature selection
    bool enable_feature_selection = false;
    int max_features = -1;  // -1 for no limit
    std::string feature_selection_method = "variance";  // "variance", "correlation", "importance"
    
    // Time series specific
    int window_size = 1;
    int prediction_horizon = 1;
    bool include_derivatives = false;
};

// Cross-validation configuration
struct MLCrossValidationConfig {
    bool enabled = false;
    int k_folds = 5;
    std::string validation_method = "k_fold";  // "k_fold", "time_series", "holdout"
    double validation_split = 0.2;  // For holdout method
    bool shuffle_data = true;
    int random_seed = 42;
};

// Hyperparameter optimization configuration
struct MLHyperparameterConfig {
    bool enabled = false;
    std::string optimization_method = "grid_search";  // "grid_search", "random_search", "bayesian"
    int max_iterations = 100;
    std::map<std::string, std::vector<double>> parameter_ranges;
    std::string optimization_metric = "mse";  // Metric to optimize
    bool maximize_metric = false;  // true for accuracy, false for loss
};

// Model ensemble configuration
struct MLEnsembleConfig {
    bool enabled = false;
    std::string ensemble_method = "voting";  // "voting", "stacking", "bagging", "boosting"
    std::vector<std::string> base_models;
    std::map<std::string, double> model_weights;
    int num_base_models = 3;
};

// Callback function types for ML events
using MLTrainingCallback = std::function<void(const MLTrainingProgress& progress)>;
using MLPredictionCallback = std::function<void(const MLPrediction& prediction)>;
using MLValidationCallback = std::function<void(const MLModelMetrics& metrics)>;
using MLFeatureCallback = std::function<void(const std::vector<MLFeatureImportance>& features)>;

// Data filtering and validation
namespace MLDataUtils {
    // Data validation
    bool isValidDataPoint(const MLDataPoint& point);
    bool hasRequiredFeatures(const MLDataPoint& point, const std::vector<std::string>& required_features);
    
    // Data filtering
    std::vector<MLDataPoint> filterByTimeRange(const std::vector<MLDataPoint>& data, double start_time, double end_time);
    std::vector<MLDataPoint> filterByFeatures(const std::vector<MLDataPoint>& data, const std::vector<std::string>& required_features);
    std::vector<MLDataPoint> removeOutliers(const std::vector<MLDataPoint>& data, const std::string& feature, double threshold = 3.0);
    
    // Data transformation
    std::vector<MLDataPoint> normalizeFeatures(const std::vector<MLDataPoint>& data, const std::vector<std::string>& features);
    std::vector<MLDataPoint> standardizeFeatures(const std::vector<MLDataPoint>& data, const std::vector<std::string>& features);
    
    // Feature extraction
    std::vector<std::string> extractFeatureNames(const std::vector<MLDataPoint>& data);
    std::vector<std::string> extractTargetNames(const std::vector<MLDataPoint>& data);
    
    // Data splitting
    struct DataSplit {
        std::vector<MLDataPoint> training_data;
        std::vector<MLDataPoint> validation_data;
        std::vector<MLDataPoint> test_data;
    };
    
    DataSplit splitData(const std::vector<MLDataPoint>& data, double train_ratio = 0.7, double validation_ratio = 0.2);
    DataSplit splitDataTimeAware(const std::vector<MLDataPoint>& data, double train_ratio = 0.7, double validation_ratio = 0.2);
    
    // Feature engineering
    std::vector<MLDataPoint> addTimeDerivatives(const std::vector<MLDataPoint>& data, const std::vector<std::string>& features);
    std::vector<MLDataPoint> addRollingStatistics(const std::vector<MLDataPoint>& data, const std::vector<std::string>& features, int window_size);
    std::vector<MLDataPoint> addLaggedFeatures(const std::vector<MLDataPoint>& data, const std::vector<std::string>& features, int max_lag);
}

// Model configuration factory
namespace MLConfigFactory {
    MLModelConfig createPIDOptimizerConfig(const std::string& model_name, 
                                          const std::string& control_variable, 
                                          const std::string& setpoint_variable);
    
    MLModelConfig createSystemPredictorConfig(const std::string& model_name,
                                             const std::vector<std::string>& input_features,
                                             const std::vector<std::string>& output_targets,
                                             double prediction_horizon = 1.0);
    
    MLModelConfig createRegressionConfig(const std::string& model_name,
                                        const std::vector<std::string>& input_features,
                                        const std::vector<std::string>& output_targets);
    
    MLPreprocessingConfig createDefaultPreprocessingConfig();
    MLCrossValidationConfig createDefaultCVConfig();
    MLHyperparameterConfig createDefaultHyperparameterConfig();
}

} // namespace parsec 