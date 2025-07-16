#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <iomanip>
#include "tradingcore/TradingCore.h"
#include "mlcore/MLCore.h"
#include "parsec/ParsecCore.h"

using namespace parsec;
using namespace parsec::tradingcore;

class AdvancedPaperTradingBot {
private:
    std::unique_ptr<TradingCore> trading_core_;
    std::shared_ptr<MLCore> ml_core_;
    std::vector<std::string> universe_;
    
    // Performance tracking
    double initial_capital_;
    double current_portfolio_value_;
    std::vector<double> daily_returns_;
    std::chrono::steady_clock::time_point start_time_;
    
    // Strategy parameters
    struct StrategyParams {
        // Momentum Strategy
        int momentum_lookback = 20;
        double momentum_threshold = 0.02;
        
        // Mean Reversion Strategy
        int bollinger_period = 20;
        double bollinger_std_dev = 2.0;
        
        // ML Strategy
        int ml_lookback = 60;
        double ml_confidence_threshold = 0.7;
        
        // Risk Management
        double max_position_size = 0.15;  // 15% max per position
        double daily_var_limit = 0.02;    // 2% daily VaR limit
        double stop_loss_pct = 0.05;      // 5% stop loss
        double take_profit_pct = 0.10;    // 10% take profit
    } params_;
    
    // Market regime detection
    enum class MarketRegime {
        BULL_MARKET,
        BEAR_MARKET,
        SIDEWAYS_MARKET,
        HIGH_VOLATILITY
    };
    
    MarketRegime current_regime_;
    
public:
    AdvancedPaperTradingBot(double initial_capital = 100000.0) 
        : initial_capital_(initial_capital), current_portfolio_value_(initial_capital),
          current_regime_(MarketRegime::SIDEWAYS_MARKET) {
        
        // Initialize recommended investment universe
        universe_ = {
            // Large Cap Growth (Core Holdings)
            "AAPL", "MSFT", "GOOGL", "AMZN", "TSLA", "NVDA", "META",
            
            // Diversified Sectors
            "JPM",   // Financial
            "JNJ",   // Healthcare
            "PG",    // Consumer Staples
            "HD",    // Consumer Discretionary
            "BA",    // Industrial
            "XOM",   // Energy
            
            // Growth & Innovation
            "PLTR", "SNOW", "CRWD", "ZS", "DDOG", "MDB",
            
            // ETFs for broad exposure
            "SPY", "QQQ", "IWM", "VTI", "ARKK", "ICLN",
            
            // Defensive/Value
            "BRK.B", "WMT", "KO", "PFE", "VZ", "T"
        };
        
        start_time_ = std::chrono::steady_clock::now();
        initializeTradingCore();
        initializeMLCore();
    }
    
    void initializeTradingCore() {
        // Create advanced trading configuration
        auto config = TradingCoreConfig::createPaperTradingConfig();
        
        // Alpaca API configuration
        config.trading_api_config.provider = "alpaca";
        config.trading_api_config.api_key = "YOUR_ALPACA_API_KEY";
        config.trading_api_config.secret_key = "YOUR_ALPACA_SECRET_KEY";
        config.trading_api_config.base_url = "https://paper-api.alpaca.markets";
        config.trading_api_config.paper_trading = true;
        config.trading_api_config.timeout_ms = 30000;
        
        // Market data configuration
        config.market_data_config.provider = "alpaca";
        config.market_data_config.enable_real_time = true;
        config.market_data_config.enable_historical = true;
        config.market_data_config.cache_size = 10000;
        
        // Risk management
        config.risk_params.max_position_size = params_.max_position_size;
        config.risk_params.daily_loss_limit = params_.daily_var_limit;
        config.risk_params.enable_stop_loss = true;
        config.risk_params.default_stop_loss = params_.stop_loss_pct;
        config.risk_params.enable_take_profit = true;
        config.risk_params.default_take_profit = params_.take_profit_pct;
        
        // Portfolio optimization
        config.optimization_params.method = OptimizationMethod::MEAN_VARIANCE;
        config.optimization_params.risk_aversion = 3.0;
        config.optimization_params.lookback_days = 252;
        
        // Rebalancing
        config.rebalancing_config.frequency = parsec::tradingcore::RebalancingFrequency::DAILY;
        config.rebalancing_config.threshold = 0.05; // 5% deviation threshold
        config.rebalancing_config.enable_tax_optimization = false;
        
        // Strategy configurations
        config.strategy_configs = createStrategyConfigs();
        
        // Enable ML features
        config.enable_ml_features = true;
        config.ml_config_path = "ml_config.json";
        
        trading_core_ = std::make_unique<TradingCore>(config);
        
        // Add symbols to track
        trading_core_->addSymbols(universe_);
        
        // Set up callbacks
        setupCallbacks();
    }
    
    void initializeMLCore() {
        ModelConfig ml_config;
        ml_config.model_type = "ensemble";
        ml_config.training_data_path = "market_data.csv";
        ml_config.validation_split = 0.2;
        ml_config.epochs = 100;
        ml_config.batch_size = 32;
        ml_config.learning_rate = 0.001;
        
        ml_core_ = std::make_shared<MLCore>(ml_config);
    }
    
    std::vector<StrategyConfig> createStrategyConfigs() {
        std::vector<StrategyConfig> strategies;
        
        // 1. Momentum Strategy (25% allocation)
        StrategyConfig momentum_strategy;
        momentum_strategy.name = "momentum_strategy";
        momentum_strategy.type = "MOMENTUM";
        momentum_strategy.weight = 0.25;
        momentum_strategy.symbols = {"AAPL", "MSFT", "GOOGL", "AMZN", "TSLA", "NVDA", "META"};
        momentum_strategy.parameters["lookback_period"] = params_.momentum_lookback;
        momentum_strategy.parameters["momentum_threshold"] = params_.momentum_threshold;
        momentum_strategy.parameters["rebalance_frequency"] = 1; // Daily
        strategies.push_back(momentum_strategy);
        
        // 2. Mean Reversion Strategy (20% allocation)
        StrategyConfig mean_reversion;
        mean_reversion.name = "mean_reversion_strategy";
        mean_reversion.type = "MEAN_REVERSION";
        mean_reversion.weight = 0.20;
        mean_reversion.symbols = {"SPY", "QQQ", "IWM", "VTI"};
        mean_reversion.parameters["bollinger_period"] = params_.bollinger_period;
        mean_reversion.parameters["bollinger_std_dev"] = params_.bollinger_std_dev;
        mean_reversion.parameters["oversold_threshold"] = -2.0;
        mean_reversion.parameters["overbought_threshold"] = 2.0;
        strategies.push_back(mean_reversion);
        
        // 3. ML Enhanced Strategy (30% allocation)
        StrategyConfig ml_strategy;
        ml_strategy.name = "ml_enhanced_strategy";
        ml_strategy.type = "ML_ENHANCED";
        ml_strategy.weight = 0.30;
        ml_strategy.symbols = {"PLTR", "SNOW", "CRWD", "ZS", "DDOG", "MDB", "ARKK"};
        ml_strategy.parameters["lookback_period"] = params_.ml_lookback;
        ml_strategy.parameters["confidence_threshold"] = params_.ml_confidence_threshold;
        ml_strategy.parameters["model_retrain_frequency"] = 7; // Weekly
        strategies.push_back(ml_strategy);
        
        // 4. Sector Rotation Strategy (15% allocation)
        StrategyConfig sector_rotation;
        sector_rotation.name = "sector_rotation_strategy";
        sector_rotation.type = "SECTOR_ROTATION";
        sector_rotation.weight = 0.15;
        sector_rotation.symbols = {"JPM", "JNJ", "PG", "HD", "BA", "XOM"};
        sector_rotation.parameters["rotation_period"] = 5; // 5 days
        sector_rotation.parameters["momentum_lookback"] = 60;
        sector_rotation.parameters["min_relative_strength"] = 1.05;
        strategies.push_back(sector_rotation);
        
        // 5. Defensive Strategy (10% allocation)
        StrategyConfig defensive;
        defensive.name = "defensive_strategy";
        defensive.type = "DEFENSIVE";
        defensive.weight = 0.10;
        defensive.symbols = {"BRK.B", "WMT", "KO", "PFE", "VZ", "T"};
        defensive.parameters["quality_threshold"] = 0.7;
        defensive.parameters["dividend_yield_min"] = 0.02;
        defensive.parameters["volatility_max"] = 0.25;
        strategies.push_back(defensive);
        
        return strategies;
    }
    
    void setupCallbacks() {
        // Market data callback
        trading_core_->setMarketDataCallback([this](const MarketDataPoint& data) {
            onMarketDataReceived(data);
        });
        
        // Signal callback
        trading_core_->setSignalCallback([this](const TradingSignal& signal) {
            onSignalGenerated(signal);
        });
        
        // Trade callback
        trading_core_->setTradeCallback([this](const Trade& trade) {
            onTradeExecuted(trade);
        });
        
        // Risk alert callback
        trading_core_->setRiskAlertCallback([this](const std::string& alert, double severity) {
            onRiskAlert(alert, severity);
        });
    }
    
    void onMarketDataReceived(const MarketDataPoint& data) {
        // Update market regime detection
        updateMarketRegime(data);
        
        // Log market data (optional)
        if (data.symbol == "SPY") {
            std::cout << "[MARKET] SPY: $" << std::fixed << std::setprecision(2) 
                      << data.price << " | Vol: " << data.volume 
                      << " | Regime: " << getRegimeString(current_regime_) << std::endl;
        }
    }
    
    void onSignalGenerated(const TradingSignal& signal) {
        std::cout << "[SIGNAL] " << signal.symbol << " | " 
                  << getSignalTypeString(signal.type) << " | Conf: " 
                  << std::fixed << std::setprecision(2) << signal.confidence 
                  << " | Price: $" << signal.price 
                  << " | Strategy: " << signal.strategy_name << std::endl;
    }
    
    void onTradeExecuted(const Trade& trade) {
        std::cout << "[TRADE] " << trade.symbol << " | " 
                  << (trade.side == OrderSide::BUY ? "BUY" : "SELL") 
                  << " | Qty: " << trade.quantity 
                  << " | Price: $" << std::fixed << std::setprecision(2) << trade.price 
                  << " | Commission: $" << trade.commission << std::endl;
        
        // Update portfolio tracking
        updatePortfolioMetrics();
    }
    
    void onRiskAlert(const std::string& alert, double severity) {
        std::cout << "[RISK ALERT] " << alert << " | Severity: " 
                  << std::fixed << std::setprecision(2) << severity << std::endl;
        
        // Implement risk management actions
        if (severity > 0.8) {
            handleHighRiskSituation(alert);
        }
    }
    
    void updateMarketRegime(const MarketDataPoint& data) {
        // Simplified market regime detection
        // In a real implementation, this would be more sophisticated
        static double prev_spy_price = 0.0;
        static std::vector<double> recent_returns;
        
        if (data.symbol == "SPY" && prev_spy_price > 0.0) {
            double return_pct = (data.price - prev_spy_price) / prev_spy_price;
            recent_returns.push_back(return_pct);
            
            if (recent_returns.size() > 20) {
                recent_returns.erase(recent_returns.begin());
            }
            
            if (recent_returns.size() >= 10) {
                double avg_return = std::accumulate(recent_returns.begin(), recent_returns.end(), 0.0) / recent_returns.size();
                double volatility = calculateVolatility(recent_returns);
                
                if (volatility > 0.03) {
                    current_regime_ = MarketRegime::HIGH_VOLATILITY;
                } else if (avg_return > 0.001) {
                    current_regime_ = MarketRegime::BULL_MARKET;
                } else if (avg_return < -0.001) {
                    current_regime_ = MarketRegime::BEAR_MARKET;
                } else {
                    current_regime_ = MarketRegime::SIDEWAYS_MARKET;
                }
            }
        }
        
        prev_spy_price = data.price;
    }
    
    void handleHighRiskSituation(const std::string& alert) {
        std::cout << "[RISK MANAGEMENT] Executing defensive measures..." << std::endl;
        
        // Reduce position sizes
        auto current_weights = trading_core_->getCurrentWeights();
        std::map<std::string, double> defensive_weights;
        
        for (const auto& [symbol, weight] : current_weights) {
            defensive_weights[symbol] = weight * 0.5; // Reduce by 50%
        }
        
        trading_core_->setTargetWeights(defensive_weights);
        
        // Increase cash allocation
        defensive_weights["CASH"] = 0.3; // 30% cash
        
        // Execute defensive rebalancing
        trading_core_->executeRebalance();
    }
    
    void updatePortfolioMetrics() {
        current_portfolio_value_ = trading_core_->getPortfolioValue();
        
        // Calculate daily return
        static double prev_portfolio_value = initial_capital_;
        double daily_return = (current_portfolio_value_ - prev_portfolio_value) / prev_portfolio_value;
        daily_returns_.push_back(daily_return);
        prev_portfolio_value = current_portfolio_value_;
        
        // Keep only last 252 days (1 year)
        if (daily_returns_.size() > 252) {
            daily_returns_.erase(daily_returns_.begin());
        }
    }
    
    double calculateVolatility(const std::vector<double>& returns) {
        if (returns.size() < 2) return 0.0;
        
        double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
        double variance = 0.0;
        
        for (double ret : returns) {
            variance += (ret - mean) * (ret - mean);
        }
        
        return std::sqrt(variance / (returns.size() - 1));
    }
    
    std::string getRegimeString(MarketRegime regime) {
        switch (regime) {
            case MarketRegime::BULL_MARKET: return "BULL";
            case MarketRegime::BEAR_MARKET: return "BEAR";
            case MarketRegime::SIDEWAYS_MARKET: return "SIDEWAYS";
            case MarketRegime::HIGH_VOLATILITY: return "HIGH_VOL";
            default: return "UNKNOWN";
        }
    }
    
    std::string getSignalTypeString(SignalType type) {
        switch (type) {
            case SignalType::BUY: return "BUY";
            case SignalType::SELL: return "SELL";
            case SignalType::HOLD: return "HOLD";
            case SignalType::STRONG_BUY: return "STRONG_BUY";
            case SignalType::STRONG_SELL: return "STRONG_SELL";
            default: return "UNKNOWN";
        }
    }
    
    void printDashboard() {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::hours>(now - start_time_);
        
        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "ADVANCED PAPER TRADING DASHBOARD" << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        
        std::cout << "Runtime: " << duration.count() << " hours" << std::endl;
        std::cout << "Initial Capital: $" << std::fixed << std::setprecision(2) << initial_capital_ << std::endl;
        std::cout << "Current Portfolio Value: $" << current_portfolio_value_ << std::endl;
        std::cout << "Total Return: " << std::setprecision(2) << ((current_portfolio_value_ - initial_capital_) / initial_capital_ * 100) << "%" << std::endl;
        
        if (!daily_returns_.empty()) {
            double volatility = calculateVolatility(daily_returns_) * std::sqrt(252); // Annualized
            double avg_return = std::accumulate(daily_returns_.begin(), daily_returns_.end(), 0.0) / daily_returns_.size() * 252;
            double sharpe_ratio = (avg_return - 0.02) / volatility; // Assuming 2% risk-free rate
            
            std::cout << "Annualized Return: " << std::setprecision(2) << (avg_return * 100) << "%" << std::endl;
            std::cout << "Annualized Volatility: " << (volatility * 100) << "%" << std::endl;
            std::cout << "Sharpe Ratio: " << sharpe_ratio << std::endl;
        }
        
        std::cout << "Market Regime: " << getRegimeString(current_regime_) << std::endl;
        
        // Portfolio allocation
        auto weights = trading_core_->getCurrentWeights();
        std::cout << "\nCurrent Allocation:" << std::endl;
        for (const auto& [symbol, weight] : weights) {
            if (weight > 0.01) { // Only show positions > 1%
                std::cout << "  " << symbol << ": " << std::setprecision(1) << (weight * 100) << "%" << std::endl;
            }
        }
        
        // Risk metrics
        auto risk_metrics = trading_core_->getRiskMetrics();
        std::cout << "\nRisk Metrics:" << std::endl;
        for (const auto& [metric, value] : risk_metrics) {
            std::cout << "  " << metric << ": " << std::setprecision(3) << value << std::endl;
        }
        
        std::cout << std::string(80, '=') << std::endl;
    }
    
    void exportPerformanceData(const std::string& filename) {
        std::ofstream file(filename);
        file << "Date,Portfolio_Value,Daily_Return,Cumulative_Return,Market_Regime\n";
        
        double cumulative_return = 0.0;
        for (size_t i = 0; i < daily_returns_.size(); ++i) {
            cumulative_return = (1.0 + cumulative_return) * (1.0 + daily_returns_[i]) - 1.0;
            file << i << "," << (initial_capital_ * (1.0 + cumulative_return)) << ","
                 << daily_returns_[i] << "," << cumulative_return << ","
                 << getRegimeString(current_regime_) << "\n";
        }
        
        file.close();
        std::cout << "Performance data exported to " << filename << std::endl;
    }
    
    void runTradingSession(int duration_hours = 24) {
        std::cout << "Starting Advanced Paper Trading Session..." << std::endl;
        std::cout << "Universe: " << universe_.size() << " symbols" << std::endl;
        std::cout << "Initial Capital: $" << initial_capital_ << std::endl;
        std::cout << "Duration: " << duration_hours << " hours" << std::endl;
        
        // Initialize trading core
        if (!trading_core_->initialize()) {
            std::cerr << "Failed to initialize trading core!" << std::endl;
            return;
        }
        
        // Start real-time data stream
        trading_core_->startRealTimeData([this](const MarketDataPoint& data) {
            onMarketDataReceived(data);
        });
        
        // Main trading loop
        auto end_time = std::chrono::steady_clock::now() + std::chrono::hours(duration_hours);
        int cycle_count = 0;
        
        while (std::chrono::steady_clock::now() < end_time) {
            try {
                // Generate and execute signals
                auto signals = trading_core_->generateSignals();
                if (!signals.empty()) {
                    trading_core_->executeSignals(signals);
                }
                
                // Check for rebalancing
                if (trading_core_->shouldRebalance()) {
                    std::cout << "[REBALANCE] Executing portfolio rebalancing..." << std::endl;
                    trading_core_->executeRebalance();
                }
                
                // Print dashboard every 100 cycles
                if (cycle_count % 100 == 0) {
                    printDashboard();
                }
                
                // Check risk limits
                if (!trading_core_->checkRiskLimits()) {
                    std::cout << "[RISK] Risk limits exceeded, implementing protective measures..." << std::endl;
                    handleHighRiskSituation("Risk limits exceeded");
                }
                
                cycle_count++;
                
                // Wait before next cycle
                std::this_thread::sleep_for(std::chrono::seconds(30));
                
            } catch (const std::exception& e) {
                std::cerr << "Error in trading loop: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(60));
            }
        }
        
        // Final dashboard and export
        printDashboard();
        exportPerformanceData("trading_performance.csv");
        
        std::cout << "\nTrading session completed!" << std::endl;
    }
};

int main() {
    std::cout << "Advanced Paper Trading Bot - StarSim Integration" << std::endl;
    std::cout << "=================================================" << std::endl;
    
    try {
        // Create bot with $100,000 initial capital
        AdvancedPaperTradingBot bot(100000.0);
        
        // Run for 72 hours (3 days)
        bot.runTradingSession(72);
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}