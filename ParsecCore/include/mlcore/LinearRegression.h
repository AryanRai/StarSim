#pragma once

#include "mlcore/MLDataStructures.h"
#include <vector>
#include <string>

namespace parsec {

namespace MLAlgorithms {

class LinearRegression {
public:
    LinearRegression();
    explicit LinearRegression(const MLModelConfig& config);
    
    // Core regression functionality
    struct RegressionResult {
        std::vector<double> coefficients;
        double intercept = 0.0;
        double r_squared = 0.0;
        double adjusted_r_squared = 0.0;
        double mean_squared_error = 0.0;
        double root_mean_squared_error = 0.0;
        double mean_absolute_error = 0.0;
        std::vector<double> residuals;
        std::vector<double> predictions;
        bool successful = false;
    };
    
    // Training methods
    RegressionResult train(const std::vector<MLDataPoint>& training_data,
                          const std::vector<std::string>& input_features,
                          const std::string& target_variable);
    
    RegressionResult trainMultipleTargets(const std::vector<MLDataPoint>& training_data,
                                         const std::vector<std::string>& input_features,
                                         const std::vector<std::string>& target_variables);
    
    // Prediction
    MLPrediction predict(const MLDataPoint& input_data) const;
    std::vector<MLPrediction> predictBatch(const std::vector<MLDataPoint>& input_data) const;
    
    // Different regression algorithms
    RegressionResult ordinaryLeastSquares(const std::vector<MLDataPoint>& data,
                                         const std::vector<std::string>& features,
                                         const std::string& target);
    
    RegressionResult ridgeRegression(const std::vector<MLDataPoint>& data,
                                    const std::vector<std::string>& features,
                                    const std::string& target,
                                    double alpha = 1.0);
    
    RegressionResult lassoRegression(const std::vector<MLDataPoint>& data,
                                    const std::vector<std::string>& features,
                                    const std::string& target,
                                    double alpha = 1.0);
    
    RegressionResult elasticNetRegression(const std::vector<MLDataPoint>& data,
                                         const std::vector<std::string>& features,
                                         const std::string& target,
                                         double alpha = 1.0,
                                         double l1_ratio = 0.5);
    
    // Polynomial regression
    RegressionResult polynomialRegression(const std::vector<MLDataPoint>& data,
                                         const std::vector<std::string>& features,
                                         const std::string& target,
                                         int degree = 2);
    
    // Robust regression
    RegressionResult robustRegression(const std::vector<MLDataPoint>& data,
                                     const std::vector<std::string>& features,
                                     const std::string& target,
                                     const std::string& method = "huber");
    
    // Model evaluation and validation
    struct ValidationResult {
        double cross_validation_score = 0.0;
        std::vector<double> fold_scores;
        double validation_r_squared = 0.0;
        double validation_mse = 0.0;
        MLModelMetrics metrics;
    };
    
    ValidationResult crossValidate(const std::vector<MLDataPoint>& data,
                                  const std::vector<std::string>& features,
                                  const std::string& target,
                                  int k_folds = 5);
    
    ValidationResult validateOnTestSet(const std::vector<MLDataPoint>& test_data,
                                      const std::vector<std::string>& features,
                                      const std::string& target) const;
    
    // Feature importance and analysis
    std::vector<MLFeatureImportance> getFeatureImportance() const;
    std::vector<double> getCoefficients() const;
    double getIntercept() const;
    
    // Model persistence
    bool saveModel(const std::string& filepath) const;
    bool loadModel(const std::string& filepath);
    
    // Diagnostics
    struct DiagnosticResults {
        std::vector<double> residuals;
        std::vector<double> standardized_residuals;
        std::vector<double> leverage_values;
        std::vector<double> cooks_distance;
        std::vector<int> outlier_indices;
        double durbin_watson_statistic = 0.0;
        bool normality_test_passed = true;
        bool homoscedasticity_test_passed = true;
    };
    
    DiagnosticResults runDiagnostics(const std::vector<MLDataPoint>& data,
                                    const std::vector<std::string>& features,
                                    const std::string& target) const;
    
    // Configuration
    void setRegularizationParameters(double alpha, double l1_ratio = 0.5);
    void setConvergenceThreshold(double threshold);
    void setMaxIterations(int max_iterations);
    
private:
    MLModelConfig config_;
    RegressionResult currentModel_;
    std::vector<std::string> featureNames_;
    std::string targetName_;
    
    // Regularization parameters
    double alpha_ = 1.0;
    double l1_ratio_ = 0.5;
    double convergence_threshold_ = 1e-6;
    int max_iterations_ = 1000;
    
    // Internal implementation methods
    std::vector<std::vector<double>> extractFeatureMatrix(const std::vector<MLDataPoint>& data,
                                                         const std::vector<std::string>& features) const;
    
    std::vector<double> extractTargetVector(const std::vector<MLDataPoint>& data,
                                           const std::string& target) const;
    
    RegressionResult solveNormalEquations(const std::vector<std::vector<double>>& X,
                                         const std::vector<double>& y) const;
    
    RegressionResult solveRegularized(const std::vector<std::vector<double>>& X,
                                     const std::vector<double>& y,
                                     double alpha,
                                     const std::string& regularization_type) const;
    
    std::vector<double> predictFromMatrix(const std::vector<std::vector<double>>& X) const;
    
    double calculateRSquared(const std::vector<double>& y_true,
                            const std::vector<double>& y_pred) const;
    
    double calculateMSE(const std::vector<double>& y_true,
                       const std::vector<double>& y_pred) const;
    
    double calculateMAE(const std::vector<double>& y_true,
                       const std::vector<double>& y_pred) const;
    
    // Matrix operations helpers
    std::vector<std::vector<double>> transposeMatrix(const std::vector<std::vector<double>>& matrix) const;
    std::vector<std::vector<double>> multiplyMatrices(const std::vector<std::vector<double>>& A,
                                                     const std::vector<std::vector<double>>& B) const;
    std::vector<std::vector<double>> invertMatrix(const std::vector<std::vector<double>>& matrix) const;
    
    // Polynomial feature expansion
    std::vector<std::vector<double>> expandPolynomialFeatures(const std::vector<std::vector<double>>& X,
                                                             int degree) const;
    
    // Validation helpers
    std::vector<std::vector<MLDataPoint>> createKFolds(const std::vector<MLDataPoint>& data,
                                                       int k_folds) const;
};

} // namespace MLAlgorithms

} // namespace parsec 