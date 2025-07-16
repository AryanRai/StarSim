#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include <random>

#include "tradingcore/TradingCore.h"

using namespace parsec::tradingcore;

class QuickTradingDemo {
private:
    std::map<std::string, double> prices_;
    std::map<std::string, std::vector<double>> history_;
    std::map<std::string, double> positions_;
    double cash_ = 100000.0;
    double portfolio_value_ = 100000.0;
    std::mt19937 rng_;
    int cycle_count_ = 0;
    
public:
    QuickTradingDemo() : rng_(std::random_device{}()) {
        // Initialize with 2024 recommended stocks
        prices_ = {
            {"NVDA", 875.00}, {"AAPL", 185.00}, {"MSFT", 420.00}, 
            {"GOOGL", 140.00}, {"PLTR", 18.50}, {"TSLA", 240.00},
            {"SPY", 485.00}, {"QQQ", 415.00}
        };
        
        // Pre-populate history to enable immediate signals
        for (auto& [symbol, price] : prices_) {
            // Create 25 historical points with slight trend
            for (int i = 0; i < 25; ++i) {
                double trend_factor = 1.0 + (i * 0.002); // Slight upward trend
                double noise = std::normal_distribution<double>(0.0, 0.01)(rng_);
                history_[symbol].push_back(price * trend_factor * (1.0 + noise));
            }
        }
    }
    
    void updatePrices() {
        std::normal_distribution<double> change_dist(0.001, 0.02); // Slight upward bias
        
        for (auto& [symbol, price] : prices_) {
            double change = change_dist(rng_);
            
            // Add volatility for growth stocks
            if (symbol == "NVDA" || symbol == "PLTR" || symbol == "TSLA") {
                change *= 1.5;
            }
            
            price *= (1.0 + change);
            price = std::max(price, 1.0);
            
            history_[symbol].push_back(price);
            if (history_[symbol].size() > 30) {
                history_[symbol].erase(history_[symbol].begin());
            }
        }
    }
    
    double calculateSMA(const std::vector<double>& data, int period) {
        if (data.size() < period) return 0.0;
        
        double sum = 0.0;
        for (int i = data.size() - period; i < data.size(); ++i) {
            sum += data[i];
        }
        return sum / period;
    }
    
    std::vector<TradingSignal> generateSignals() {
        std::vector<TradingSignal> signals;
        
        for (const auto& [symbol, price] : prices_) {
            const auto& hist = history_[symbol];
            if (hist.size() < 20) continue;
            
            double sma5 = calculateSMA(hist, 5);
            double sma20 = calculateSMA(hist, 20);
            
            if (sma5 == 0.0 || sma20 == 0.0) continue;
            
            TradingSignal signal;
            signal.symbol = symbol;
            signal.price = price;
            signal.timestamp = std::chrono::system_clock::now();
            signal.strategy_name = "QuickMomentum";
            signal.type = SignalType::HOLD;
            signal.confidence = 0.0;
            
            // Generate signals with lower threshold for demo
            if (sma5 > sma20 * 1.01) { // 1% threshold instead of 2%
                signal.type = SignalType::BUY;
                signal.confidence = 0.75;
                
                // Boost AI stocks
                if (symbol == "NVDA" || symbol == "PLTR") {
                    signal.confidence = 0.85;
                }
            } else if (sma5 < sma20 * 0.99) {
                signal.type = SignalType::SELL;
                signal.confidence = 0.70;
            }
            
            if (signal.type != SignalType::HOLD) {
                signals.push_back(signal);
            }
        }
        
        return signals;
    }
    
    void executeSignals(const std::vector<TradingSignal>& signals) {
        for (const auto& signal : signals) {
            double position_size = portfolio_value_ * 0.05; // 5% position size
            double quantity = position_size / signal.price;
            
            if (quantity < 1.0) continue;
            
            bool has_position = positions_[signal.symbol] > 0;
            
            if (signal.type == SignalType::BUY && !has_position && cash_ >= position_size) {
                positions_[signal.symbol] = quantity;
                cash_ -= quantity * signal.price;
                
                std::cout << "🟢 BUY EXECUTED: " << signal.symbol 
                          << " | Qty: " << std::fixed << std::setprecision(0) << quantity
                          << " | Price: $" << std::setprecision(2) << signal.price
                          << " | Confidence: " << signal.confidence << std::endl;
                          
            } else if (signal.type == SignalType::SELL && has_position) {
                double sell_qty = std::min(quantity, positions_[signal.symbol]);
                positions_[signal.symbol] -= sell_qty;
                cash_ += sell_qty * signal.price;
                
                std::cout << "🔴 SELL EXECUTED: " << signal.symbol 
                          << " | Qty: " << std::fixed << std::setprecision(0) << sell_qty
                          << " | Price: $" << std::setprecision(2) << signal.price
                          << " | Confidence: " << signal.confidence << std::endl;
            }
        }
    }
    
    void updatePortfolio() {
        double total_value = cash_;
        for (const auto& [symbol, qty] : positions_) {
            if (qty > 0) {
                total_value += qty * prices_[symbol];
            }
        }
        portfolio_value_ = total_value;
    }
    
    void printDashboard() {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "🎯 QUICK TRADING DEMO - CYCLE " << cycle_count_ << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        std::cout << "💰 Portfolio: $" << std::fixed << std::setprecision(2) << portfolio_value_ << std::endl;
        std::cout << "💵 Cash: $" << cash_ << std::endl;
        
        double return_pct = ((portfolio_value_ - 100000.0) / 100000.0) * 100;
        std::cout << "📈 Return: " << std::setprecision(2) << return_pct << "%" << std::endl;
        
        std::cout << "\n📊 Active Positions:" << std::endl;
        for (const auto& [symbol, qty] : positions_) {
            if (qty > 0) {
                double value = qty * prices_[symbol];
                double weight = (value / portfolio_value_) * 100;
                std::cout << "  " << symbol << ": " << std::setprecision(0) << qty 
                          << " @ $" << std::setprecision(2) << prices_[symbol]
                          << " = $" << value << " (" << weight << "%)" << std::endl;
            }
        }
        
        std::cout << "\n📈 Current Prices:" << std::endl;
        for (const auto& [symbol, price] : prices_) {
            const auto& hist = history_[symbol];
            double sma5 = calculateSMA(hist, 5);
            double sma20 = calculateSMA(hist, 20);
            std::string trend = (sma5 > sma20) ? "📈" : "📉";
            
            std::cout << "  " << symbol << ": $" << std::fixed << std::setprecision(2) 
                      << price << " " << trend << std::endl;
        }
        
        std::cout << std::string(60, '=') << std::endl;
    }
    
    void runQuickDemo() {
        std::cout << "🚀 Quick Trading Demo - Immediate Action!" << std::endl;
        std::cout << "📊 2024 Investment Universe: AI/Tech Focus" << std::endl;
        std::cout << "⚡ Accelerated cycles for instant results\n" << std::endl;
        
        for (int i = 0; i < 15; ++i) { // 15 quick cycles
            cycle_count_ = i;
            
            std::cout << "\n[CYCLE " << i << "] Processing..." << std::endl;
            
            // Update market data
            updatePrices();
            
            // Generate signals
            auto signals = generateSignals();
            std::cout << "Generated " << signals.size() << " signals" << std::endl;
            
            // Execute trades
            if (!signals.empty()) {
                executeSignals(signals);
            }
            
            // Update portfolio
            updatePortfolio();
            
            // Show dashboard every 3 cycles
            if (i % 3 == 0) {
                printDashboard();
            }
            
            // Quick pause for dramatic effect
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        
        // Final results
        std::cout << "\n🎉 DEMO COMPLETE!" << std::endl;
        printDashboard();
        
        std::cout << "\n🎯 Key Takeaways:" << std::endl;
        std::cout << "✅ Momentum strategy successfully generated signals" << std::endl;
        std::cout << "✅ Automatic position sizing and risk management" << std::endl;
        std::cout << "✅ Real-time portfolio tracking and updates" << std::endl;
        std::cout << "✅ Focus on 2024 AI/technology recommendations" << std::endl;
        
        double final_return = ((portfolio_value_ - 100000.0) / 100000.0) * 100;
        if (final_return > 0) {
            std::cout << "🚀 Profitable demo session: +" << std::fixed << std::setprecision(2) 
                      << final_return << "%" << std::endl;
        } else {
            std::cout << "📉 Demo loss: " << std::fixed << std::setprecision(2) 
                      << final_return << "% (normal in short timeframes)" << std::endl;
        }
        
        std::cout << "\nNext: Set up real Alpaca API for live paper trading!" << std::endl;
    }
};

int main() {
    try {
        QuickTradingDemo demo;
        demo.runQuickDemo();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}