#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <vector>
#include <map>
#include <string>
#include <random>
#include <numeric>
#include <algorithm>

#include "tradingcore/TradingCore.h"
#include "mlcore/MLCore.h"
#include "parsec/ParsecCore.h"

using namespace parsec;
using namespace parsec::tradingcore;

class SimplePaperTradingBot {
private:
    std::unique_ptr<TradingCore> trading_core_;
    std::vector<std::string> universe_;
    
    // Performance tracking
    double initial_capital_;
    double current_portfolio_value_;
    std::vector<double> daily_returns_;
    std::chrono::steady_clock::time_point start_time_;
    
    // Simple market data simulation
    std::map<std::string, double> current_prices_;
    std::map<std::string, OHLCV> market_data_;
    
    // Random number generator for simulation
    std::mt19937 rng_;
    std::normal_distribution<double> price_change_dist_;
    
public:
    SimplePaperTradingBot(double initial_capital = 100000.0) 
        : initial_capital_(initial_capital), 
          current_portfolio_value_(initial_capital),
          rng_(std::random_device{}()),
          price_change_dist_(0.0, 0.02) // 2% daily volatility
    {
        // Recommended investment universe for 2024
        universe_ = {
            // AI/Technology Leaders
            "AAPL", "MSFT", "GOOGL", "AMZN", "NVDA", "META", "TSLA",
            
            // Growth Stocks
            "PLTR", "SNOW", "CRWD", "DDOG", "MDB",
            
            // Diversified Holdings
            "JPM", "JNJ", "PG", "HD", "BA", "XOM",
            
            // ETFs
            "SPY", "QQQ", "VTI", "ARKK"
        };
        
        start_time_ = std::chrono::steady_clock::now();
        initializeMarketData();
        initializeTradingCore();
    }
    
    void initializeMarketData() {
        // Initialize with realistic starting prices
        std::map<std::string, double> base_prices = {
            {"AAPL", 185.00}, {"MSFT", 420.00}, {"GOOGL", 140.00}, {"AMZN", 155.00},
            {"NVDA", 875.00}, {"META", 485.00}, {"TSLA", 240.00}, {"PLTR", 18.50},
            {"SNOW", 140.00}, {"CRWD", 285.00}, {"DDOG", 115.00}, {"MDB", 380.00},
            {"JPM", 180.00}, {"JNJ", 155.00}, {"PG", 160.00}, {"HD", 385.00},
            {"BA", 220.00}, {"XOM", 110.00}, {"SPY", 485.00}, {"QQQ", 415.00},
            {"VTI", 245.00}, {"ARKK", 48.00}
        };
        
        for (const auto& symbol : universe_) {
            double price = base_prices.count(symbol) ? base_prices[symbol] : 100.0;
            current_prices_[symbol] = price;
            
            // Create OHLCV data
            OHLCV ohlcv;
            ohlcv.open = price * 0.995;
            ohlcv.high = price * 1.01;
            ohlcv.low = price * 0.99;
            ohlcv.close = price;
            ohlcv.volume = 1000000;
            ohlcv.timestamp = std::chrono::system_clock::now();
            
            market_data_[symbol] = ohlcv;
        }
    }
    
    void initializeTradingCore() {
        // Create paper trading configuration
        auto config = TradingCoreConfig::createPaperTradingConfig();
        
        trading_core_ = std::make_unique<TradingCore>(config);
        
        // Add symbols to track
        for (const auto& symbol : universe_) {
            trading_core_->addSymbol(symbol);
        }
        
        std::cout << "TradingCore initialized with " << universe_.size() << " symbols" << std::endl;
    }
    
    void simulateMarketData() {
        // Simulate realistic market movements
        for (const auto& symbol : universe_) {
            double current_price = current_prices_[symbol];
            
            // Generate price change with some correlation to market (SPY)
            double market_change = 0.0;
            if (current_prices_.count("SPY")) {
                market_change = price_change_dist_(rng_) * 0.5; // 50% market correlation
            }
            
            double individual_change = price_change_dist_(rng_) * 0.5; // 50% individual
            double total_change = market_change + individual_change;
            
            // Apply some sector-specific logic
            if (symbol == "NVDA" || symbol == "TSLA") {
                total_change *= 1.5; // Higher volatility for growth stocks
            } else if (symbol == "JNJ" || symbol == "PG") {
                total_change *= 0.7; // Lower volatility for defensive stocks
            }
            
            double new_price = current_price * (1.0 + total_change);
            new_price = std::max(new_price, 1.0); // Minimum price floor
            
            current_prices_[symbol] = new_price;
            
            // Update OHLCV data
            OHLCV& ohlcv = market_data_[symbol];
            ohlcv.open = current_price;
            ohlcv.close = new_price;
            ohlcv.high = std::max(current_price, new_price) * (1.0 + std::abs(price_change_dist_(rng_)) * 0.1);
            ohlcv.low = std::min(current_price, new_price) * (1.0 - std::abs(price_change_dist_(rng_)) * 0.1);
            ohlcv.volume = static_cast<Volume>(1000000 * (1.0 + std::abs(total_change) * 5));
            ohlcv.timestamp = std::chrono::system_clock::now();
        }
    }
    
    std::vector<TradingSignal> generateSimpleSignals() {
        std::vector<TradingSignal> signals;
        
        for (const auto& symbol : universe_) {
            TradingSignal signal;
            signal.symbol = symbol;
            signal.price = current_prices_[symbol];
            signal.timestamp = std::chrono::system_clock::now();
            signal.strategy_name = "SimpleBot";
            
            // Simple momentum strategy
            static std::map<std::string, std::vector<double>> price_history;
            price_history[symbol].push_back(current_prices_[symbol]);
            
            if (price_history[symbol].size() > 10) {
                price_history[symbol].erase(price_history[symbol].begin());
            }
            
            if (price_history[symbol].size() >= 5) {
                double recent_avg = std::accumulate(price_history[symbol].end() - 3, price_history[symbol].end(), 0.0) / 3.0;
                double older_avg = std::accumulate(price_history[symbol].begin(), price_history[symbol].begin() + 3, 0.0) / 3.0;
                
                double momentum = (recent_avg - older_avg) / older_avg;
                
                if (momentum > 0.02) { // 2% momentum threshold
                    signal.type = SignalType::BUY;
                    signal.confidence = std::min(0.9, std::abs(momentum) * 10);
                } else if (momentum < -0.02) {
                    signal.type = SignalType::SELL;
                    signal.confidence = std::min(0.9, std::abs(momentum) * 10);
                } else {
                    signal.type = SignalType::HOLD;
                    signal.confidence = 0.5;
                }
                
                // Add some randomness to make it more realistic
                if (std::uniform_real_distribution<double>(0.0, 1.0)(rng_) > 0.7) {
                    signals.push_back(signal);
                }
            }
        }
        
        return signals;
    }
    
    void executeSignals(const std::vector<TradingSignal>& signals) {
        for (const auto& signal : signals) {
            if (signal.confidence > 0.6) { // Only execute high confidence signals
                
                // Create order
                Order order;
                order.symbol = signal.symbol;
                order.type = OrderType::MARKET;
                order.price = signal.price;
                order.created_at = std::chrono::system_clock::now();
                order.status = OrderStatus::PENDING;
                
                // Simple position sizing (2% of portfolio per trade)
                double position_value = current_portfolio_value_ * 0.02;
                order.quantity = position_value / signal.price;
                
                if (signal.type == SignalType::BUY) {
                    order.side = OrderSide::BUY;
                } else if (signal.type == SignalType::SELL) {
                    order.side = OrderSide::SELL;
                }
                
                // Simulate order execution
                simulateOrderExecution(order);
            }
        }
    }
    
    void simulateOrderExecution(const Order& order) {
        // Simulate order execution with realistic slippage
        double slippage_factor = 1.0 + (std::uniform_real_distribution<double>(-0.001, 0.001)(rng_));
        double execution_price = order.price * slippage_factor;
        
        // Create trade
        Trade trade;
        trade.symbol = order.symbol;
        trade.side = order.side;
        trade.quantity = order.quantity;
        trade.price = execution_price;
        trade.timestamp = std::chrono::system_clock::now();
        trade.commission = std::max(1.0, order.quantity * execution_price * 0.001); // 0.1% commission
        
        // Update portfolio value
        if (order.side == OrderSide::BUY) {
            current_portfolio_value_ -= (trade.quantity * trade.price + trade.commission);
        } else {
            current_portfolio_value_ += (trade.quantity * trade.price - trade.commission);
        }
        
        // Log trade
        std::cout << "[TRADE] " << trade.symbol << " | " 
                  << (trade.side == OrderSide::BUY ? "BUY" : "SELL") 
                  << " | Qty: " << std::fixed << std::setprecision(0) << trade.quantity 
                  << " | Price: $" << std::setprecision(2) << trade.price 
                  << " | Commission: $" << trade.commission 
                  << " | Portfolio: $" << current_portfolio_value_ << std::endl;
    }
    
    void printDashboard() {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::minutes>(now - start_time_);
        
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "SIMPLE PAPER TRADING DASHBOARD" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        std::cout << "Runtime: " << duration.count() << " minutes" << std::endl;
        std::cout << "Initial Capital: $" << std::fixed << std::setprecision(2) << initial_capital_ << std::endl;
        std::cout << "Current Portfolio Value: $" << current_portfolio_value_ << std::endl;
        
        double total_return = (current_portfolio_value_ - initial_capital_) / initial_capital_ * 100;
        std::cout << "Total Return: " << std::setprecision(2) << total_return << "%" << std::endl;
        
        // Show current prices for key stocks
        std::cout << "\nKey Stock Prices:" << std::endl;
        std::vector<std::string> key_stocks = {"AAPL", "MSFT", "GOOGL", "NVDA", "TSLA", "SPY"};
        for (const auto& symbol : key_stocks) {
            if (current_prices_.count(symbol)) {
                std::cout << "  " << symbol << ": $" << std::fixed << std::setprecision(2) 
                          << current_prices_[symbol] << std::endl;
            }
        }
        
        std::cout << std::string(60, '=') << std::endl;
    }
    
    void exportResults(const std::string& filename) {
        std::ofstream file(filename);
        file << "Symbol,Current_Price,Daily_Change\n";
        
        for (const auto& [symbol, price] : current_prices_) {
            file << symbol << "," << price << ",0.0\n"; // Would calculate actual changes in real implementation
        }
        
        file.close();
        std::cout << "Results exported to " << filename << std::endl;
    }
    
    void runTradingSession(int duration_minutes = 60) {
        std::cout << "Starting Simple Paper Trading Session..." << std::endl;
        std::cout << "Universe: " << universe_.size() << " symbols" << std::endl;
        std::cout << "Initial Capital: $" << std::fixed << std::setprecision(2) << initial_capital_ << std::endl;
        std::cout << "Duration: " << duration_minutes << " minutes" << std::endl;
        std::cout << std::string(60, '-') << std::endl;
        
        auto end_time = std::chrono::steady_clock::now() + std::chrono::minutes(duration_minutes);
        int cycle_count = 0;
        
        while (std::chrono::steady_clock::now() < end_time) {
            try {
                // Simulate market data updates
                simulateMarketData();
                
                // Generate trading signals
                auto signals = generateSimpleSignals();
                
                // Execute signals
                if (!signals.empty()) {
                    std::cout << "[CYCLE " << cycle_count << "] Generated " << signals.size() << " signals" << std::endl;
                    executeSignals(signals);
                }
                
                // Print dashboard every 10 cycles
                if (cycle_count % 10 == 0) {
                    printDashboard();
                }
                
                cycle_count++;
                
                // Wait before next cycle (simulate real-time trading)
                std::this_thread::sleep_for(std::chrono::seconds(5));
                
            } catch (const std::exception& e) {
                std::cerr << "Error in trading loop: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(10));
            }
        }
        
        // Final dashboard and export
        printDashboard();
        exportResults("trading_results.csv");
        
        std::cout << "\nTrading session completed!" << std::endl;
        
        // Calculate final performance metrics
        double total_return = (current_portfolio_value_ - initial_capital_) / initial_capital_ * 100;
        if (total_return > 0) {
            std::cout << "🎉 Profitable session! Total return: " << std::fixed << std::setprecision(2) << total_return << "%" << std::endl;
        } else {
            std::cout << "📉 Loss in this session. Total return: " << std::fixed << std::setprecision(2) << total_return << "%" << std::endl;
        }
    }
};

// Investment recommendations for 2024
void printInvestmentRecommendations() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "INVESTMENT RECOMMENDATIONS FOR 2024" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    
    std::cout << "\n🚀 TOP AI/TECHNOLOGY PICKS:" << std::endl;
    std::cout << "• NVDA (NVIDIA) - AI chip leader, strong growth potential" << std::endl;
    std::cout << "• MSFT (Microsoft) - Cloud computing + AI integration" << std::endl;
    std::cout << "• GOOGL (Alphabet) - Undervalued AI leader with search dominance" << std::endl;
    std::cout << "• PLTR (Palantir) - Government AI contracts and data analytics" << std::endl;
    
    std::cout << "\n💼 DIVERSIFIED CORE HOLDINGS:" << std::endl;
    std::cout << "• AAPL (Apple) - Stable ecosystem, AI hardware potential" << std::endl;
    std::cout << "• AMZN (Amazon) - E-commerce + AWS cloud dominance" << std::endl;
    std::cout << "• JPM (JPMorgan) - Financial sector leader" << std::endl;
    std::cout << "• JNJ (Johnson & Johnson) - Healthcare stability" << std::endl;
    
    std::cout << "\n🔒 DEFENSIVE POSITIONS:" << std::endl;
    std::cout << "• SPY (S&P 500 ETF) - Broad market exposure" << std::endl;
    std::cout << "• VTI (Total Stock Market) - Diversified US exposure" << std::endl;
    std::cout << "• PG (Procter & Gamble) - Consumer staples stability" << std::endl;
    
    std::cout << "\n⚡ HIGH-GROWTH OPPORTUNITIES:" << std::endl;
    std::cout << "• TSLA (Tesla) - Electric vehicle + energy storage" << std::endl;
    std::cout << "• SNOW (Snowflake) - Cloud data platform growth" << std::endl;
    std::cout << "• CRWD (CrowdStrike) - Cybersecurity expansion" << std::endl;
    std::cout << "• ARKK (ARK Innovation ETF) - Disruptive technology exposure" << std::endl;
    
    std::cout << "\n📊 RECOMMENDED ALLOCATION:" << std::endl;
    std::cout << "• 40% Large Cap Tech (AAPL, MSFT, GOOGL, AMZN)" << std::endl;
    std::cout << "• 25% Growth Stocks (NVDA, TSLA, PLTR, AI stocks)" << std::endl;
    std::cout << "• 20% Diversified Sectors (JPM, JNJ, PG, HD)" << std::endl;
    std::cout << "• 10% ETFs (SPY, QQQ, VTI)" << std::endl;
    std::cout << "• 5% Cash for opportunities" << std::endl;
    
    std::cout << "\n⚠️ RISK CONSIDERATIONS:" << std::endl;
    std::cout << "• Interest rate sensitivity for growth stocks" << std::endl;
    std::cout << "• Regulatory risks in tech sector" << std::endl;
    std::cout << "• Market volatility in AI/growth names" << std::endl;
    std::cout << "• Geopolitical tensions affecting tech" << std::endl;
    
    std::cout << "\n💡 TRADING STRATEGY:" << std::endl;
    std::cout << "• Use momentum signals for entry/exit" << std::endl;
    std::cout << "• Implement stop-losses at 5-7% below entry" << std::endl;
    std::cout << "• Take profits at 15-20% gains" << std::endl;
    std::cout << "• Position size: 2-5% of portfolio per trade" << std::endl;
    std::cout << "• Rebalance monthly or on significant moves" << std::endl;
    
    std::cout << std::string(70, '=') << std::endl;
}

int main() {
    std::cout << "Simple Paper Trading Bot - StarSim Integration" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // Print investment recommendations
    printInvestmentRecommendations();
    
    std::cout << "\nStarting paper trading demonstration..." << std::endl;
    
    try {
        // Create bot with $100,000 initial capital
        SimplePaperTradingBot bot(100000.0);
        
        // Run for 30 minutes (adjust for longer demo)
        bot.runTradingSession(30);
        
        std::cout << "\n🎯 Next Steps:" << std::endl;
        std::cout << "1. Set up real Alpaca API credentials" << std::endl;
        std::cout << "2. Implement more sophisticated strategies" << std::endl;
        std::cout << "3. Add ML-based signal generation" << std::endl;
        std::cout << "4. Implement proper risk management" << std::endl;
        std::cout << "5. Run extended backtests" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}