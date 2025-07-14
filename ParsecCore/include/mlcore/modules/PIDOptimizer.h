#pragma once

#include "mlcore/MLDataStructures.h"
#include <vector>
#include <string>
#include <map>

namespace parsec {

namespace MLAlgorithms {

class PIDOptimizer {
public:
    PIDOptimizer();
    explicit PIDOptimizer(const MLModelConfig& config);
    
    // Core PID optimization functionality
    struct PIDParameters {
        double kp = 0.0;
        double ki = 0.0;
        double kd = 0.0;
        double fitness_score = 0.0;
        std::string optimization_method;
    };
    
    // Optimization methods
    PIDParameters optimizeZieglerNichols(const std::vector<MLDataPoint>& data,
                                        const std::string& control_variable,
                                        const std::string& setpoint_variable);
    
    PIDParameters optimizeCohenCoon(const std::vector<MLDataPoint>& data,
                                   const std::string& control_variable,
                                   const std::string& setpoint_variable);
    
    PIDParameters optimizeGeneticAlgorithm(const std::vector<MLDataPoint>& data,
                                          const std::string& control_variable,
                                          const std::string& setpoint_variable,
                                          int population_size = 50,
                                          int generations = 100);
    
    PIDParameters optimizeParticleSwarm(const std::vector<MLDataPoint>& data,
                                       const std::string& control_variable,
                                       const std::string& setpoint_variable,
                                       int swarm_size = 30,
                                       int iterations = 100);
    
    PIDParameters optimizeGradientDescent(const std::vector<MLDataPoint>& data,
                                         const std::string& control_variable,
                                         const std::string& setpoint_variable,
                                         double learning_rate = 0.01,
                                         int max_iterations = 1000);
    
    // Fitness evaluation
    struct FitnessMetrics {
        double integral_squared_error = 0.0;
        double integral_absolute_error = 0.0;
        double integral_time_absolute_error = 0.0;
        double overshoot = 0.0;
        double settling_time = 0.0;
        double rise_time = 0.0;
        double steady_state_error = 0.0;
        double control_effort = 0.0;
    };
    
    FitnessMetrics evaluatePIDPerformance(const PIDParameters& pid,
                                         const std::vector<MLDataPoint>& data,
                                         const std::string& control_variable,
                                         const std::string& setpoint_variable);
    
    double calculateFitnessScore(const FitnessMetrics& metrics, const std::string& objective = "ise");
    
    // System identification for PID tuning
    struct SystemModel {
        double process_gain = 1.0;
        double time_constant = 1.0;
        double dead_time = 0.0;
        double time_delay = 0.0;
        std::string model_type = "FOPDT";  // First Order Plus Dead Time
    };
    
    SystemModel identifySystem(const std::vector<MLDataPoint>& data,
                              const std::string& input_variable,
                              const std::string& output_variable);
    
    PIDParameters tuneFromSystemModel(const SystemModel& model, const std::string& tuning_method = "ziegler_nichols");
    
    // Advanced PID features
    struct AdvancedPIDConfig {
        bool enable_derivative_filter = true;
        double derivative_filter_time = 0.1;
        bool enable_integral_windup_protection = true;
        double integral_max = 100.0;
        double integral_min = -100.0;
        bool enable_output_limits = true;
        double output_max = 100.0;
        double output_min = 0.0;
        bool enable_setpoint_weighting = false;
        double setpoint_weight = 1.0;
    };
    
    PIDParameters optimizeAdvancedPID(const std::vector<MLDataPoint>& data,
                                     const std::string& control_variable,
                                     const std::string& setpoint_variable,
                                     const AdvancedPIDConfig& config);
    
    // Multi-objective optimization
    struct MultiObjectiveResult {
        std::vector<PIDParameters> pareto_front;
        std::vector<std::vector<double>> objective_values;
        PIDParameters recommended_solution;
    };
    
    MultiObjectiveResult optimizeMultiObjective(const std::vector<MLDataPoint>& data,
                                               const std::string& control_variable,
                                               const std::string& setpoint_variable,
                                               const std::vector<std::string>& objectives);
    
    // Adaptive PID tuning
    struct AdaptivePIDState {
        PIDParameters current_parameters;
        double adaptation_rate = 0.01;
        double forgetting_factor = 0.95;
        std::vector<double> parameter_history;
        bool adaptation_enabled = true;
    };
    
    void updateAdaptivePID(AdaptivePIDState& state,
                          const MLDataPoint& current_data,
                          const std::string& control_variable,
                          const std::string& setpoint_variable);
    
    // Validation and testing
    struct ValidationResult {
        double cross_validation_score = 0.0;
        std::vector<double> fold_scores;
        double robustness_score = 0.0;
        std::map<std::string, double> performance_metrics;
    };
    
    ValidationResult validatePIDParameters(const PIDParameters& pid,
                                          const std::vector<MLDataPoint>& validation_data,
                                          const std::string& control_variable,
                                          const std::string& setpoint_variable);
    
    // Configuration and utilities
    void setOptimizationBounds(double kp_min, double kp_max,
                              double ki_min, double ki_max,
                              double kd_min, double kd_max);
    
    void setFitnessWeights(double ise_weight = 1.0, double overshoot_weight = 1.0,
                          double settling_time_weight = 1.0, double control_effort_weight = 0.1);
    
    // Model persistence
    bool savePIDModel(const PIDParameters& pid, const std::string& filepath);
    bool loadPIDModel(PIDParameters& pid, const std::string& filepath);
    
    // Callbacks for monitoring optimization progress
    void setProgressCallback(std::function<void(int iteration, const PIDParameters& best_params, double fitness)> callback);
    
private:
    MLModelConfig config_;
    
    // Optimization bounds
    double kp_min_ = 0.0, kp_max_ = 100.0;
    double ki_min_ = 0.0, ki_max_ = 10.0;
    double kd_min_ = 0.0, kd_max_ = 10.0;
    
    // Fitness weights
    double ise_weight_ = 1.0;
    double overshoot_weight_ = 1.0;
    double settling_time_weight_ = 1.0;
    double control_effort_weight_ = 0.1;
    
    // Progress callback
    std::function<void(int, const PIDParameters&, double)> progress_callback_;
    
    // Internal helper methods
    std::vector<double> extractTimeSeries(const std::vector<MLDataPoint>& data, const std::string& variable);
    std::vector<double> simulatePIDResponse(const PIDParameters& pid,
                                           const std::vector<double>& setpoint,
                                           const std::vector<double>& process_variable,
                                           double dt = 0.01);
    
    double calculateStepResponseMetrics(const std::vector<double>& response,
                                       const std::vector<double>& time,
                                       const std::string& metric);
    
    // Optimization algorithms implementation
    PIDParameters geneticAlgorithmImplementation(const std::vector<MLDataPoint>& data,
                                                const std::string& control_var,
                                                const std::string& setpoint_var,
                                                int population_size,
                                                int generations);
    
    PIDParameters particleSwarmImplementation(const std::vector<MLDataPoint>& data,
                                             const std::string& control_var,
                                             const std::string& setpoint_var,
                                             int swarm_size,
                                             int iterations);
};

} // namespace MLAlgorithms

} // namespace parsec 