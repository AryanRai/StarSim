#pragma once

#include "tradingcore/TradingDataStructures.h"
#include "tradingcore/modules/MarketData.h"
#include "mathcore/math.h"
#include "parsec/SolverCore.h"
#include <memory>
#include <map>
#include <vector>
#include <functional>

namespace parsec {
namespace tradingcore {

// Portfolio optimization algorithms
enum class OptimizationMethod {
    MEAN_VARIANCE,      // Modern Portfolio Theory
    RISK_PARITY,        // Equal risk contribution
    EQUAL_WEIGHT,       // 1/N portfolio
    MIN_VARIANCE,       // Minimum variance
    MAX_SHARPE,         // Maximum Sharpe ratio
    BLACK_LITTERMAN,    // Black-Litterman model
    HIERARCHICAL_RISK_PARITY // HRP
};

// Portfolio constraints
struct PortfolioConstraints {
    double min_weight = 0.0;           // Minimum allocation per asset
    double max_weight = 1.0;           // Maximum allocation per asset
    double max_concentration = 0.4;    // Maximum single asset concentration
    double min_cash_reserve = 0.05;    // Minimum cash percentage
    double max_turnover = 0.5;         // Maximum portfolio turnover per rebalance
    bool allow_short_selling = false;  // Allow negative weights
    
    // Sector/category constraints
    std::map<std::string, double> sector_max_weights;
    std::map<std::string, std::vector<std::string>> sector_mappings;
    
    // Asset-specific constraints
    std::map<std::string, double> asset_min_weights;
    std::map<std::string, double> asset_max_weights;
    
    PortfolioConstraints() = default;
    bool validate() const;
};

// Portfolio optimization parameters
struct OptimizationParams {
    OptimizationMethod method = OptimizationMethod::MEAN_VARIANCE;
    int lookback_periods = 252;        // Historical data periods for estimation
    double risk_aversion = 1.0;        // Risk aversion parameter
    double expected_return_method = 0;  // 0=historical, 1=CAPM, 2=custom
    bool use_shrinkage = true;         // Use covariance shrinkage
    double shrinkage_intensity = 0.5;  // Ledoit-Wolf shrinkage parameter
    
    // Black-Litterman specific
    double tau = 0.05;                 // Uncertainty parameter
    std::map<std::string, double> views; // Expected returns views
    std::map<std::string, double> view_confidence; // Confidence in views
    
    // Risk parity specific
    double risk_parity_tolerance = 1e-6;
    int max_iterations = 1000;
    
    OptimizationParams() = default;
};

// Rebalancing configuration
struct RebalancingConfig {
    enum class TriggerType {
        TIME_BASED,     // Fixed time intervals
        DRIFT_BASED,    // When weights drift beyond threshold
        VOLATILITY_BASED, // Based on volatility regime
        SIGNAL_BASED    // Based on trading signals
    };
    
    TriggerType trigger_type = TriggerType::TIME_BASED;
    int rebalance_frequency_days = 30; // For time-based
    double drift_threshold = 0.05;     // For drift-based (5% deviation)
    double volatility_threshold = 0.2; // For volatility-based
    
    // Transaction cost considerations
    bool consider_transaction_costs = true;
    double fixed_cost_per_trade = 0.0;
    double variable_cost_bps = 5.0;    // 5 basis points
    
    // Timing constraints
    bool allow_partial_rebalancing = true;
    int max_trades_per_rebalance = 10;
    std::vector<int> allowed_rebalance_hours = {9, 10, 14, 15}; // Market hours
    
    RebalancingConfig() = default;
};

// Base class for portfolio optimizers
class PortfolioOptimizer {
public:
    explicit PortfolioOptimizer(const OptimizationParams& params);
    virtual ~PortfolioOptimizer() = default;
    
    virtual std::map<std::string, double> optimize(
        const std::vector<std::string>& assets,
        const std::map<std::string, HistoricalData>& price_data,
        const PortfolioConstraints& constraints) = 0;
    
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    
    // Parameter management
    OptimizationParams getParams() const { return params_; }
    void setParams(const OptimizationParams& params) { params_ = params; }
    
    // Diagnostics
    virtual std::map<std::string, double> getDiagnostics() const { return diagnostics_; }
    
protected:
    OptimizationParams params_;
    std::map<std::string, double> diagnostics_;
    
    // Common utility methods
    std::vector<std::vector<double>> calculateCovarianceMatrix(
        const std::map<std::string, std::vector<double>>& returns);
    
    std::vector<double> calculateExpectedReturns(
        const std::map<std::string, std::vector<double>>& returns);
    
    std::vector<std::vector<double>> applyShrinkage(
        const std::vector<std::vector<double>>& sample_cov,
        double intensity);
    
    std::map<std::string, std::vector<double>> calculateReturns(
        const std::map<std::string, HistoricalData>& price_data);
};

// Mean-Variance Optimizer (Modern Portfolio Theory)
class MeanVarianceOptimizer : public PortfolioOptimizer {
public:
    explicit MeanVarianceOptimizer(const OptimizationParams& params);
    
    std::map<std::string, double> optimize(
        const std::vector<std::string>& assets,
        const std::map<std::string, HistoricalData>& price_data,
        const PortfolioConstraints& constraints) override;
    
    std::string getName() const override { return "Mean-Variance"; }
    std::string getDescription() const override { return "Modern Portfolio Theory optimization"; }
    
    // Efficient frontier generation
    std::vector<std::pair<double, double>> generateEfficientFrontier(
        const std::vector<std::string>& assets,
        const std::map<std::string, HistoricalData>& price_data,
        int num_points = 50);
    
private:
    std::shared_ptr<parsec::SolverCore> solver_;
    
    // Optimization objective functions
    double calculatePortfolioVariance(const std::vector<double>& weights,
                                    const std::vector<std::vector<double>>& cov_matrix);
    
    double calculatePortfolioReturn(const std::vector<double>& weights,
                                  const std::vector<double>& expected_returns);
    
    double calculateUtility(const std::vector<double>& weights,
                          const std::vector<double>& expected_returns,
                          const std::vector<std::vector<double>>& cov_matrix);
};

// Risk Parity Optimizer
class RiskParityOptimizer : public PortfolioOptimizer {
public:
    explicit RiskParityOptimizer(const OptimizationParams& params);
    
    std::map<std::string, double> optimize(
        const std::vector<std::string>& assets,
        const std::map<std::string, HistoricalData>& price_data,
        const PortfolioConstraints& constraints) override;
    
    std::string getName() const override { return "Risk-Parity"; }
    std::string getDescription() const override { return "Equal risk contribution optimization"; }
    
private:
    std::vector<double> calculateRiskContributions(
        const std::vector<double>& weights,
        const std::vector<std::vector<double>>& cov_matrix);
    
    double calculateRiskParityObjective(
        const std::vector<double>& weights,
        const std::vector<std::vector<double>>& cov_matrix);
};

// Black-Litterman Optimizer
class BlackLittermanOptimizer : public PortfolioOptimizer {
public:
    explicit BlackLittermanOptimizer(const OptimizationParams& params);
    
    std::map<std::string, double> optimize(
        const std::vector<std::string>& assets,
        const std::map<std::string, HistoricalData>& price_data,
        const PortfolioConstraints& constraints) override;
    
    std::string getName() const override { return "Black-Litterman"; }
    std::string getDescription() const override { return "Black-Litterman model with investor views"; }
    
    // View management
    void addView(const std::string& asset, double expected_return, double confidence);
    void clearViews();
    std::map<std::string, std::pair<double, double>> getViews() const;
    
private:
    std::vector<double> calculateImpliedReturns(
        const std::vector<std::vector<double>>& cov_matrix,
        const std::vector<double>& market_cap_weights);
    
    std::vector<double> calculateBlackLittermanReturns(
        const std::vector<double>& implied_returns,
        const std::vector<std::vector<double>>& cov_matrix);
};

// Main Portfolio Manager class
class PortfolioManager {
public:
    explicit PortfolioManager(const RebalancingConfig& config);
    ~PortfolioManager();
    
    // Configuration
    void setRebalancingConfig(const RebalancingConfig& config);
    void setOptimizationParams(const OptimizationParams& params);
    void setConstraints(const PortfolioConstraints& constraints);
    
    RebalancingConfig getRebalancingConfig() const { return rebalancing_config_; }
    OptimizationParams getOptimizationParams() const { return optimization_params_; }
    PortfolioConstraints getConstraints() const { return constraints_; }
    
    // Portfolio management
    void setCurrentPortfolio(const Portfolio& portfolio);
    Portfolio getCurrentPortfolio() const { return current_portfolio_; }
    
    void addAsset(const std::string& symbol, double target_weight = 0.0);
    void removeAsset(const std::string& symbol);
    std::vector<std::string> getAssets() const;
    
    // Portfolio optimization
    std::map<std::string, double> optimizePortfolio(
        const std::map<std::string, HistoricalData>& price_data,
        OptimizationMethod method = OptimizationMethod::MEAN_VARIANCE);
    
    std::map<std::string, double> getTargetWeights() const { return target_weights_; }
    void setTargetWeights(const std::map<std::string, double>& weights);
    
    // Rebalancing
    bool shouldRebalance(const std::map<std::string, MarketDataPoint>& current_prices);
    std::vector<Order> generateRebalanceOrders(const std::map<std::string, MarketDataPoint>& current_prices);
    
    void executeRebalance(const std::map<std::string, MarketDataPoint>& current_prices);
    
    // Performance tracking
    void updatePortfolioValue(const std::map<std::string, MarketDataPoint>& current_prices);
    PerformanceMetrics calculatePerformance(const Timestamp& start_date = {});
    
    std::vector<double> getValueHistory() const { return value_history_; }
    std::vector<Timestamp> getValueTimestamps() const { return value_timestamps_; }
    
    // Risk management
    double calculatePortfolioVaR(double confidence_level = 0.05, int horizon_days = 1);
    double calculateMaxDrawdown();
    double calculateSharpeRatio(double risk_free_rate = Constants::RISK_FREE_RATE);
    double calculateVolatility(int lookback_days = 30);
    
    // Risk monitoring
    bool isRiskLimitBreached(const RiskParams& risk_params);
    std::map<std::string, double> getRiskMetrics();
    
    // Diagnostics and reporting
    std::map<std::string, double> getPortfolioAnalytics();
    std::map<std::string, double> getAssetContributions(); // Risk/return contributions
    std::map<std::string, double> getCurrentWeights();
    std::map<std::string, double> getWeightDeviations();
    
    // Transaction cost analysis
    double estimateTransactionCosts(const std::vector<Order>& orders);
    double calculateTurnover(const std::map<std::string, double>& new_weights);
    
    // Callbacks
    void setRebalanceCallback(std::function<void(const std::vector<Order>&)> callback);
    void setRiskAlertCallback(std::function<void(const std::string&, double)> callback);
    
private:
    RebalancingConfig rebalancing_config_;
    OptimizationParams optimization_params_;
    PortfolioConstraints constraints_;
    
    Portfolio current_portfolio_;
    std::map<std::string, double> target_weights_;
    std::vector<std::string> universe_;
    
    // Optimizers
    std::map<OptimizationMethod, std::unique_ptr<PortfolioOptimizer>> optimizers_;
    
    // Performance tracking
    std::vector<double> value_history_;
    std::vector<Timestamp> value_timestamps_;
    Timestamp last_rebalance_time_;
    
    // Callbacks
    std::function<void(const std::vector<Order>&)> rebalance_callback_;
    std::function<void(const std::string&, double)> risk_alert_callback_;
    
    // Helper methods
    void initializeOptimizers();
    bool validateWeights(const std::map<std::string, double>& weights);
    std::map<std::string, double> normalizeWeights(const std::map<std::string, double>& weights);
    double calculateWeightDeviation(const std::map<std::string, double>& current_weights);
    void logRebalanceEvent(const std::vector<Order>& orders);
    void checkRiskLimits();
};

// Portfolio optimizer factory
class OptimizerFactory {
public:
    static std::unique_ptr<PortfolioOptimizer> createOptimizer(
        OptimizationMethod method,
        const OptimizationParams& params);
    
    static std::vector<OptimizationMethod> getAvailableMethods();
    static std::string getMethodName(OptimizationMethod method);
    static OptimizationParams getDefaultParams(OptimizationMethod method);
};

// Portfolio analysis utilities
namespace PortfolioUtils {
    // Portfolio statistics
    double calculateSharpeRatio(const std::vector<double>& returns, double risk_free_rate = 0.02);
    double calculateSortinoRatio(const std::vector<double>& returns, double target_return = 0.0);
    double calculateMaxDrawdown(const std::vector<double>& values);
    double calculateCalmarRatio(const std::vector<double>& returns);
    double calculateInformationRatio(const std::vector<double>& portfolio_returns,
                                   const std::vector<double>& benchmark_returns);
    
    // Risk metrics
    double calculateVaR(const std::vector<double>& returns, double confidence_level = 0.05);
    double calculateCVaR(const std::vector<double>& returns, double confidence_level = 0.05);
    double calculateBeta(const std::vector<double>& portfolio_returns,
                        const std::vector<double>& market_returns);
    
    // Portfolio composition analysis
    std::map<std::string, double> calculateRiskContributions(
        const std::map<std::string, double>& weights,
        const std::vector<std::vector<double>>& cov_matrix,
        const std::vector<std::string>& asset_names);
    
    double calculateConcentrationRisk(const std::map<std::string, double>& weights);
    double calculateEffectiveNumberOfAssets(const std::map<std::string, double>& weights);
    
    // Correlation analysis
    std::vector<std::vector<double>> calculateCorrelationMatrix(
        const std::map<std::string, std::vector<double>>& returns);
    
    double calculateAverageCorrelation(const std::vector<std::vector<double>>& corr_matrix);
    
    // Backtesting utilities
    std::vector<double> simulatePortfolioReturns(
        const std::map<std::string, double>& weights,
        const std::map<std::string, std::vector<double>>& asset_returns);
    
    PerformanceMetrics calculateBacktestMetrics(
        const std::vector<double>& portfolio_returns,
        double risk_free_rate = Constants::RISK_FREE_RATE);
}

} // namespace tradingcore
} // namespace parsec