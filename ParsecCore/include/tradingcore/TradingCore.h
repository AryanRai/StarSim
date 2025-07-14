#pragma once

// Core TradingCore aggregator header - includes all trading functionality
// This provides the main public API for TradingCore

#include "TradingDataStructures.h"
#include "static/TradingConstants.h"
#include "modules/MarketData.h"
#include "modules/SignalGenerator.h"
#include "modules/PortfolioManager.h"
#include "modules/TradeExecutor.h"
#include "modules/SimulationEngine.h"

#include "parsec/ModelConfig.h"
#include "mlcore/MLCore.h"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace parsec {
namespace tradingcore {

// Forward declarations
class MarketDataManager;
class SignalGenerator;
class PortfolioManager;
class TradeExecutor;
class TradingSimulationSuite;

// Main TradingCore configuration
struct TradingCoreConfig {
    // Market data configuration
    MarketDataConfig market_data_config;
    
    // API configuration for trading
    APIConfig trading_api_config;
    
    // Execution configuration
    ExecutionConfig execution_config;
    
    // Portfolio management
    RebalancingConfig rebalancing_config;
    OptimizationParams optimization_params;
    PortfolioConstraints portfolio_constraints;
    
    // Risk management
    RiskParams risk_params;
    
    // Strategy configuration
    std::vector<StrategyConfig> strategy_configs;
    
    // Simulation settings
    SimulationConfig simulation_config;
    
    // ML integration (optional)
    bool enable_ml_features = false;
    std::string ml_config_path;
    
    // Operational settings
    bool paper_trading_mode = true;
    bool enable_real_time_data = false;
    bool enable_logging = true;
    std::string log_level = "INFO";
    
    TradingCoreConfig() = default;
    
    // Validation
    bool validate() const;
    std::vector<std::string> getValidationErrors() const;
    
    // Factory methods for common configurations
    static TradingCoreConfig createDefaultConfig();
    static TradingCoreConfig createBacktestConfig();
    static TradingCoreConfig createPaperTradingConfig();
    static TradingCoreConfig createLiveTradingConfig();
};

// Main TradingCore orchestrator class
class TradingCore {
public:
    explicit TradingCore(const TradingCoreConfig& config);
    ~TradingCore();
    
    // === INITIALIZATION AND CONFIGURATION ===
    
    bool initialize();
    void shutdown();
    bool isInitialized() const { return initialized_; }
    
    // Configuration management
    void setConfig(const TradingCoreConfig& config);
    TradingCoreConfig getConfig() const { return config_; }
    
    bool loadConfigFromFile(const std::string& config_file);
    bool saveConfigToFile(const std::string& config_file) const;
    
    // === MARKET DATA MANAGEMENT ===
    
    // Add market data symbols
    void addSymbol(const std::string& symbol);
    void addSymbols(const std::vector<std::string>& symbols);
    void removeSymbol(const std::string& symbol);
    std::vector<std::string> getSymbols() const;
    
    // Historical data
    HistoricalData getHistoricalData(const std::string& symbol, 
                                   const std::string& timeframe = "",
                                   int bars = 1000);
    
    std::map<std::string, HistoricalData> getMultipleHistoricalData(
        const std::vector<std::string>& symbols,
        const std::string& timeframe = "",
        int bars = 1000);
    
    // Real-time data
    MarketDataPoint getCurrentPrice(const std::string& symbol);
    std::map<std::string, MarketDataPoint> getCurrentPrices(const std::vector<std::string>& symbols);
    
    bool startRealTimeData(MarketDataCallback callback);
    void stopRealTimeData();
    
    // === STRATEGY MANAGEMENT ===
    
    // Strategy configuration
    void addStrategy(const StrategyConfig& strategy_config);
    void removeStrategy(const std::string& strategy_name);
    void updateStrategy(const std::string& strategy_name, const StrategyConfig& config);
    std::vector<std::string> getStrategyNames() const;
    
    // Signal generation
    std::vector<TradingSignal> generateSignals();
    std::vector<TradingSignal> generateSignals(const std::string& symbol);
    TradingSignal generateConsensusSignal(const std::string& symbol);
    
    // Strategy performance
    std::map<std::string, PerformanceMetrics> getStrategyPerformance() const;
    
    // === PORTFOLIO MANAGEMENT ===
    
    // Portfolio status
    Portfolio getCurrentPortfolio() const;
    std::map<std::string, double> getCurrentWeights() const;
    std::map<std::string, double> getTargetWeights() const;
    
    // Portfolio optimization
    std::map<std::string, double> optimizePortfolio(OptimizationMethod method = OptimizationMethod::MEAN_VARIANCE);
    void setTargetWeights(const std::map<std::string, double>& weights);
    
    // Rebalancing
    bool shouldRebalance();
    std::vector<Order> generateRebalanceOrders();
    void executeRebalance();
    
    // Performance tracking
    PerformanceMetrics getPortfolioPerformance() const;
    double getPortfolioValue() const;
    std::vector<double> getValueHistory() const;
    
    // === TRADE EXECUTION ===
    
    // Order management
    std::string submitOrder(const Order& order);
    bool cancelOrder(const std::string& order_id);
    std::vector<Order> getActiveOrders() const;
    std::vector<Trade> getRecentTrades(int hours = 24) const;
    
    // Signal execution
    std::vector<std::string> executeSignals(const std::vector<TradingSignal>& signals);
    std::string executeSignal(const TradingSignal& signal);
    
    // Batch operations
    std::vector<std::string> executeBatchOrders(const std::vector<Order>& orders);
    
    // === RISK MANAGEMENT ===
    
    // Risk monitoring
    std::map<std::string, double> getRiskMetrics() const;
    bool checkRiskLimits() const;
    std::vector<std::string> getRiskAlerts() const;
    
    // Risk controls
    void setRiskParams(const RiskParams& params);
    RiskParams getRiskParams() const { return config_.risk_params; }
    
    // Emergency controls
    void enableKillSwitch(bool enable = true);
    void cancelAllOrders();
    bool isKillSwitchActive() const;
    
    // === SIMULATION AND BACKTESTING ===
    
    // Backtesting
    SimulationResult runBacktest(const Timestamp& start_date, const Timestamp& end_date);
    SimulationResult runBacktest(const std::vector<std::string>& symbols,
                               const Timestamp& start_date, const Timestamp& end_date);
    
    // Monte Carlo simulation
    MonteCarloResult runMonteCarloSimulation(const MonteCarloConfig& config);
    
    // Walk-forward analysis
    std::map<std::string, SimulationResult> runWalkForwardAnalysis(const WalkForwardConfig& config);
    
    // Strategy optimization
    std::map<std::string, double> optimizeStrategyParameters(const std::string& strategy_name,
                                                           const std::map<std::string, std::pair<double, double>>& ranges);
    
    // === ML INTEGRATION ===
    
    // ML configuration (if enabled)
    bool loadMLConfiguration(const std::string& ml_config_path);
    bool isMLEnabled() const { return false; } // Simplified for basic build
    
    // ML-enhanced signals
    std::vector<TradingSignal> generateMLEnhancedSignals();
    void trainMLModels(const std::map<std::string, HistoricalData>& training_data);
    
    // ML performance
    std::map<std::string, double> getMLModelPerformance() const;
    
    // === EVENT HANDLING AND CALLBACKS ===
    
    // Event callbacks
    void setMarketDataCallback(MarketDataCallback callback);
    void setSignalCallback(SignalCallback callback);
    void setTradeCallback(TradeCallback callback);
    void setOrderCallback(OrderCallback callback);
    void setErrorCallback(ErrorCallback callback);
    
    // Portfolio events
    void setRebalanceCallback(std::function<void(const std::vector<Order>&)> callback);
    void setRiskAlertCallback(std::function<void(const std::string&, double)> callback);
    
    // === STATUS AND DIAGNOSTICS ===
    
    // Connection status
    bool isConnected() const;
    std::map<std::string, std::string> getConnectionStatus() const;
    
    // System health
    std::map<std::string, std::string> getSystemStatus() const;
    std::vector<std::string> getErrorLog() const;
    
    // Performance statistics
    std::map<std::string, double> getSystemMetrics() const;
    
    // === UTILITY METHODS ===
    
    // Data export/import
    bool exportData(const std::string& filename, const std::string& format = "csv") const;
    bool importData(const std::string& filename, const std::string& format = "csv");
    
    // Configuration templates
    static StrategyConfig createMovingAverageConfig(const std::string& name, 
                                                  const std::vector<std::string>& symbols,
                                                  int short_period = 10, int long_period = 20);
    
    static StrategyConfig createRSIConfig(const std::string& name,
                                        const std::vector<std::string>& symbols,
                                        int period = 14, double oversold = 30, double overbought = 70);
    
    static PortfolioConstraints createBasicConstraints(double max_position = 0.2, double min_cash = 0.05);
    
private:
    TradingCoreConfig config_;
    bool initialized_;
    
    // Internal state
    std::vector<std::string> symbols_;
    
    // Error tracking
    std::vector<std::string> error_log_;
    
    // Initialization methods
    bool initializeMarketData();
    bool initializeSignalGeneration();
    bool initializePortfolioManagement();
    bool initializeTradeExecution();
    bool initializeSimulation();
    bool initializeMLIntegration();
    
    // Validation methods
    bool validateConfiguration() const;
    bool validateConnections() const;
    
    // Error handling
    void logError(const std::string& error_message);
    void handleError(const std::string& component, const std::string& error);
    
    // Component coordination
    void synchronizeComponents();
    void updatePortfolioFromTrades();
    void processSignalsToOrders(const std::vector<TradingSignal>& signals);
    
    // Internal callbacks
    void onMarketDataReceived(const MarketDataPoint& data);
    void onSignalGenerated(const TradingSignal& signal);
    void onTradeExecuted(const Trade& trade);
    void onOrderStatusChanged(const Order& order);
};

// Factory functions for easy setup
namespace TradingCoreFactory {
    // Pre-configured TradingCore instances
    std::unique_ptr<TradingCore> createBacktestingInstance(const std::vector<std::string>& symbols);
    std::unique_ptr<TradingCore> createPaperTradingInstance(const std::vector<std::string>& symbols,
                                                          const APIConfig& api_config);
    std::unique_ptr<TradingCore> createLiveTradingInstance(const std::vector<std::string>& symbols,
                                                         const APIConfig& api_config);
    
    // Configuration builders
    TradingCoreConfig buildMovingAverageStrategy(const std::vector<std::string>& symbols,
                                               int short_period = 10, int long_period = 20);
    
    TradingCoreConfig buildPortfolioOptimizationStrategy(const std::vector<std::string>& symbols,
                                                       OptimizationMethod method = OptimizationMethod::MEAN_VARIANCE);
    
    TradingCoreConfig buildMLEnhancedStrategy(const std::vector<std::string>& symbols,
                                            const std::string& ml_config_path);
}

} // namespace tradingcore
} // namespace parsec