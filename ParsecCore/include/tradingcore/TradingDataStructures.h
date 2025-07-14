#pragma once

#include <vector>
#include <string>
#include <map>
#include <chrono>
#include <functional>

namespace parsec {
namespace tradingcore {

// Core data types
using Timestamp = std::chrono::system_clock::time_point;
using Price = double;
using Volume = double;
using Quantity = double;

// Market data structures
struct OHLCV {
    Price open = 0.0;
    Price high = 0.0;
    Price low = 0.0;
    Price close = 0.0;
    Volume volume = 0.0;
    Timestamp timestamp;
    
    OHLCV() = default;
    OHLCV(Price o, Price h, Price l, Price c, Volume v, Timestamp t)
        : open(o), high(h), low(l), close(c), volume(v), timestamp(t) {}
};

struct MarketDataPoint {
    std::string symbol;
    OHLCV data;
    std::map<std::string, double> additional_fields; // For custom indicators
    
    MarketDataPoint() = default;
    MarketDataPoint(const std::string& sym, const OHLCV& ohlcv)
        : symbol(sym), data(ohlcv) {}
};

struct HistoricalData {
    std::string symbol;
    std::string timeframe; // "1min", "5min", "1hour", "1day"
    std::vector<OHLCV> bars;
    
    // Utility methods
    std::vector<Price> getCloses() const;
    std::vector<Price> getOpens() const;
    std::vector<Price> getHighs() const;
    std::vector<Price> getLows() const;
    std::vector<Volume> getVolumes() const;
    size_t size() const { return bars.size(); }
    bool empty() const { return bars.empty(); }
};

// Trading signal structures
enum class SignalType {
    BUY,
    SELL,
    HOLD,
    STRONG_BUY,
    STRONG_SELL
};

struct TradingSignal {
    std::string symbol;
    SignalType type;
    Price price;
    double confidence; // 0.0 to 1.0
    Timestamp timestamp;
    std::string strategy_name;
    std::map<std::string, double> metadata; // Additional signal info
    
    TradingSignal() : type(SignalType::HOLD), price(0.0), confidence(0.0) {}
    TradingSignal(const std::string& sym, SignalType t, Price p, double conf, Timestamp ts, const std::string& strategy)
        : symbol(sym), type(t), price(p), confidence(conf), timestamp(ts), strategy_name(strategy) {}
};

// Order and trade structures
enum class OrderType {
    MARKET,
    LIMIT,
    STOP,
    STOP_LIMIT
};

enum class OrderSide {
    BUY,
    SELL
};

enum class OrderStatus {
    PENDING,
    FILLED,
    PARTIALLY_FILLED,
    CANCELLED,
    REJECTED
};

struct Order {
    std::string id;
    std::string symbol;
    OrderType type;
    OrderSide side;
    Quantity quantity;
    Price price; // For limit orders
    Price stop_price; // For stop orders
    OrderStatus status;
    Timestamp created_at;
    Timestamp filled_at;
    Quantity filled_quantity;
    Price average_fill_price;
    
    Order() : type(OrderType::MARKET), side(OrderSide::BUY), quantity(0.0), 
              price(0.0), stop_price(0.0), status(OrderStatus::PENDING), 
              filled_quantity(0.0), average_fill_price(0.0) {}
};

struct Trade {
    std::string id;
    std::string symbol;
    OrderSide side;
    Quantity quantity;
    Price price;
    Timestamp timestamp;
    double commission;
    std::string strategy_name;
    
    Trade() : side(OrderSide::BUY), quantity(0.0), price(0.0), commission(0.0) {}
    
    double getNotionalValue() const { return quantity * price; }
    double getNetValue() const { return getNotionalValue() - commission; }
};

// Portfolio structures
struct Position {
    std::string symbol;
    Quantity quantity;
    Price average_cost;
    Price current_price;
    double unrealized_pnl;
    double realized_pnl;
    Timestamp last_updated;
    
    Position() : quantity(0.0), average_cost(0.0), current_price(0.0), 
                 unrealized_pnl(0.0), realized_pnl(0.0) {}
    
    double getMarketValue() const { return quantity * current_price; }
    double getTotalPnL() const { return unrealized_pnl + realized_pnl; }
    bool isLong() const { return quantity > 0; }
    bool isShort() const { return quantity < 0; }
    bool isEmpty() const { return quantity == 0; }
};

struct Portfolio {
    double cash;
    double total_value;
    std::map<std::string, Position> positions;
    std::vector<Trade> trade_history;
    Timestamp last_updated;
    
    Portfolio() : cash(0.0), total_value(0.0) {}
    
    double getEquityValue() const;
    double getTotalPnL() const;
    std::vector<std::string> getHoldings() const;
    Position* getPosition(const std::string& symbol);
    const Position* getPosition(const std::string& symbol) const;
};

// Strategy configuration
struct StrategyConfig {
    std::string name;
    std::string type; // "moving_average", "rsi_mean_reversion", "portfolio_optimization"
    std::map<std::string, double> parameters;
    std::vector<std::string> symbols;
    bool enabled;
    
    StrategyConfig() : enabled(true) {}
    
    // Helper methods for common parameters
    double getParameter(const std::string& key, double default_val = 0.0) const;
    void setParameter(const std::string& key, double value);
    bool hasParameter(const std::string& key) const;
};

// Risk management structures
struct RiskParams {
    double max_position_size; // As fraction of portfolio
    double max_daily_loss; // As fraction of portfolio
    double max_drawdown; // As fraction of portfolio
    double stop_loss_pct; // Stop loss percentage
    double take_profit_pct; // Take profit percentage
    bool use_atr_sizing; // Use ATR for position sizing
    double atr_multiplier; // ATR multiplier for stop loss
    
    RiskParams() : max_position_size(0.1), max_daily_loss(0.02), max_drawdown(0.1),
                   stop_loss_pct(0.05), take_profit_pct(0.10), use_atr_sizing(false),
                   atr_multiplier(2.0) {}
};

// Performance metrics
struct PerformanceMetrics {
    double total_return;
    double annual_return;
    double sharpe_ratio;
    double max_drawdown;
    double win_rate;
    double avg_win;
    double avg_loss;
    double profit_factor;
    int total_trades;
    int winning_trades;
    int losing_trades;
    
    PerformanceMetrics() : total_return(0.0), annual_return(0.0), sharpe_ratio(0.0),
                          max_drawdown(0.0), win_rate(0.0), avg_win(0.0), avg_loss(0.0),
                          profit_factor(0.0), total_trades(0), winning_trades(0), losing_trades(0) {}
};

// API configuration
struct APIConfig {
    std::string provider; // "alpaca", "oanda", "interactive_brokers"
    std::string api_key;
    std::string secret_key;
    std::string base_url;
    bool paper_trading;
    int timeout_ms;
    int rate_limit_requests_per_minute;
    
    APIConfig() : paper_trading(true), timeout_ms(30000), rate_limit_requests_per_minute(200) {}
};

// Market data configuration
struct MarketDataConfig {
    std::vector<std::string> symbols;
    std::string timeframe;
    bool real_time;
    std::string data_provider; // "alpaca", "yahoo", "alpha_vantage"
    int max_bars; // Maximum historical bars to fetch
    
    MarketDataConfig() : timeframe("1min"), real_time(false), max_bars(1000) {}
};

// Callback function types
using MarketDataCallback = std::function<void(const MarketDataPoint&)>;
using SignalCallback = std::function<void(const TradingSignal&)>;
using OrderCallback = std::function<void(const Order&)>;
using TradeCallback = std::function<void(const Trade&)>;
using ErrorCallback = std::function<void(const std::string&, const std::string&)>;

// Utility functions
namespace TradingUtils {
    // Time utilities
    Timestamp parseTimestamp(const std::string& iso_string);
    std::string formatTimestamp(const Timestamp& ts);
    
    // Price utilities
    Price roundToTick(Price price, Price tick_size);
    bool isValidPrice(Price price);
    
    // Signal utilities
    std::string signalTypeToString(SignalType type);
    SignalType stringToSignalType(const std::string& str);
    
    // Order utilities
    std::string orderTypeToString(OrderType type);
    std::string orderSideToString(OrderSide side);
    std::string orderStatusToString(OrderStatus status);
    
    // Risk utilities
    Quantity calculatePositionSize(double portfolio_value, Price entry_price, 
                                 double risk_per_trade, Price stop_loss);
    double calculateRiskReward(Price entry, Price stop_loss, Price take_profit);
}

} // namespace tradingcore
} // namespace parsec