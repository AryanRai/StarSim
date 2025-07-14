#pragma once

#include <vector>
#include <array>
#include <string>
#include <functional>

namespace parsec {

namespace Controls {
    // PID controller
    double calculatePIDOutput(double setpoint, double process_variable, double kp, double ki, double kd, 
                             double& integral_sum, double& previous_error, double dt);
    
    // Transfer functions
    double calculateFirstOrderResponse(double input, double time_constant, double gain, double time);
    double calculateSecondOrderResponse(double input, double natural_frequency, double damping_ratio, double gain, double time);
    
    // Stability analysis
    bool isStable(const std::vector<double>& characteristic_polynomial_coefficients);
    double calculateSettlingTime(double natural_frequency, double damping_ratio);
    double calculateOvershoot(double damping_ratio);
    
    // Control system design
    std::array<double, 3> tunePIDZieglerNichols(double ultimate_gain, double ultimate_period, const std::string& controller_type);
    std::array<double, 3> tunePIDCohenCoon(double process_gain, double time_constant, double dead_time);
    
    // Advanced PID features
    double calculateIntegralWindup(double integral_sum, double max_integral, double min_integral);
    double calculateDerivativeKick(double setpoint_change, double kd);
    
    // State space control
    struct StateSpaceModel {
        std::vector<std::vector<double>> A_matrix;  // State matrix
        std::vector<std::vector<double>> B_matrix;  // Input matrix
        std::vector<std::vector<double>> C_matrix;  // Output matrix
        std::vector<std::vector<double>> D_matrix;  // Feedthrough matrix
    };
    
    std::vector<double> calculateStateSpaceResponse(const StateSpaceModel& model, 
                                                   const std::vector<double>& current_state,
                                                   const std::vector<double>& input);
    
    // Modern control methods
    std::vector<double> calculateLQRGains(const StateSpaceModel& model, 
                                         const std::vector<std::vector<double>>& Q_matrix,
                                         const std::vector<std::vector<double>>& R_matrix);
    
    // Kalman filtering
    struct KalmanFilter {
        std::vector<double> state_estimate;
        std::vector<std::vector<double>> covariance_matrix;
        std::vector<std::vector<double>> process_noise;
        std::vector<std::vector<double>> measurement_noise;
    };
    
    void updateKalmanFilter(KalmanFilter& filter, const StateSpaceModel& model,
                           const std::vector<double>& measurement);
    
    // Adaptive control
    double calculateMRAC(double reference_signal, double plant_output, 
                        double adaptation_gain, double& adaptive_parameter);
    
    // Fuzzy logic control
    struct FuzzySet {
        std::string name;
        std::vector<double> membership_function;
        double min_value;
        double max_value;
    };
    
    double calculateFuzzyOutput(const std::vector<FuzzySet>& input_sets,
                               const std::vector<FuzzySet>& output_sets,
                               const std::vector<double>& inputs);
    
    // Model predictive control
    std::vector<double> calculateMPCAction(const StateSpaceModel& model,
                                          const std::vector<double>& current_state,
                                          const std::vector<double>& reference_trajectory,
                                          int prediction_horizon,
                                          int control_horizon);
}

} // namespace parsec 