#include <gtest/gtest.h>
#include "tradingcore/TradingCore.h"
#include "tradingcore/TradingDataStructures.h"
#include "tradingcore/static/TradingConstants.h"

namespace parsec {
namespace tradingcore {

class TradingCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_ = TradingCoreConfig::createDefaultConfig();
    }

    TradingCoreConfig config_;
};

TEST_F(TradingCoreTest, ConfigDefaultCreation) {
    EXPECT_TRUE(config_.paper_trading_mode);
    EXPECT_FALSE(config_.enable_real_time_data);
    EXPECT_TRUE(config_.enable_logging);
    EXPECT_EQ(config_.log_level, "INFO");
}

TEST_F(TradingCoreTest, ConfigValidation) {
    auto valid_config = TradingCoreConfig::createDefaultConfig();
    EXPECT_TRUE(valid_config.validate());
    EXPECT_TRUE(valid_config.getValidationErrors().empty());
}

TEST_F(TradingCoreTest, TradingCoreConstruction) {
    EXPECT_NO_THROW({
        TradingCore trading_core(config_);
    });
}

TEST_F(TradingCoreTest, TradingConstants) {
    EXPECT_GT(Constants::RISK_FREE_RATE, 0.0);
    EXPECT_GT(Constants::TRADING_DAYS_PER_YEAR, 0.0);
    EXPECT_EQ(Constants::TRADING_DAYS_PER_YEAR, 252.0);
}

TEST_F(TradingCoreTest, DataStructureBasics) {
    OHLCV bar;
    bar.open = 100.0;
    bar.high = 105.0;
    bar.low = 99.0;
    bar.close = 103.0;
    bar.volume = 1000.0;
    
    EXPECT_EQ(bar.open, 100.0);
    EXPECT_EQ(bar.high, 105.0);
    EXPECT_EQ(bar.low, 99.0);
    EXPECT_EQ(bar.close, 103.0);
    EXPECT_EQ(bar.volume, 1000.0);
}

TEST_F(TradingCoreTest, TradingSignalCreation) {
    TradingSignal signal;
    signal.symbol = "AAPL";
    signal.type = SignalType::BUY;
    signal.price = 150.0;
    signal.confidence = 0.9;
    signal.strategy_name = "TestStrategy";
    
    EXPECT_EQ(signal.symbol, "AAPL");
    EXPECT_EQ(signal.type, SignalType::BUY);
    EXPECT_EQ(signal.price, 150.0);
    EXPECT_EQ(signal.confidence, 0.9);
    EXPECT_EQ(signal.strategy_name, "TestStrategy");
}

TEST_F(TradingCoreTest, PortfolioStructure) {
    Portfolio portfolio;
    portfolio.cash = 10000.0;
    portfolio.total_value = 15000.0;
    
    Position position;
    position.symbol = "AAPL";
    position.quantity = 100.0;
    position.average_cost = 150.0;
    position.current_price = 155.0;
    
    portfolio.positions["AAPL"] = position;
    
    EXPECT_EQ(portfolio.cash, 10000.0);
    EXPECT_EQ(portfolio.total_value, 15000.0);
    EXPECT_EQ(portfolio.positions.size(), 1);
    EXPECT_EQ(portfolio.positions["AAPL"].quantity, 100.0);
}

TEST_F(TradingCoreTest, OrderCreation) {
    Order order;
    order.symbol = "AAPL";
    order.side = OrderSide::BUY;
    order.type = OrderType::MARKET;
    order.quantity = 100.0;
    order.status = OrderStatus::PENDING;
    
    EXPECT_EQ(order.symbol, "AAPL");
    EXPECT_EQ(order.side, OrderSide::BUY);
    EXPECT_EQ(order.type, OrderType::MARKET);
    EXPECT_EQ(order.quantity, 100.0);
    EXPECT_EQ(order.status, OrderStatus::PENDING);
}

TEST_F(TradingCoreTest, ConfigFactoryMethods) {
    auto backtest_config = TradingCoreConfig::createBacktestConfig();
    EXPECT_TRUE(backtest_config.paper_trading_mode);
    EXPECT_FALSE(backtest_config.enable_real_time_data);
    
    auto paper_trading_config = TradingCoreConfig::createPaperTradingConfig();
    EXPECT_TRUE(paper_trading_config.paper_trading_mode);
    
    auto live_trading_config = TradingCoreConfig::createLiveTradingConfig();
    EXPECT_FALSE(live_trading_config.paper_trading_mode);
}

TEST_F(TradingCoreTest, PerformanceMetricsStructure) {
    PerformanceMetrics metrics;
    metrics.total_return = 0.15;
    metrics.annual_return = 0.12;
    metrics.sharpe_ratio = 0.67;
    metrics.max_drawdown = -0.08;
    metrics.win_rate = 0.65;
    
    EXPECT_EQ(metrics.total_return, 0.15);
    EXPECT_EQ(metrics.annual_return, 0.12);
    EXPECT_EQ(metrics.sharpe_ratio, 0.67);
    EXPECT_EQ(metrics.max_drawdown, -0.08);
    EXPECT_EQ(metrics.win_rate, 0.65);
}

} // namespace tradingcore
} // namespace parsec