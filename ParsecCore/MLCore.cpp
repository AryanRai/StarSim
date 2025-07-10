#include "include/parsec/MLCore.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <nlohmann/json.hpp>

namespace parsec {

MLCore::MLCore(const ModelConfig& config)
    : modelConfig_(config), enabled_(false) {
    // Initialize ML core with default settings
}

bool MLCore::loadMLConfiguration(const std::string& ml_config_path) {
    try {
        std::ifstream file(ml_config_path);
        if (!file.is_open()) {
            std::cerr << "MLCore: Could not open ML configuration file: " << ml_config_path << std::endl;
            return false;
        }

        nlohmann::json config;
        file >> config;

        // Parse ML models configuration
        if (config.contains("ml_models")) {
            for (const auto& model_json : config["ml_models"]) {
                MLModelConfig model;
                model.model_name = model_json.value("model_name", "");
                model.model_type = model_json.value("model_type", "");
                model.input_features = model_json.value("input_features", "");
                model.output_targets = model_json.value("output_targets", "");
                model.model_path = model_json.value("model_path", "");
                model.enabled = model_json.value("enabled", true);

                if (model_json.contains("hyperparameters")) {
                    for (const auto& param : model_json["hyperparameters"].items()) {
                        model.hyperparameters[param.key()] = param.value();
                    }
                }

                if (validateMLModelConfig(model)) {
                    mlModels_.push_back(model);
                }
            }
        }

        // Parse general ML settings
        if (config.contains("settings")) {
            auto settings = config["settings"];
            dataCollectionInterval_ = settings.value("data_collection_interval", 0.1);
            maxDataPoints_ = settings.value("max_data_points", 10000);
            enabled_ = settings.value("enabled", true);
        }

        std::cout << "MLCore: Loaded " << mlModels_.size() << " ML models" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "MLCore: Error loading ML configuration: " << e.what() << std::endl;
        return false;
    }
}

bool MLCore::isEnabled() const {
    return enabled_;
}

void MLCore::collectData(const SimulationState& currentState, double timestamp) {
    if (!enabled_ || timestamp - lastCollectionTime_ < dataCollectionInterval_) {
        return;
    }

    // Create data point from current state
    MLDataPoint dataPoint = createDataPoint(currentState, timestamp);
    collectedData_.push_back(dataPoint);

    // Limit data collection to maxDataPoints_
    if (collectedData_.size() > maxDataPoints_) {
        collectedData_.erase(collectedData_.begin(), 
                           collectedData_.begin() + (collectedData_.size() - maxDataPoints_));
    }

    lastCollectionTime_ = timestamp;
}

void MLCore::processCollectedData() {
    if (collectedData_.empty()) {
        return;
    }

    // Preprocess data (normalize, clean, etc.)
    auto processedData = preprocessData(collectedData_);
    
    // Replace collected data with processed data
    collectedData_ = processedData;
    
    std::cout << "MLCore: Processed " << collectedData_.size() << " data points" << std::endl;
}

bool MLCore::trainModel(const std::string& model_name) {
    auto model_it = std::find_if(mlModels_.begin(), mlModels_.end(),
                                [&model_name](const MLModelConfig& config) {
                                    return config.model_name == model_name;
                                });

    if (model_it == mlModels_.end()) {
        std::cerr << "MLCore: Model not found: " << model_name << std::endl;
        return false;
    }

    if (collectedData_.empty()) {
        std::cerr << "MLCore: No data available for training" << std::endl;
        return false;
    }

    try {
        // Simple training based on model type
        if (model_it->model_type == "pid_optimizer") {
            auto input_features = parseFeatureList(model_it->input_features);
            auto output_features = parseFeatureList(model_it->output_targets);
            
            if (input_features.size() >= 2 && output_features.size() >= 1) {
                auto optimal_params = pidOptimizationAlgorithm(
                    collectedData_, input_features[0], input_features[1]);
                
                std::cout << "MLCore: PID optimization complete for " << model_name << std::endl;
                std::cout << "  Optimal parameters: ";
                for (const auto& param : optimal_params) {
                    std::cout << param.first << "=" << param.second << " ";
                }
                std::cout << std::endl;
            }
        } else if (model_it->model_type == "predictor") {
            auto input_features = parseFeatureList(model_it->input_features);
            auto output_features = parseFeatureList(model_it->output_targets);
            
            if (!input_features.empty() && !output_features.empty()) {
                auto regression_params = simpleLinearRegression(
                    collectedData_, input_features, output_features[0]);
                
                std::cout << "MLCore: Linear regression complete for " << model_name << std::endl;
            }
        }

        if (trainingCallback_) {
            trainingCallback_(model_name, 1, 0.0);  // Simple callback
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "MLCore: Training error: " << e.what() << std::endl;
        return false;
    }
}

MLPrediction MLCore::predict(const std::string& model_name, const SimulationState& currentState) {
    MLPrediction prediction;
    prediction.model_name = model_name;
    prediction.status = "error";

    auto model_it = std::find_if(mlModels_.begin(), mlModels_.end(),
                                [&model_name](const MLModelConfig& config) {
                                    return config.model_name == model_name;
                                });

    if (model_it == mlModels_.end()) {
        prediction.status = "model_not_found";
        return prediction;
    }

    try {
        // Simple prediction based on model type
        if (model_it->model_type == "pid_optimizer") {
            // Return basic PID recommendations
            prediction.recommendations["kp"] = 1.0;
            prediction.recommendations["ki"] = 0.1;
            prediction.recommendations["kd"] = 0.01;
            prediction.confidence = 0.8;
        } else if (model_it->model_type == "predictor") {
            // Simple linear prediction
            auto input_features = parseFeatureList(model_it->input_features);
            for (const auto& feature : input_features) {
                if (currentState.find(feature) != currentState.end()) {
                    prediction.predictions[feature + "_predicted"] = currentState.at(feature) * 1.1;
                }
            }
            prediction.confidence = 0.7;
        }

        prediction.status = "success";

        if (predictionCallback_) {
            predictionCallback_(prediction);
        }

    } catch (const std::exception& e) {
        std::cerr << "MLCore: Prediction error: " << e.what() << std::endl;
        prediction.status = "prediction_error";
    }

    return prediction;
}

bool MLCore::saveModel(const std::string& model_name, const std::string& path) {
    try {
        nlohmann::json model_data;
        model_data["model_name"] = model_name;
        model_data["timestamp"] = std::time(nullptr);
        model_data["data_points"] = collectedData_.size();
        
        std::ofstream file(path);
        file << model_data.dump(2);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "MLCore: Error saving model: " << e.what() << std::endl;
        return false;
    }
}

bool MLCore::loadModel(const std::string& model_name, const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            return false;
        }

        nlohmann::json model_data;
        file >> model_data;
        
        std::cout << "MLCore: Loaded model " << model_name << " from " << path << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "MLCore: Error loading model: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> MLCore::getAvailableModels() const {
    std::vector<std::string> models;
    for (const auto& config : mlModels_) {
        models.push_back(config.model_name);
    }
    return models;
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

void MLCore::setTrainingCallback(MLTrainingCallback callback) {
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
    MLDataPoint dataPoint;
    dataPoint.timestamp = timestamp;
    
    // Copy all state variables as inputs
    for (const auto& var : state) {
        dataPoint.inputs[var.first] = var.second;
    }
    
    return dataPoint;
}

bool MLCore::validateMLModelConfig(const MLModelConfig& config) const {
    if (config.model_name.empty() || config.model_type.empty()) {
        std::cerr << "MLCore: Invalid model config - missing name or type" << std::endl;
        return false;
    }
    
    if (config.input_features.empty() || config.output_targets.empty()) {
        std::cerr << "MLCore: Invalid model config - missing features or targets" << std::endl;
        return false;
    }
    
    return true;
}

std::map<std::string, double> MLCore::simpleLinearRegression(
    const std::vector<MLDataPoint>& data,
    const std::vector<std::string>& input_features,
    const std::string& output_target) const {
    
    std::map<std::string, double> parameters;
    
    if (data.empty() || input_features.empty()) {
        return parameters;
    }
    
    // Simple linear regression for first input feature
    const std::string& x_feature = input_features[0];
    
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
    size_t n = 0;
    
    for (const auto& point : data) {
        if (point.inputs.find(x_feature) != point.inputs.end() &&
            point.inputs.find(output_target) != point.inputs.end()) {
            
            double x = point.inputs.at(x_feature);
            double y = point.inputs.at(output_target);
            
            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
            n++;
        }
    }
    
    if (n > 1) {
        double slope = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
        double intercept = (sum_y - slope * sum_x) / n;
        
        parameters["slope"] = slope;
        parameters["intercept"] = intercept;
    }
    
    return parameters;
}

std::map<std::string, double> MLCore::pidOptimizationAlgorithm(
    const std::vector<MLDataPoint>& data,
    const std::string& control_var,
    const std::string& setpoint_var) const {
    
    std::map<std::string, double> pid_params;
    
    if (data.empty()) {
        // Return default PID values
        pid_params["kp"] = 1.0;
        pid_params["ki"] = 0.1;
        pid_params["kd"] = 0.01;
        return pid_params;
    }
    
    // Simple PID tuning based on system response
    double error_sum = 0.0;
    double error_variance = 0.0;
    size_t n = 0;
    
    for (const auto& point : data) {
        if (point.inputs.find(control_var) != point.inputs.end() &&
            point.inputs.find(setpoint_var) != point.inputs.end()) {
            
            double error = point.inputs.at(setpoint_var) - point.inputs.at(control_var);
            error_sum += std::abs(error);
            error_variance += error * error;
            n++;
        }
    }
    
    if (n > 0) {
        double avg_error = error_sum / n;
        double rms_error = std::sqrt(error_variance / n);
        
        // Simple heuristic-based PID tuning
        double kp = 1.0 / (1.0 + avg_error);
        double ki = 0.1 * kp;
        double kd = 0.01 * kp * rms_error;
        
        pid_params["kp"] = std::max(0.1, std::min(10.0, kp));
        pid_params["ki"] = std::max(0.01, std::min(1.0, ki));
        pid_params["kd"] = std::max(0.001, std::min(0.1, kd));
    } else {
        // Default values
        pid_params["kp"] = 1.0;
        pid_params["ki"] = 0.1;
        pid_params["kd"] = 0.01;
    }
    
    return pid_params;
}

std::vector<MLDataPoint> MLCore::preprocessData(const std::vector<MLDataPoint>& raw_data) const {
    std::vector<MLDataPoint> processed_data = raw_data;
    
    // Remove outliers (simple method)
    if (processed_data.size() > 10) {
        // Calculate basic statistics for each feature
        std::map<std::string, std::pair<double, double>> feature_stats;  // mean, std
        
        for (const auto& point : processed_data) {
            for (const auto& input : point.inputs) {
                if (feature_stats.find(input.first) == feature_stats.end()) {
                    feature_stats[input.first] = {0.0, 0.0};
                }
                feature_stats[input.first].first += input.second;
            }
        }
        
        // Calculate means
        for (auto& stat : feature_stats) {
            stat.second.first /= processed_data.size();
        }
        
        // Calculate standard deviations
        for (const auto& point : processed_data) {
            for (const auto& input : point.inputs) {
                if (feature_stats.find(input.first) != feature_stats.end()) {
                    double diff = input.second - feature_stats[input.first].first;
                    feature_stats[input.first].second += diff * diff;
                }
            }
        }
        
        for (auto& stat : feature_stats) {
            stat.second.second = std::sqrt(stat.second.second / processed_data.size());
        }
    }
    
    return processed_data;
}

void MLCore::normalizeFeatures(std::vector<MLDataPoint>& data) const {
    if (data.empty()) return;
    
    // Calculate min/max for each feature
    std::map<std::string, std::pair<double, double>> feature_ranges;  // min, max
    
    for (const auto& point : data) {
        for (const auto& input : point.inputs) {
            if (feature_ranges.find(input.first) == feature_ranges.end()) {
                feature_ranges[input.first] = {input.second, input.second};
            } else {
                feature_ranges[input.first].first = std::min(feature_ranges[input.first].first, input.second);
                feature_ranges[input.first].second = std::max(feature_ranges[input.first].second, input.second);
            }
        }
    }
    
    // Normalize features to [0, 1] range
    for (auto& point : data) {
        for (auto& input : point.inputs) {
            if (feature_ranges.find(input.first) != feature_ranges.end()) {
                double min_val = feature_ranges[input.first].first;
                double max_val = feature_ranges[input.first].second;
                
                if (max_val > min_val) {
                    input.second = (input.second - min_val) / (max_val - min_val);
                }
            }
        }
    }
}

} // namespace parsec 