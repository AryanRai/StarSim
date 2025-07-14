#pragma once

#include "tradingcore/TradingDataStructures.h"
#include "tradingcore/modules/MarketData.h"
#include <memory>
#include <queue>
#include <mutex>
#include <future>
#include <atomic>

namespace parsec {
namespace tradingcore {

// Forward declarations
class BrokerageAPI;

// Order execution configuration
struct ExecutionConfig {
    double slippage_bps = 5.0;          // Expected slippage in basis points
    double max_market_impact = 0.01;    // Maximum market impact (1%)
    int max_order_size = 1000;          // Maximum single order size
    int order_timeout_seconds = 300;    // Order timeout (5 minutes)
    bool enable_smart_routing = true;   // Use smart order routing
    bool split_large_orders = true;     // Split large orders into smaller chunks
    
    // Risk controls
    double max_position_size = 0.1;     // Maximum position as % of portfolio
    double daily_loss_limit = 0.02;    // Daily loss limit (2%)
    bool enable_position_limits = true; // Enable position size limits
    bool enable_order_validation = true; // Validate orders before submission
    
    ExecutionConfig() = default;
};

// Order execution result
struct ExecutionResult {
    std::string order_id;
    OrderStatus status;
    std::string error_message;
    Quantity filled_quantity;
    Price average_fill_price;
    double total_commission;
    Timestamp execution_time;
    std::vector<Trade> fills;
    
    ExecutionResult() : status(OrderStatus::PENDING), filled_quantity(0.0), 
                       average_fill_price(0.0), total_commission(0.0) {}
    
    bool isSuccessful() const { return status == OrderStatus::FILLED; }
    bool isPartiallyFilled() const { return status == OrderStatus::PARTIALLY_FILLED; }
    bool hasFailed() const { return status == OrderStatus::REJECTED || status == OrderStatus::CANCELLED; }
};

// Base class for brokerage API implementations
class BrokerageAPI {
public:
    virtual ~BrokerageAPI() = default;
    
    // Account information
    virtual Portfolio getAccountInfo() = 0;
    virtual double getAccountBalance() = 0;
    virtual std::map<std::string, Position> getPositions() = 0;
    virtual std::vector<Order> getOpenOrders() = 0;
    
    // Order management
    virtual std::string submitOrder(const Order& order) = 0;
    virtual bool cancelOrder(const std::string& order_id) = 0;
    virtual bool modifyOrder(const std::string& order_id, const Order& modified_order) = 0;
    virtual Order getOrderStatus(const std::string& order_id) = 0;
    
    // Trade history
    virtual std::vector<Trade> getTradeHistory(const Timestamp& start_date = {},
                                             const Timestamp& end_date = {}) = 0;
    
    // Market data (basic)
    virtual MarketDataPoint getQuote(const std::string& symbol) = 0;
    
    // Connection and authentication
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual std::string getProviderName() const = 0;
    
    // Configuration
    virtual void setConfig(const APIConfig& config) = 0;
    virtual APIConfig getConfig() const = 0;
};

// Alpaca API implementation
class AlpacaAPI : public BrokerageAPI {
public:
    explicit AlpacaAPI(const APIConfig& config);
    ~AlpacaAPI() override;
    
    // BrokerageAPI interface
    Portfolio getAccountInfo() override;
    double getAccountBalance() override;
    std::map<std::string, Position> getPositions() override;
    std::vector<Order> getOpenOrders() override;
    
    std::string submitOrder(const Order& order) override;
    bool cancelOrder(const std::string& order_id) override;
    bool modifyOrder(const std::string& order_id, const Order& modified_order) override;
    Order getOrderStatus(const std::string& order_id) override;
    
    std::vector<Trade> getTradeHistory(const Timestamp& start_date = {},
                                     const Timestamp& end_date = {}) override;
    
    MarketDataPoint getQuote(const std::string& symbol) override;
    
    bool connect() override;
    void disconnect() override;
    bool isConnected() const override { return connected_; }
    std::string getProviderName() const override { return "Alpaca"; }
    
    void setConfig(const APIConfig& config) override { config_ = config; }
    APIConfig getConfig() const override { return config_; }
    
private:
    APIConfig config_;
    bool connected_;
    std::unique_ptr<class HTTPClient> http_client_;
    
    // Helper methods
    std::string makeRequest(const std::string& endpoint, const std::string& method,
                          const std::string& data = "");
    Order parseOrderResponse(const std::string& json_response);
    Portfolio parseAccountResponse(const std::string& json_response);
    std::vector<Trade> parseTradesResponse(const std::string& json_response);
    std::string orderToJson(const Order& order);
    bool validateOrder(const Order& order);
};

// OANDA API implementation
class OandaAPI : public BrokerageAPI {
public:
    explicit OandaAPI(const APIConfig& config);
    ~OandaAPI() override;
    
    // BrokerageAPI interface
    Portfolio getAccountInfo() override;
    double getAccountBalance() override;
    std::map<std::string, Position> getPositions() override;
    std::vector<Order> getOpenOrders() override;
    
    std::string submitOrder(const Order& order) override;
    bool cancelOrder(const std::string& order_id) override;
    bool modifyOrder(const std::string& order_id, const Order& modified_order) override;
    Order getOrderStatus(const std::string& order_id) override;
    
    std::vector<Trade> getTradeHistory(const Timestamp& start_date = {},
                                     const Timestamp& end_date = {}) override;
    
    MarketDataPoint getQuote(const std::string& symbol) override;
    
    bool connect() override;
    void disconnect() override;
    bool isConnected() const override { return connected_; }
    std::string getProviderName() const override { return "OANDA"; }
    
    void setConfig(const APIConfig& config) override { config_ = config; }
    APIConfig getConfig() const override { return config_; }
    
private:
    APIConfig config_;
    bool connected_;
    std::string account_id_;
    std::unique_ptr<class HTTPClient> http_client_;
    
    // Helper methods
    std::string makeRequest(const std::string& endpoint, const std::string& method,
                          const std::string& data = "");
    Order parseOrderResponse(const std::string& json_response);
    Portfolio parseAccountResponse(const std::string& json_response);
    std::string orderToOandaFormat(const Order& order);
    std::string convertSymbolToOanda(const std::string& symbol);
};

// Smart order execution algorithms
enum class ExecutionAlgorithm {
    MARKET,         // Immediate market order
    VWAP,          // Volume Weighted Average Price
    TWAP,          // Time Weighted Average Price
    IMPLEMENTATION_SHORTFALL, // IS algorithm
    ARRIVAL_PRICE  // Arrival price algorithm
};

struct ExecutionAlgorithmConfig {
    ExecutionAlgorithm algorithm = ExecutionAlgorithm::MARKET;
    int duration_minutes = 60;      // Execution duration for TWAP/VWAP
    double participation_rate = 0.1; // Target participation rate (10%)
    bool allow_crossing = true;     // Allow aggressive executions
    double urgency = 0.5;           // Urgency factor (0-1)
    
    ExecutionAlgorithmConfig() = default;
};

// Order management system
class OrderManager {
public:
    OrderManager();
    ~OrderManager();
    
    // Order lifecycle management
    std::string submitOrder(const Order& order, const ExecutionAlgorithmConfig& algo_config = {});
    bool cancelOrder(const std::string& order_id);
    bool modifyOrder(const std::string& order_id, const Order& modified_order);
    
    // Order status and tracking
    Order getOrder(const std::string& order_id);
    std::vector<Order> getActiveOrders();
    std::vector<Order> getOrdersBySymbol(const std::string& symbol);
    std::vector<Order> getOrdersByStatus(OrderStatus status);
    
    // Execution algorithms
    void setExecutionAlgorithm(const std::string& order_id, const ExecutionAlgorithmConfig& config);
    ExecutionAlgorithmConfig getExecutionAlgorithm(const std::string& order_id);
    
    // Order validation and risk checks
    bool validateOrder(const Order& order);
    std::vector<std::string> getOrderValidationErrors(const Order& order);
    
    // Parent-child order relationships (for algos)
    std::vector<std::string> getChildOrders(const std::string& parent_order_id);
    std::string getParentOrder(const std::string& child_order_id);
    
    // Performance tracking
    std::map<std::string, double> getExecutionMetrics(const std::string& order_id);
    double calculateImplementationShortfall(const std::string& order_id);
    
private:
    std::map<std::string, Order> orders_;
    std::map<std::string, ExecutionAlgorithmConfig> algo_configs_;
    std::map<std::string, std::vector<std::string>> parent_child_map_;
    std::mutex orders_mutex_;
    
    std::string generateOrderId();
    void updateOrderStatus(const std::string& order_id, OrderStatus status);
};

// Main trade execution engine
class TradeExecutor {
public:
    explicit TradeExecutor(const ExecutionConfig& config);
    ~TradeExecutor();
    
    // Configuration
    void setConfig(const ExecutionConfig& config);
    ExecutionConfig getConfig() const { return config_; }
    
    void setBrokerageAPI(std::unique_ptr<BrokerageAPI> api);
    BrokerageAPI* getBrokerageAPI() const { return brokerage_api_.get(); }
    
    // Order execution
    std::future<ExecutionResult> executeOrder(const Order& order);
    std::future<ExecutionResult> executeOrderWithAlgorithm(const Order& order,
                                                          const ExecutionAlgorithmConfig& algo_config);
    
    // Batch order execution
    std::vector<std::future<ExecutionResult>> executeBatch(const std::vector<Order>& orders);
    
    // Signal-based execution
    std::future<ExecutionResult> executeSignal(const TradingSignal& signal, double portfolio_value);
    std::vector<std::future<ExecutionResult>> executeSignals(const std::vector<TradingSignal>& signals,
                                                            double portfolio_value);
    
    // Portfolio rebalancing
    std::vector<std::future<ExecutionResult>> executeRebalance(
        const std::map<std::string, double>& target_weights,
        const Portfolio& current_portfolio);
    
    // Order management
    bool cancelOrder(const std::string& order_id);
    bool modifyOrder(const std::string& order_id, const Order& modified_order);
    
    // Status and monitoring
    std::vector<Order> getActiveOrders();
    std::vector<Trade> getRecentTrades(int hours = 24);
    Portfolio getCurrentPortfolio();
    
    // Risk management
    bool checkRiskLimits(const Order& order);
    std::map<std::string, double> getRiskMetrics();
    void setRiskParams(const RiskParams& params);
    
    // Performance tracking
    PerformanceMetrics getExecutionPerformance();
    double getTotalCommissions(const Timestamp& start_date = {});
    double getAverageSlippage();
    
    // Event callbacks
    void setOrderCallback(OrderCallback callback);
    void setTradeCallback(TradeCallback callback);
    void setErrorCallback(ErrorCallback callback);
    
    // Connection management
    bool connect();
    void disconnect();
    bool isConnected() const;
    
    // Emergency controls
    void cancelAllOrders();
    void enableKillSwitch(bool enable = true);
    bool isKillSwitchActive() const { return kill_switch_active_; }
    
private:
    ExecutionConfig config_;
    std::unique_ptr<BrokerageAPI> brokerage_api_;
    std::unique_ptr<OrderManager> order_manager_;
    RiskParams risk_params_;
    
    // State management
    std::atomic<bool> kill_switch_active_;
    std::atomic<bool> connected_;
    
    // Performance tracking
    std::vector<Trade> trade_history_;
    std::vector<ExecutionResult> execution_results_;
    
    // Callbacks
    OrderCallback order_callback_;
    TradeCallback trade_callback_;
    ErrorCallback error_callback_;
    
    // Threading
    std::queue<Order> order_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_condition_;
    std::atomic<bool> processing_active_;
    std::thread processing_thread_;
    
    // Order processing
    void processOrderQueue();
    ExecutionResult executeOrderInternal(const Order& order);
    Order createOrderFromSignal(const TradingSignal& signal, double portfolio_value);
    
    // Risk checks
    bool checkPositionLimits(const Order& order);
    bool checkDailyLossLimit();
    bool checkOrderSize(const Order& order);
    
    // Execution algorithms
    ExecutionResult executeMarketOrder(const Order& order);
    ExecutionResult executeVWAPOrder(const Order& order, const ExecutionAlgorithmConfig& config);
    ExecutionResult executeTWAPOrder(const Order& order, const ExecutionAlgorithmConfig& config);
    
    // Helper methods
    double calculateSlippage(const Order& order, Price execution_price);
    void updatePerformanceMetrics(const ExecutionResult& result);
    void notifyOrderEvent(const Order& order);
    void notifyTradeEvent(const Trade& trade);
    void notifyError(const std::string& error_code, const std::string& message);
};

// Factory for creating brokerage APIs
class BrokerageFactory {
public:
    static std::unique_ptr<BrokerageAPI> createAPI(const std::string& provider_name,
                                                  const APIConfig& config);
    
    static std::vector<std::string> getAvailableProviders();
    static APIConfig getDefaultConfig(const std::string& provider_name);
    
private:
    static std::map<std::string, std::function<std::unique_ptr<BrokerageAPI>(const APIConfig&)>> api_creators_;
};

// Execution utilities
namespace ExecutionUtils {
    // Order sizing
    Quantity calculatePositionSize(double portfolio_value, Price entry_price,
                                 double risk_per_trade, Price stop_loss);
    
    Quantity adjustForMinimumSize(Quantity desired_quantity, const std::string& symbol);
    Quantity roundToLotSize(Quantity quantity, Quantity lot_size);
    
    // Commission calculation
    double calculateCommission(const Order& order, const APIConfig& config);
    double calculateTotalTransactionCost(const Order& order, double slippage_bps);
    
    // Execution timing
    bool isMarketOpen(const std::string& symbol = "");
    int getMinutesUntilMarketClose();
    bool isGoodTimeToTrade(); // Avoid first/last 30 minutes
    
    // Order validation
    bool isValidOrderSize(Quantity quantity, const std::string& symbol);
    bool isValidOrderPrice(Price price, const std::string& symbol);
    bool isValidSymbol(const std::string& symbol);
    
    // Performance analysis
    double calculateImplementationShortfall(Price decision_price, Price execution_price,
                                          Quantity quantity, OrderSide side);
    
    double calculateVWAPPerformance(const std::vector<Trade>& fills,
                                  const std::vector<OHLCV>& market_data);
    
    // Order splitting
    std::vector<Order> splitLargeOrder(const Order& large_order, Quantity max_size);
    std::vector<Order> createTWAPSlices(const Order& order, int num_slices, int duration_minutes);
}

} // namespace tradingcore
} // namespace parsec