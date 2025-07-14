#pragma once

#include "tradingcore/TradingDataStructures.h"
#include "tradingcore/modules/MarketData.h"
#include "tradingcore/modules/SignalGenerator.h"
#include "tradingcore/modules/PortfolioManager.h"
#include "parsec/SolverCore.h"
#include <memory>
#include <random>
#include <optional>

namespace parsec {
namespace tradingcore {

// Simulation configuration
struct SimulationConfig {
    double initial_capital = 10000.0;
    Timestamp start_date;
    Timestamp end_date;
    
    // Transaction costs
    double commission_per_trade = 0.0;
    double commission_rate = 0.0;      // As percentage of trade value
    double bid_ask_spread_bps = 5.0;   // Bid-ask spread in basis points
    double slippage_bps = 2.0;         // Market impact slippage
    
    // Realism factors
    bool enable_transaction_costs = true;
    bool enable_slippage = true;
    bool enable_market_hours = true;   // Only trade during market hours
    bool enable_liquidity_constraints = false; // Consider volume limits
    
    // Position limits
    double max_position_size = 0.2;    // 20% max position
    double max_leverage = 1.0;         // No leverage
    bool allow_short_selling = false;
    
    // Frequency
    std::string rebalance_frequency = "daily"; // "intraday", "daily", "weekly", "monthly"
    int intraday_frequency_minutes = 60;       // For intraday simulations
    
    SimulationConfig() {
        // Default to 1 year backtest
        auto now = std::chrono::system_clock::now();
        end_date = now;
        start_date = now - std::chrono::hours(24 * 365);
    }
};

// Simulation results
struct SimulationResult {
    // Performance metrics
    PerformanceMetrics performance;
    
    // Time series data
    std::vector<double> portfolio_values;
    std::vector<double> returns;
    std::vector<Timestamp> timestamps;
    std::vector<double> drawdowns;
    std::vector<double> cash_levels;
    
    // Trading activity
    std::vector<Trade> all_trades;
    std::vector<TradingSignal> all_signals;
    std::map<std::string, int> trades_per_symbol;
    
    // Statistics
    int total_periods;
    double total_commission_paid;
    double total_slippage_cost;
    double gross_return;
    double net_return;
    
    // Risk metrics
    double var_95;           // 95% Value at Risk
    double max_drawdown_pct;
    double calmar_ratio;
    double sortino_ratio;
    
    // Strategy-specific metrics
    std::map<std::string, PerformanceMetrics> strategy_performance;
    std::map<std::string, double> strategy_contributions;
    
    SimulationResult() : total_periods(0), total_commission_paid(0.0), total_slippage_cost(0.0),
                        gross_return(0.0), net_return(0.0), var_95(0.0), max_drawdown_pct(0.0),
                        calmar_ratio(0.0), sortino_ratio(0.0) {}
    
    // Utility methods
    double getFinalValue() const { return portfolio_values.empty() ? 0.0 : portfolio_values.back(); }
    double getTotalReturn() const { return net_return; }
    double getAnnualizedReturn() const;
    double getVolatility() const;
    double getSharpeRatio(double risk_free_rate = Constants::RISK_FREE_RATE) const;
};

// Monte Carlo simulation parameters
struct MonteCarloConfig {
    int num_simulations = 1000;
    int forecast_periods = 252;        // 1 year daily
    bool use_bootstrapping = true;     // Bootstrap historical returns
    bool use_geometric_brownian = false; // Use GBM for price simulation
    
    // Return distribution parameters (if not bootstrapping)
    double annual_return = 0.08;
    double annual_volatility = 0.15;
    
    // Correlation structure
    bool preserve_correlations = true;
    std::vector<std::vector<double>> correlation_matrix;
    
    // Random seed for reproducibility
    std::optional<int> random_seed;
    
    MonteCarloConfig() = default;
};

// Monte Carlo simulation results
struct MonteCarloResult {
    std::vector<std::vector<double>> simulation_paths; // [simulation][time_point]
    std::vector<double> final_values;
    std::vector<double> max_drawdowns;
    std::vector<double> returns;
    
    // Statistical summary
    double mean_final_value;
    double median_final_value;
    double std_final_value;
    double percentile_5;
    double percentile_95;
    double probability_of_loss;
    double probability_of_target; // Probability of reaching target return
    
    // Risk metrics
    double expected_shortfall_5;  // Expected loss in worst 5% scenarios
    double maximum_drawdown_95;   // 95th percentile max drawdown
    
    MonteCarloResult() : mean_final_value(0.0), median_final_value(0.0), std_final_value(0.0),
                        percentile_5(0.0), percentile_95(0.0), probability_of_loss(0.0),
                        probability_of_target(0.0), expected_shortfall_5(0.0), maximum_drawdown_95(0.0) {}
};

// Walk-forward analysis configuration
struct WalkForwardConfig {
    int training_periods = 252;       // Training window size
    int validation_periods = 63;      // Out-of-sample test periods
    int step_size = 21;               // Step forward by 21 days
    bool reoptimize_parameters = true; // Reoptimize on each step
    bool enable_parameter_stability = false; // Check parameter stability
    
    // Parameter ranges for optimization
    std::map<std::string, std::pair<double, double>> parameter_ranges;
    
    WalkForwardConfig() = default;
};

// Base class for simulation engines
class SimulationEngine {
public:
    explicit SimulationEngine(const SimulationConfig& config);
    virtual ~SimulationEngine() = default;
    
    // Configuration
    virtual void setConfig(const SimulationConfig& config) { config_ = config; }
    SimulationConfig getConfig() const { return config_; }
    
    // Data management
    virtual void setMarketData(const std::map<std::string, HistoricalData>& data);
    virtual void addMarketData(const std::string& symbol, const HistoricalData& data);
    
    // Core simulation methods
    virtual SimulationResult runBacktest() = 0;
    virtual SimulationResult runBacktest(const std::vector<std::string>& symbols) = 0;
    
    // Analysis methods
    virtual MonteCarloResult runMonteCarloSimulation(const MonteCarloConfig& mc_config) = 0;
    virtual std::map<std::string, SimulationResult> runWalkForwardAnalysis(const WalkForwardConfig& wf_config) = 0;
    
    // Utility methods
    virtual bool validateData() const;
    virtual std::vector<std::string> getAvailableSymbols() const;
    virtual void reset();
    
protected:
    SimulationConfig config_;
    std::map<std::string, HistoricalData> market_data_;
    
    // Helper methods
    double calculateTransactionCost(const Trade& trade) const;
    double calculateSlippage(const Order& order, const OHLCV& market_bar) const;
    bool isMarketOpen(const Timestamp& timestamp) const;
    Timestamp getNextTradingTime(const Timestamp& current_time) const;
};

// Strategy backtesting engine
class StrategyBacktester : public SimulationEngine {
public:
    explicit StrategyBacktester(const SimulationConfig& config);
    ~StrategyBacktester() override;
    
    // Strategy management
    void addStrategy(std::unique_ptr<TradingStrategy> strategy);
    void removeStrategy(const std::string& strategy_name);
    void setStrategyWeight(const std::string& strategy_name, double weight);
    std::vector<std::string> getStrategyNames() const;
    
    // Signal generation configuration
    void setSignalGenerator(std::unique_ptr<SignalGenerator> generator);
    SignalGenerator* getSignalGenerator() const { return signal_generator_.get(); }
    
    // Core simulation methods
    SimulationResult runBacktest() override;
    SimulationResult runBacktest(const std::vector<std::string>& symbols) override;
    
    // Advanced analysis
    MonteCarloResult runMonteCarloSimulation(const MonteCarloConfig& mc_config) override;
    std::map<std::string, SimulationResult> runWalkForwardAnalysis(const WalkForwardConfig& wf_config) override;
    
    // Parameter optimization
    std::map<std::string, double> optimizeParameters(const std::string& strategy_name,
                                                   const std::map<std::string, std::pair<double, double>>& ranges);
    
    // Multiple strategy analysis
    SimulationResult runMultiStrategyBacktest(const std::map<std::string, double>& strategy_weights);
    std::map<std::string, double> optimizeStrategyWeights();
    
private:
    std::unique_ptr<SignalGenerator> signal_generator_;
    std::map<std::string, double> strategy_weights_;
    
    // Internal simulation state
    Portfolio current_portfolio_;
    std::vector<TradingSignal> signal_history_;
    std::vector<Trade> trade_history_;
    
    // Simulation methods
    void initializeSimulation();
    void processTimeStep(const Timestamp& timestamp);
    std::vector<TradingSignal> generateSignalsForTimestamp(const Timestamp& timestamp);
    std::vector<Trade> executeSignals(const std::vector<TradingSignal>& signals, const Timestamp& timestamp);
    void updatePortfolioValue(const Timestamp& timestamp);
    
    // Order execution simulation
    Trade simulateOrderExecution(const Order& order, const OHLCV& market_bar);
    Price getExecutionPrice(const Order& order, const OHLCV& market_bar);
    
    // Performance calculation
    PerformanceMetrics calculatePerformanceMetrics(const SimulationResult& result);
    void updateDrawdowns(std::vector<double>& drawdowns, const std::vector<double>& values);
};

// Portfolio optimization backtester
class PortfolioBacktester : public SimulationEngine {
public:
    explicit PortfolioBacktester(const SimulationConfig& config);
    ~PortfolioBacktester() override;
    
    // Portfolio management
    void setPortfolioManager(std::unique_ptr<PortfolioManager> manager);
    PortfolioManager* getPortfolioManager() const { return portfolio_manager_.get(); }
    
    // Asset universe management
    void setAssetUniverse(const std::vector<std::string>& symbols);
    std::vector<std::string> getAssetUniverse() const { return asset_universe_; }
    
    // Core simulation methods
    SimulationResult runBacktest() override;
    SimulationResult runBacktest(const std::vector<std::string>& symbols) override;
    
    // Advanced analysis
    MonteCarloResult runMonteCarloSimulation(const MonteCarloConfig& mc_config) override;
    std::map<std::string, SimulationResult> runWalkForwardAnalysis(const WalkForwardConfig& wf_config) override;
    
    // Portfolio-specific analysis
    SimulationResult runRebalancingAnalysis(const std::vector<int>& rebalance_frequencies);
    std::map<OptimizationMethod, SimulationResult> compareOptimizationMethods();
    
    // Risk analysis
    std::map<std::string, double> analyzeRiskContributions();
    std::vector<double> calculateRollingMetrics(const std::string& metric, int window_size);
    
private:
    std::unique_ptr<PortfolioManager> portfolio_manager_;
    std::vector<std::string> asset_universe_;
    
    // Internal state
    std::map<std::string, double> current_weights_;
    std::vector<Timestamp> rebalance_dates_;
    
    // Portfolio-specific simulation methods
    void performRebalancing(const Timestamp& timestamp);
    std::map<std::string, double> calculateOptimalWeights(const Timestamp& timestamp);
    std::vector<Trade> generateRebalanceTrades(const std::map<std::string, double>& target_weights,
                                             const Timestamp& timestamp);
};

// Comprehensive simulation suite
class TradingSimulationSuite {
public:
    explicit TradingSimulationSuite(const SimulationConfig& config);
    ~TradingSimulationSuite();
    
    // Configuration
    void setConfig(const SimulationConfig& config);
    void setMarketData(const std::map<std::string, HistoricalData>& data);
    
    // Comprehensive analysis
    struct ComprehensiveResults {
        SimulationResult strategy_backtest;
        SimulationResult portfolio_backtest;
        MonteCarloResult monte_carlo;
        std::map<std::string, SimulationResult> walk_forward;
        std::map<std::string, double> sensitivity_analysis;
        std::map<std::string, double> robustness_metrics;
    };
    
    ComprehensiveResults runComprehensiveAnalysis(const std::vector<std::string>& symbols);
    
    // Individual analysis components
    SimulationResult runStrategyAnalysis(const std::vector<std::string>& symbols);
    SimulationResult runPortfolioAnalysis(const std::vector<std::string>& symbols);
    MonteCarloResult runRiskAnalysis(const MonteCarloConfig& mc_config);
    
    // Sensitivity analysis
    std::map<std::string, double> runSensitivityAnalysis(const std::string& parameter_name,
                                                        const std::vector<double>& parameter_values);
    
    // Robustness testing
    std::map<std::string, double> runRobustnessTests();
    
    // Stress testing
    SimulationResult runStressTest(const std::string& scenario_name);
    std::map<std::string, SimulationResult> runMultipleStressTests();
    
    // Reporting
    std::string generateReport(const ComprehensiveResults& results);
    void exportResults(const ComprehensiveResults& results, const std::string& filename);
    
private:
    SimulationConfig config_;
    std::map<std::string, HistoricalData> market_data_;
    
    std::unique_ptr<StrategyBacktester> strategy_backtester_;
    std::unique_ptr<PortfolioBacktester> portfolio_backtester_;
    
    // Stress testing scenarios
    std::map<std::string, std::function<void(std::map<std::string, HistoricalData>&)>> stress_scenarios_;
    
    void initializeStressScenarios();
    void applyMarketCrashScenario(std::map<std::string, HistoricalData>& data);
    void applyHighVolatilityScenario(std::map<std::string, HistoricalData>& data);
    void applyInterestRateShockScenario(std::map<std::string, HistoricalData>& data);
};

// Simulation utilities
namespace SimulationUtils {
    // Data preparation
    std::map<std::string, HistoricalData> alignDataTimestamps(
        const std::map<std::string, HistoricalData>& data);
    
    std::map<std::string, HistoricalData> fillMissingData(
        const std::map<std::string, HistoricalData>& data);
    
    // Performance calculation
    PerformanceMetrics calculateMetrics(const std::vector<double>& returns,
                                      double risk_free_rate = Constants::RISK_FREE_RATE);
    
    double calculateMaxDrawdown(const std::vector<double>& values);
    std::vector<double> calculateRollingReturns(const std::vector<double>& values, int window);
    
    // Statistical analysis
    std::vector<double> calculateBootstrapReturns(const std::vector<double>& historical_returns,
                                                 int num_samples, std::mt19937& rng);
    
    std::vector<std::vector<double>> generateCorrelatedReturns(
        const std::vector<std::vector<double>>& correlation_matrix,
        const std::vector<double>& means,
        const std::vector<double>& std_devs,
        int num_samples, std::mt19937& rng);
    
    // Risk analysis
    double calculateVaR(const std::vector<double>& returns, double confidence_level = 0.05);
    double calculateExpectedShortfall(const std::vector<double>& returns, double confidence_level = 0.05);
    
    // Validation
    bool validateSimulationConfig(const SimulationConfig& config);
    bool validateMarketData(const std::map<std::string, HistoricalData>& data);
    
    // Reporting
    std::string formatPerformanceMetrics(const PerformanceMetrics& metrics);
    std::string formatSimulationSummary(const SimulationResult& result);
}

} // namespace tradingcore
} // namespace parsec