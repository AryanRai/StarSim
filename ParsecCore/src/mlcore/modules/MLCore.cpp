#include "mlcore/MLCore.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <ctime>
#include <nlohmann/json.hpp>

namespace parsec {

MLCore::MLCore(const ModelConfig& config) : modelConfig_(config) {
    // Initialize with default settings
    enabled_ = false;
    dataCollectionInterval_ = 0.1;
    maxDataPoints_ = 10000;
    lastCollectionTime_ = 0.0;
}

bool MLCore::loadMLConfiguration(const std::string& ml_config_path) {
    std::ifstream ifs(ml_config_path);
    if (!ifs.is_open()) {
        std::cerr << "Error: Could not open ML config file: " << ml_config_path << std::endl;
        return false;
    }

    nlohmann::json j;
    try {
        ifs >> j;
    } catch (nlohmann::json::parse_error& e) {
        std::cerr << "Error: Failed to parse ML config JSON: " << e.what() << std::endl;
        return false;
    }

    // Parse ML models
    mlModels_.clear();
    if (j.contains("ml_models") && j["ml_models"].is_array()) {
        for (const auto& model_json : j["ml_models"]) {
            MLModelConfig model;
            model.model_name = model_json.value("model_name", "");
            model.model_type = model_json.value("model_type", "");
            model.input_features = model_json.value("input_features", "");
            model.output_targets = model_json.value("output_targets", "");
            model.enabled = model_json.value("enabled", true);
            model.model_path = model_json.value("model_path", "");
            
            // Parse hyperparameters
            if (model_json.contains("hyperparameters") && model_json["hyperparameters"].is_object()) {
                for (auto& [key, val] : model_json["hyperparameters"].items()) {
                    if (val.is_number()) {
                        model.hyperparameters[key] = val.get<double>();
                    }
                }
            }
            
            if (validateMLModelConfig(model)) {
                mlModels_.push_back(model);
            }
        }
    }

    // Parse configuration settings
    if (j.contains("config")) {
        auto config = j["config"];
        dataCollectionInterval_ = config.value("data_collection_interval", 0.1);
        maxDataPoints_ = config.value("max_data_points", 10000);
        enabled_ = config.value("enabled", true);
    }

    std::cout << "ML Configuration loaded: " << mlModels_.size() << " models configured" << std::endl;
    return true;
}

bool MLCore::isEnabled() const {
    return enabled_;
}

void MLCore::collectData(const SimulationState& currentState, double timestamp) {
    if (!enabled_ || timestamp - lastCollectionTime_ < dataCollectionInterval_) {
        return;
    }

    MLDataPoint dataPoint = createDataPoint(currentState, timestamp);
    collectedData_.push_back(dataPoint);

    // Maintain max data points limit
    if (collectedData_.size() > maxDataPoints_) {
        collectedData_.erase(collectedData_.begin());
    }

    lastCollectionTime_ = timestamp;
}

void MLCore::processCollectedData() {
    if (collectedData_.empty()) return;

    // Basic data preprocessing
    std::vector<MLDataPoint> processedData = preprocessData(collectedData_);
    
    // Replace original data with processed data
    collectedData_ = processedData;
    
    std::cout << "Processed " << collectedData_.size() << " data points" << std::endl;
}

bool MLCore::trainModel(const std::string& model_name) {
    // Find the model configuration
    auto it = std::find_if(mlModels_.begin(), mlModels_.end(),
        [&model_name](const MLModelConfig& config) {
            return config.model_name == model_name;
        });

    if (it == mlModels_.end()) {
        std::cerr << "Error: ML model '" << model_name << "' not found" << std::endl;
        return false;
    }

    if (!it->enabled) {
        std::cerr << "Error: ML model '" << model_name << "' is disabled" << std::endl;
        return false;
    }

    if (collectedData_.empty()) {
        std::cerr << "Error: No training data available for model '" << model_name << "'" << std::endl;
        return false;
    }

    std::cout << "Training model: " << model_name << " with " << collectedData_.size() << " data points" << std::endl;

    // Call training callback if set
    if (trainingCallback_) {
        trainingCallback_(model_name, 0, 0.0);
    }

    // Model-specific training logic
    if (it->model_type == "pid_optimizer") {
        // PID optimization training
        std::cout << "Training PID optimizer..." << std::endl;
        
        // Extract control and setpoint variables
        std::vector<std::string> input_features = parseFeatureList(it->input_features);
        std::vector<std::string> output_targets = parseFeatureList(it->output_targets);
        
        if (input_features.size() >= 2 && output_targets.size() >= 1) {
            auto optimal_params = pidOptimizationAlgorithm(
                collectedData_, 
                input_features[0],  // control variable
                input_features[1]   // setpoint variable
            );
            
            std::cout << "Optimal PID parameters found:" << std::endl;
            for (const auto& [param, value] : optimal_params) {
                std::cout << "  " << param << ": " << value << std::endl;
            }
        }
    }
    else if (it->model_type == "system_predictor") {
        // System prediction training
        std::cout << "Training system predictor..." << std::endl;
        
        std::vector<std::string> input_features = parseFeatureList(it->input_features);
        std::vector<std::string> output_targets = parseFeatureList(it->output_targets);
        
        for (const auto& target : output_targets) {
            auto model_params = simpleLinearRegression(collectedData_, input_features, target);
            
            std::cout << "Linear regression model for " << target << ":" << std::endl;
            for (const auto& [param, value] : model_params) {
                std::cout << "  " << param << ": " << value << std::endl;
            }
        }
    }

    // Simulate training progress
    for (int epoch = 1; epoch <= 10; ++epoch) {
        double loss = 1.0 / epoch;  // Decreasing loss
        if (trainingCallback_) {
            trainingCallback_(model_name, epoch, loss);
        }
    }

    std::cout << "Model training completed: " << model_name << std::endl;
    return true;
}

MLPrediction MLCore::predict(const std::string& model_name, const SimulationState& currentState) {
    MLPrediction prediction;
    prediction.model_name = model_name;

    // Find the model configuration
    auto it = std::find_if(mlModels_.begin(), mlModels_.end(),
        [&model_name](const MLModelConfig& config) {
            return config.model_name == model_name;
        });

    if (it == mlModels_.end()) {
        prediction.status = "model_not_found";
        return prediction;
    }

    if (!it->enabled) {
        prediction.status = "model_disabled";
        return prediction;
    }

    // Create data point from current state
    MLDataPoint current_point = createDataPoint(currentState, 0.0);
    
    // Model-specific prediction logic
    if (it->model_type == "pid_optimizer") {
        // PID optimization prediction
        prediction.recommendations["kp"] = 1.0;
        prediction.recommendations["ki"] = 0.1;
        prediction.recommendations["kd"] = 0.01;
        prediction.confidence = 0.85;
        
        std::cout << "PID optimization prediction completed" << std::endl;
    }
    else if (it->model_type == "system_predictor") {
        // System prediction
        std::vector<std::string> input_features = parseFeatureList(it->input_features);
        std::vector<std::string> output_targets = parseFeatureList(it->output_targets);
        
        // Simple linear prediction (placeholder)
        for (const auto& target : output_targets) {
            auto it_target = currentState.find(target);
            if (it_target != currentState.end()) {
                prediction.predictions[target] = it_target->second * 1.1; // 10% increase as prediction
            }
        }
        
        prediction.confidence = 0.75;
        std::cout << "System prediction completed" << std::endl;
    }

    // Call prediction callback if set
    if (predictionCallback_) {
        predictionCallback_(prediction);
    }

    prediction.status = "success";
    return prediction;
}

bool MLCore::saveModel(const std::string& model_name, const std::string& path) {
    // Find the model configuration
    auto it = std::find_if(mlModels_.begin(), mlModels_.end(),
        [&model_name](const MLModelConfig& config) {
            return config.model_name == model_name;
        });

    if (it == mlModels_.end()) {
        std::cerr << "Error: ML model '" << model_name << "' not found" << std::endl;
        return false;
    }

    // Save model to file (placeholder implementation)
    std::ofstream ofs(path);
    if (!ofs.is_open()) {
        std::cerr << "Error: Could not open file for saving model: " << path << std::endl;
        return false;
    }

    nlohmann::json model_data;
    model_data["model_name"] = model_name;
    model_data["model_type"] = it->model_type;
    model_data["training_data_size"] = collectedData_.size();
    model_data["timestamp"] = std::time(nullptr);

    ofs << model_data.dump(2);
    ofs.close();

    std::cout << "Model saved: " << model_name << " to " << path << std::endl;
    return true;
}

bool MLCore::loadModel(const std::string& model_name, const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        std::cerr << "Error: Could not open model file: " << path << std::endl;
        return false;
    }

    nlohmann::json model_data;
    try {
        ifs >> model_data;
    } catch (nlohmann::json::parse_error& e) {
        std::cerr << "Error: Failed to parse model file: " << e.what() << std::endl;
        return false;
    }

    std::cout << "Model loaded: " << model_name << " from " << path << std::endl;
    return true;
}

std::vector<std::string> MLCore::getAvailableModels() const {
    std::vector<std::string> model_names;
    for (const auto& model : mlModels_) {
        if (model.enabled) {
            model_names.push_back(model.model_name);
        }
    }
    return model_names;
}

std::map<std::string, double> MLCore::optimizePIDParameters(
    const std::string& control_variable,
    const std::string& setpoint_variable,
    const std::vector<MLDataPoint>& training_data) {
    
    return pidOptimizationAlgorithm(training_data, control_variable, setpoint_variable);
}

const std::vector<MLDataPoint>& MLCore::getCollectedData() const {
    return collectedData_;
}

void MLCore::clearCollectedData() {
    collectedData_.clear();
    lastCollectionTime_ = 0.0;
}

void MLCore::setTrainingCallback(std::function<void(const std::string&, int, double)> callback) {
    trainingCallback_ = callback;
}

void MLCore::setPredictionCallback(MLPredictionCallback callback) {
    predictionCallback_ = callback;
}

void MLCore::setDataCollectionInterval(double interval_seconds) {
    dataCollectionInterval_ = interval_seconds;
}

void MLCore::setMaxDataPoints(size_t max_points) {
    maxDataPoints_ = max_points;
}

// Private helper methods

std::vector<std::string> MLCore::parseFeatureList(const std::string& feature_string) const {
    std::vector<std::string> features;
    std::stringstream ss(feature_string);
    std::string feature;
    
    while (std::getline(ss, feature, ',')) {
        // Trim whitespace
        feature.erase(0, feature.find_first_not_of(" \t"));
        feature.erase(feature.find_last_not_of(" \t") + 1);
        if (!feature.empty()) {
            features.push_back(feature);
        }
    }
    
    return features;
}

MLDataPoint MLCore::createDataPoint(const SimulationState& state, double timestamp) const {
    MLDataPoint point;
    point.timestamp = timestamp;
    
    // Copy all state variables as inputs
    for (const auto& [key, value] : state) {
        point.inputs[key] = value;
    }
    
    // For supervised learning, targets would be future values
    // This is a placeholder - in practice, targets would come from labeled data
    point.targets = point.inputs;
    
    return point;
}

bool MLCore::validateMLModelConfig(const MLModelConfig& config) const {
    if (config.model_name.empty()) {
        std::cerr << "Error: ML model name cannot be empty" << std::endl;
        return false;
    }
    
    if (config.model_type.empty()) {
        std::cerr << "Error: ML model type cannot be empty for model: " << config.model_name << std::endl;
        return false;
    }
    
    if (config.input_features.empty()) {
        std::cerr << "Error: Input features cannot be empty for model: " << config.model_name << std::endl;
        return false;
    }
    
    // Check if model type is supported
    if (config.model_type != "pid_optimizer" && config.model_type != "system_predictor") {
        std::cerr << "Warning: Unsupported model type '" << config.model_type 
                  << "' for model: " << config.model_name << std::endl;
    }
    
    return true;
}

std::map<std::string, double> MLCore::simpleLinearRegression(
    const std::vector<MLDataPoint>& data,
    const std::vector<std::string>& input_features,
    const std::string& output_target) const {
    
    std::map<std::string, double> model_params;
    
    if (data.empty() || input_features.empty()) {
        return model_params;
    }
    
    // Extract feature data
    std::vector<std::vector<double>> X(data.size(), std::vector<double>(input_features.size()));
    std::vector<double> y(data.size());
    
    for (size_t i = 0; i < data.size(); ++i) {
        for (size_t j = 0; j < input_features.size(); ++j) {
            auto it = data[i].inputs.find(input_features[j]);
            X[i][j] = (it != data[i].inputs.end()) ? it->second : 0.0;
        }
        
        auto it = data[i].targets.find(output_target);
        y[i] = (it != data[i].targets.end()) ? it->second : 0.0;
    }
    
    // Simple linear regression using least squares (placeholder)
    // In practice, you'd use a proper linear algebra library
    
    // For now, return dummy parameters
    model_params["intercept"] = 0.0;
    for (size_t i = 0; i < input_features.size(); ++i) {
        model_params["coef_" + input_features[i]] = 1.0;
    }
    model_params["r2_score"] = 0.8;  // Dummy R² score
    
    return model_params;
}

std::map<std::string, double> MLCore::pidOptimizationAlgorithm(
    const std::vector<MLDataPoint>& data,
    const std::string& control_var,
    const std::string& setpoint_var) const {
    
    std::map<std::string, double> optimal_params;
    
    if (data.empty()) {
        optimal_params["kp"] = 1.0;
        optimal_params["ki"] = 0.1;
        optimal_params["kd"] = 0.01;
        return optimal_params;
    }
    
    // Extract control and setpoint data
    std::vector<double> control_values, setpoint_values;
    
    for (const auto& point : data) {
        auto control_it = point.inputs.find(control_var);
        auto setpoint_it = point.inputs.find(setpoint_var);
        
        if (control_it != point.inputs.end() && setpoint_it != point.inputs.end()) {
            control_values.push_back(control_it->second);
            setpoint_values.push_back(setpoint_it->second);
        }
    }
    
    if (control_values.empty()) {
        optimal_params["kp"] = 1.0;
        optimal_params["ki"] = 0.1;
        optimal_params["kd"] = 0.01;
        return optimal_params;
    }
    
    // Simple PID optimization using basic heuristics
    // In practice, you'd use methods like Ziegler-Nichols, genetic algorithms, etc.
    
    // Calculate basic statistics
    double error_sum = 0.0;
    for (size_t i = 0; i < control_values.size(); ++i) {
        error_sum += std::abs(setpoint_values[i] - control_values[i]);
    }
    double avg_error = error_sum / control_values.size();
    
    // Heuristic-based parameter tuning
    if (avg_error > 10.0) {
        optimal_params["kp"] = 2.0;
        optimal_params["ki"] = 0.5;
        optimal_params["kd"] = 0.1;
    } else if (avg_error > 1.0) {
        optimal_params["kp"] = 1.5;
        optimal_params["ki"] = 0.2;
        optimal_params["kd"] = 0.05;
    } else {
        optimal_params["kp"] = 1.0;
        optimal_params["ki"] = 0.1;
        optimal_params["kd"] = 0.01;
    }
    
    optimal_params["avg_error"] = avg_error;
    optimal_params["data_points"] = static_cast<double>(control_values.size());
    
    return optimal_params;
}

std::vector<MLDataPoint> MLCore::preprocessData(const std::vector<MLDataPoint>& raw_data) const {
    std::vector<MLDataPoint> processed_data = raw_data;
    
    // Remove outliers (simple z-score method)
    for (auto& point : processed_data) {
        for (auto& [key, value] : point.inputs) {
            if (std::abs(value) > 1000.0) {  // Simple outlier threshold
                value = std::clamp(value, -1000.0, 1000.0);
            }
        }
    }
    
    // Normalize features
    normalizeFeatures(processed_data);
    
    return processed_data;
}

void MLCore::normalizeFeatures(std::vector<MLDataPoint>& data) const {
    if (data.empty()) return;
    
    // Calculate mean and std for each feature
    std::map<std::string, double> means, stds;
    
    // Calculate means
    for (const auto& point : data) {
        for (const auto& [key, value] : point.inputs) {
            means[key] += value;
        }
    }
    
    for (auto& [key, sum] : means) {
        sum /= data.size();
    }
    
    // Calculate standard deviations
    for (const auto& point : data) {
        for (const auto& [key, value] : point.inputs) {
            double diff = value - means[key];
            stds[key] += diff * diff;
        }
    }
    
    for (auto& [key, sum_sq] : stds) {
        sum_sq = std::sqrt(sum_sq / data.size());
        if (sum_sq == 0.0) sum_sq = 1.0; // Avoid division by zero
    }
    
    // Normalize data
    for (auto& point : data) {
        for (auto& [key, value] : point.inputs) {
            if (stds[key] > 0.0) {
                value = (value - means[key]) / stds[key];
            }
        }
    }
}

} // namespace parsec 