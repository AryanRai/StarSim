#pragma once

#include <string>

namespace parsec {
namespace tradingcore {
namespace Constants {

// Financial constants
constexpr double RISK_FREE_RATE = 0.02; // 2% annual risk-free rate
constexpr double TRADING_DAYS_PER_YEAR = 252.0;
constexpr double HOURS_PER_TRADING_DAY = 6.5; // NYSE: 9:30 AM - 4:00 PM EST
constexpr double MINUTES_PER_TRADING_DAY = HOURS_PER_TRADING_DAY * 60.0;

// Commission and fee rates (typical values)
constexpr double ALPACA_COMMISSION = 0.0; // Commission-free
constexpr double ALPACA_SEC_FEE = 0.0000278; // $0.278 per $100 of sales
constexpr double ALPACA_FINRA_FEE = 0.000145; // $0.145 per $100 of sales

constexpr double OANDA_SPREAD_TYPICAL = 0.0001; // 1 pip for major pairs
constexpr double FOREX_PIP_VALUE = 0.0001; // Standard pip value

// Market hours (in minutes from market open)
constexpr int MARKET_OPEN_MINUTE = 0;
constexpr int MARKET_CLOSE_MINUTE = static_cast<int>(MINUTES_PER_TRADING_DAY);
constexpr int PRE_MARKET_START = -240; // 4 hours before open
constexpr int AFTER_MARKET_END = 240;  // 4 hours after close

// Technical analysis defaults
constexpr int DEFAULT_MA_SHORT_PERIOD = 10;
constexpr int DEFAULT_MA_LONG_PERIOD = 20;
constexpr int DEFAULT_RSI_PERIOD = 14;
constexpr int DEFAULT_ATR_PERIOD = 14;
constexpr int DEFAULT_BOLLINGER_PERIOD = 20;
constexpr double DEFAULT_BOLLINGER_STD = 2.0;

// Risk management defaults
constexpr double DEFAULT_MAX_POSITION_SIZE = 0.1; // 10% of portfolio
constexpr double DEFAULT_STOP_LOSS_PCT = 0.02; // 2% stop loss
constexpr double DEFAULT_TAKE_PROFIT_PCT = 0.04; // 4% take profit (2:1 R/R)
constexpr double DEFAULT_MAX_DAILY_LOSS = 0.02; // 2% max daily loss
constexpr double DEFAULT_MAX_DRAWDOWN = 0.1; // 10% max drawdown
constexpr double DEFAULT_ATR_MULTIPLIER = 2.0; // 2x ATR for stop loss

// Portfolio optimization defaults
constexpr double DEFAULT_PORTFOLIO_REBALANCE_THRESHOLD = 0.05; // 5% deviation
constexpr int DEFAULT_REBALANCE_FREQUENCY_DAYS = 30; // Monthly rebalancing
constexpr double DEFAULT_MIN_WEIGHT = 0.01; // 1% minimum allocation
constexpr double DEFAULT_MAX_WEIGHT = 0.4; // 40% maximum allocation

// API rate limits and timeouts
constexpr int DEFAULT_API_TIMEOUT_MS = 30000; // 30 seconds
constexpr int DEFAULT_RATE_LIMIT_PER_MINUTE = 200;
constexpr int DEFAULT_RETRY_ATTEMPTS = 3;
constexpr int DEFAULT_RETRY_DELAY_MS = 1000; // 1 second

// Data validation constants
constexpr double MIN_VALID_PRICE = 0.01; // $0.01 minimum price
constexpr double MAX_VALID_PRICE = 1000000.0; // $1M maximum price
constexpr double MIN_VALID_VOLUME = 0.0;
constexpr double MAX_VALID_VOLUME = 1000000000.0; // 1B shares
constexpr int MAX_SYMBOL_LENGTH = 10;
constexpr int MIN_HISTORICAL_BARS = 50; // Minimum bars for analysis

// Signal confidence thresholds
constexpr double HIGH_CONFIDENCE_THRESHOLD = 0.8;
constexpr double MEDIUM_CONFIDENCE_THRESHOLD = 0.6;
constexpr double LOW_CONFIDENCE_THRESHOLD = 0.4;

// Portfolio constraints
constexpr double MIN_CASH_RESERVE = 0.05; // 5% cash reserve
constexpr double MAX_LEVERAGE = 1.0; // No leverage by default
constexpr int MAX_POSITIONS = 20; // Maximum number of positions

// Backtesting defaults
constexpr double DEFAULT_INITIAL_CAPITAL = 10000.0; // $10,000
constexpr double DEFAULT_SLIPPAGE_BPS = 5.0; // 5 basis points slippage
constexpr int MIN_BACKTEST_PERIODS = 100; // Minimum periods for backtest

// String constants for API endpoints
namespace Alpaca {
    inline const std::string PAPER_BASE_URL = "https://paper-api.alpaca.markets";
    inline const std::string LIVE_BASE_URL = "https://api.alpaca.markets";
    inline const std::string DATA_BASE_URL = "https://data.alpaca.markets";
    
    inline const std::string ENDPOINT_ACCOUNT = "/v2/account";
    inline const std::string ENDPOINT_POSITIONS = "/v2/positions";
    inline const std::string ENDPOINT_ORDERS = "/v2/orders";
    inline const std::string ENDPOINT_BARS = "/v2/stocks/{symbol}/bars";
    inline const std::string ENDPOINT_TRADES = "/v2/stocks/{symbol}/trades";
}

namespace OANDA {
    inline const std::string PRACTICE_BASE_URL = "https://api-fxpractice.oanda.com";
    inline const std::string LIVE_BASE_URL = "https://api-fxtrade.oanda.com";
    
    inline const std::string ENDPOINT_ACCOUNT = "/v3/accounts/{accountID}";
    inline const std::string ENDPOINT_POSITIONS = "/v3/accounts/{accountID}/positions";
    inline const std::string ENDPOINT_ORDERS = "/v3/accounts/{accountID}/orders";
    inline const std::string ENDPOINT_CANDLES = "/v3/instruments/{instrument}/candles";
    inline const std::string ENDPOINT_PRICES = "/v3/accounts/{accountID}/pricing";
}

// Common symbols and instruments
namespace Symbols {
    // US Equities ETFs
    inline const std::string SPY = "SPY";   // S&P 500 ETF
    inline const std::string QQQ = "QQQ";   // NASDAQ ETF
    inline const std::string IWM = "IWM";   // Russell 2000 ETF
    inline const std::string VTI = "VTI";   // Total Stock Market ETF
    inline const std::string VXUS = "VXUS"; // International ETF
    
    // Sector ETFs
    inline const std::string XLF = "XLF";   // Financial Sector
    inline const std::string XLK = "XLK";   // Technology Sector
    inline const std::string XLE = "XLE";   // Energy Sector
    inline const std::string XLV = "XLV";   // Healthcare Sector
    
    // Bond ETFs
    inline const std::string TLT = "TLT";   // 20+ Year Treasury
    inline const std::string AGG = "AGG";   // Aggregate Bond
    inline const std::string HYG = "HYG";   // High Yield Corporate
    
    // Commodity ETFs
    inline const std::string GLD = "GLD";   // Gold ETF
    inline const std::string SLV = "SLV";   // Silver ETF
    inline const std::string USO = "USO";   // Oil ETF
    
    // Major Forex Pairs (OANDA format)
    inline const std::string EUR_USD = "EUR_USD";
    inline const std::string GBP_USD = "GBP_USD";
    inline const std::string USD_JPY = "USD_JPY";
    inline const std::string USD_CHF = "USD_CHF";
    inline const std::string AUD_USD = "AUD_USD";
    inline const std::string USD_CAD = "USD_CAD";
    inline const std::string NZD_USD = "NZD_USD";
}

// Error codes and messages
namespace Errors {
    inline const std::string INVALID_SYMBOL = "INVALID_SYMBOL";
    inline const std::string INSUFFICIENT_FUNDS = "INSUFFICIENT_FUNDS";
    inline const std::string API_RATE_LIMIT = "API_RATE_LIMIT";
    inline const std::string MARKET_CLOSED = "MARKET_CLOSED";
    inline const std::string INVALID_ORDER_SIZE = "INVALID_ORDER_SIZE";
    inline const std::string CONNECTION_TIMEOUT = "CONNECTION_TIMEOUT";
    inline const std::string AUTHENTICATION_FAILED = "AUTHENTICATION_FAILED";
    inline const std::string INVALID_CONFIGURATION = "INVALID_CONFIGURATION";
}

// Timeframe strings
namespace Timeframes {
    inline const std::string ONE_MINUTE = "1Min";
    inline const std::string FIVE_MINUTES = "5Min";
    inline const std::string FIFTEEN_MINUTES = "15Min";
    inline const std::string THIRTY_MINUTES = "30Min";
    inline const std::string ONE_HOUR = "1Hour";
    inline const std::string ONE_DAY = "1Day";
    inline const std::string ONE_WEEK = "1Week";
    inline const std::string ONE_MONTH = "1Month";
}

} // namespace Constants
} // namespace tradingcore
} // namespace parsec