#pragma once

#include "tradingcore/TradingDataStructures.h"
#include "tradingcore/modules/MarketData.h"
#include "mlcore/MLCore.h"
#include "mathcore/math.h"
#include <memory>
#include <map>
#include <functional>

namespace parsec {
namespace tradingcore {

// Base class for all trading strategies
class TradingStrategy {
public:
    explicit TradingStrategy(const StrategyConfig& config);
    virtual ~TradingStrategy() = default;
    
    // Core strategy interface
    virtual TradingSignal generateSignal(const HistoricalData& data) = 0;
    virtual std::vector<TradingSignal> generateSignals(const std::map<std::string, HistoricalData>& data) = 0;
    
    // Strategy management
    virtual bool initialize() = 0;
    virtual bool validate() const = 0;
    virtual void reset() = 0;
    
    // Configuration
    StrategyConfig getConfig() const { return config_; }
    void setConfig(const StrategyConfig& config);
    void updateParameter(const std::string& key, double value);
    
    // Performance tracking
    virtual PerformanceMetrics getPerformanceMetrics() const;
    virtual void updatePerformance(const Trade& trade);
    
    // Strategy info
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual std::vector<std::string> getRequiredIndicators() const = 0;
    virtual int getMinimumBars() const = 0;
    
protected:
    StrategyConfig config_;
    PerformanceMetrics performance_;
    std::vector<Trade> trade_history_;
    
    // Utility methods for derived classes
    double getParameter(const std::string& key, double default_val = 0.0) const;
    TradingSignal createSignal(const std::string& symbol, SignalType type, 
                             Price price, double confidence, const std::string& reason = "");
};

// Moving Average Crossover Strategy
class MovingAverageCrossoverStrategy : public TradingStrategy {
public:
    explicit MovingAverageCrossoverStrategy(const StrategyConfig& config);
    
    TradingSignal generateSignal(const HistoricalData& data) override;
    std::vector<TradingSignal> generateSignals(const std::map<std::string, HistoricalData>& data) override;
    
    bool initialize() override;
    bool validate() const override;
    void reset() override;
    
    std::string getName() const override { return "MovingAverageCrossover"; }
    std::string getDescription() const override { return "Buy when short MA crosses above long MA, sell when below"; }
    std::vector<std::string> getRequiredIndicators() const override { return {"SMA"}; }
    int getMinimumBars() const override;
    
private:
    int short_period_;
    int long_period_;
    std::map<std::string, bool> last_signal_state_; // Track crossover state per symbol
    
    bool detectCrossover(const std::vector<double>& short_ma, const std::vector<double>& long_ma);
};

// RSI Mean Reversion Strategy
class RSIMeanReversionStrategy : public TradingStrategy {
public:
    explicit RSIMeanReversionStrategy(const StrategyConfig& config);
    
    TradingSignal generateSignal(const HistoricalData& data) override;
    std::vector<TradingSignal> generateSignals(const std::map<std::string, HistoricalData>& data) override;
    
    bool initialize() override;
    bool validate() const override;
    void reset() override;
    
    std::string getName() const override { return "RSIMeanReversion"; }
    std::string getDescription() const override { return "Buy when RSI oversold, sell when overbought"; }
    std::vector<std::string> getRequiredIndicators() const override { return {"RSI"}; }
    int getMinimumBars() const override { return rsi_period_ + 10; }
    
private:
    int rsi_period_;
    double oversold_threshold_;
    double overbought_threshold_;
    double neutral_zone_lower_;
    double neutral_zone_upper_;
};

// Bollinger Bands Strategy
class BollingerBandsStrategy : public TradingStrategy {
public:
    explicit BollingerBandsStrategy(const StrategyConfig& config);
    
    TradingSignal generateSignal(const HistoricalData& data) override;
    std::vector<TradingSignal> generateSignals(const std::map<std::string, HistoricalData>& data) override;
    
    bool initialize() override;
    bool validate() const override;
    void reset() override;
    
    std::string getName() const override { return "BollingerBands"; }
    std::string getDescription() const override { return "Mean reversion strategy using Bollinger Bands"; }
    std::vector<std::string> getRequiredIndicators() const override { return {"SMA", "STDDEV"}; }
    int getMinimumBars() const override { return period_ + 10; }
    
private:
    int period_;
    double num_std_dev_;
    bool squeeze_filter_; // Only trade during low volatility
    
    struct BollingerBands {
        std::vector<double> upper;
        std::vector<double> middle;
        std::vector<double> lower;
        std::vector<double> bandwidth;
    };
    
    BollingerBands calculateBands(const std::vector<Price>& prices);
};

// ML-Enhanced Strategy (uses MLCore for predictions)
class MLEnhancedStrategy : public TradingStrategy {
public:
    explicit MLEnhancedStrategy(const StrategyConfig& config, std::shared_ptr<parsec::MLCore> ml_core);
    
    TradingSignal generateSignal(const HistoricalData& data) override;
    std::vector<TradingSignal> generateSignals(const std::map<std::string, HistoricalData>& data) override;
    
    bool initialize() override;
    bool validate() const override;
    void reset() override;
    
    std::string getName() const override { return "MLEnhanced"; }
    std::string getDescription() const override { return "ML-enhanced signals using Random Forest predictions"; }
    std::vector<std::string> getRequiredIndicators() const override { return {"SMA", "RSI", "ATR"}; }
    int getMinimumBars() const override { return 100; }
    
    // ML-specific methods
    bool trainModel(const std::map<std::string, HistoricalData>& training_data);
    double getPredictionConfidence() const;
    void enableOnlineLearning(bool enable = true);
    
private:
    std::shared_ptr<parsec::MLCore> ml_core_;
    std::string ml_model_name_;
    bool model_trained_;
    bool online_learning_enabled_;
    
    // Feature engineering
    std::map<std::string, double> extractFeatures(const HistoricalData& data);
    void updateFeatures(const std::map<std::string, double>& features, const TradingSignal& signal);
};

// Multi-Asset Momentum Strategy
class MomentumStrategy : public TradingStrategy {
public:
    explicit MomentumStrategy(const StrategyConfig& config);
    
    TradingSignal generateSignal(const HistoricalData& data) override;
    std::vector<TradingSignal> generateSignals(const std::map<std::string, HistoricalData>& data) override;
    
    bool initialize() override;
    bool validate() const override;
    void reset() override;
    
    std::string getName() const override { return "Momentum"; }
    std::string getDescription() const override { return "Momentum-based strategy with multiple timeframes"; }
    std::vector<std::string> getRequiredIndicators() const override { return {"ROC", "SMA"}; }
    int getMinimumBars() const override { return std::max({short_period_, medium_period_, long_period_}) + 10; }
    
private:
    int short_period_;
    int medium_period_;
    int long_period_;
    double momentum_threshold_;
    bool use_volatility_filter_;
    
    double calculateMomentumScore(const HistoricalData& data);
    bool passesVolatilityFilter(const HistoricalData& data);
};

// Signal Generator Manager
class SignalGenerator {
public:
    SignalGenerator();
    ~SignalGenerator();
    
    // Strategy management
    void addStrategy(std::unique_ptr<TradingStrategy> strategy);
    void removeStrategy(const std::string& strategy_name);
    TradingStrategy* getStrategy(const std::string& strategy_name);
    std::vector<std::string> getStrategyNames() const;
    
    // Signal generation
    std::vector<TradingSignal> generateSignals(const std::string& symbol, const HistoricalData& data);
    std::vector<TradingSignal> generateSignals(const std::map<std::string, HistoricalData>& data);
    
    // Multi-strategy signals with consensus
    TradingSignal generateConsensusSignal(const std::string& symbol, const HistoricalData& data);
    std::map<std::string, TradingSignal> generateConsensusSignals(const std::map<std::string, HistoricalData>& data);
    
    // Signal filtering and validation
    void addSignalFilter(std::function<bool(const TradingSignal&)> filter);
    void clearSignalFilters();
    bool validateSignal(const TradingSignal& signal);
    
    // Signal scoring and ranking
    void setSignalWeights(const std::map<std::string, double>& weights);
    std::vector<TradingSignal> rankSignals(const std::vector<TradingSignal>& signals);
    
    // Performance tracking
    void updatePerformance(const std::string& strategy_name, const Trade& trade);
    std::map<std::string, PerformanceMetrics> getPerformanceMetrics();
    
    // Configuration
    void setMinConfidence(double min_confidence);
    void setMaxSignalsPerSymbol(int max_signals);
    void enableSignalCaching(bool enable = true);
    
    // Signal callbacks
    void setSignalCallback(SignalCallback callback);
    
private:
    std::map<std::string, std::unique_ptr<TradingStrategy>> strategies_;
    std::vector<std::function<bool(const TradingSignal&)>> signal_filters_;
    std::map<std::string, double> strategy_weights_;
    
    // Configuration
    double min_confidence_;
    int max_signals_per_symbol_;
    bool caching_enabled_;
    
    // Callbacks
    SignalCallback signal_callback_;
    
    // Caching
    std::map<std::string, std::pair<TradingSignal, Timestamp>> signal_cache_;
    
    // Helper methods
    TradingSignal combineSignals(const std::vector<TradingSignal>& signals, const std::string& symbol);
    double calculateConsensusConfidence(const std::vector<TradingSignal>& signals);
    SignalType determineConsensusType(const std::vector<TradingSignal>& signals);
    bool isSignalCacheValid(const std::string& cache_key);
    void notifySignalGenerated(const TradingSignal& signal);
};

// Factory for creating trading strategies
class StrategyFactory {
public:
    static std::unique_ptr<TradingStrategy> createStrategy(const StrategyConfig& config);
    static std::unique_ptr<TradingStrategy> createStrategy(const StrategyConfig& config,
                                                         std::shared_ptr<parsec::MLCore> ml_core);
    
    static std::vector<std::string> getAvailableStrategies();
    static StrategyConfig getDefaultConfig(const std::string& strategy_type);
    
    // Strategy template registration
    template<typename StrategyType>
    static void registerStrategy(const std::string& name);
    
private:
    static std::map<std::string, std::function<std::unique_ptr<TradingStrategy>(const StrategyConfig&)>> strategy_creators_;
    static std::map<std::string, StrategyConfig> default_configs_;
};

// Signal analysis utilities
namespace SignalUtils {
    // Signal statistics
    double calculateSignalAccuracy(const std::vector<TradingSignal>& signals, 
                                 const std::vector<Trade>& trades);
    
    double calculateSignalProfitability(const std::vector<TradingSignal>& signals,
                                       const std::vector<Trade>& trades);
    
    std::map<SignalType, int> getSignalDistribution(const std::vector<TradingSignal>& signals);
    
    // Signal quality metrics
    double calculateSignalSharpe(const std::vector<TradingSignal>& signals,
                                const std::vector<Trade>& trades);
    
    double calculateSignalDrawdown(const std::vector<TradingSignal>& signals,
                                  const std::vector<Trade>& trades);
    
    // Signal correlation analysis
    double calculateSignalCorrelation(const std::vector<TradingSignal>& signals1,
                                     const std::vector<TradingSignal>& signals2);
    
    // Signal timing analysis
    std::map<std::string, double> analyzeSignalTiming(const std::vector<TradingSignal>& signals);
    
    // Signal validation
    bool isSignalReasonable(const TradingSignal& signal, const HistoricalData& data);
    std::vector<std::string> validateSignalQuality(const TradingSignal& signal);
}

} // namespace tradingcore
} // namespace parsec