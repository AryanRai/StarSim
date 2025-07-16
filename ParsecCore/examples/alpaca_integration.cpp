#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

#include "tradingcore/TradingCore.h"

using json = nlohmann::json;
using namespace parsec::tradingcore;

// Callback function for libcurl
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    s->append((char*)contents, newLength);
    return newLength;
}

class AlpacaAPIClient {
private:
    std::string api_key_;
    std::string secret_key_;
    std::string base_url_;
    std::string data_url_;
    bool paper_trading_;
    
    CURL* curl_;
    struct curl_slist* headers_;
    
public:
    AlpacaAPIClient(const std::string& api_key, const std::string& secret_key, bool paper_trading = true)
        : api_key_(api_key), secret_key_(secret_key), paper_trading_(paper_trading), curl_(nullptr), headers_(nullptr) {
        
        if (paper_trading_) {
            base_url_ = "https://paper-api.alpaca.markets";
            data_url_ = "https://data.alpaca.markets";
        } else {
            base_url_ = "https://api.alpaca.markets";
            data_url_ = "https://data.alpaca.markets";
        }
        
        // Initialize libcurl
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_ = curl_easy_init();
        
        if (curl_) {
            // Setup headers
            std::string auth_header = "APCA-API-KEY-ID: " + api_key_;
            std::string secret_header = "APCA-API-SECRET-KEY: " + secret_key_;
            
            headers_ = curl_slist_append(headers_, auth_header.c_str());
            headers_ = curl_slist_append(headers_, secret_header.c_str());
            headers_ = curl_slist_append(headers_, "Content-Type: application/json");
            
            curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers_);
            curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
        }
    }
    
    ~AlpacaAPIClient() {
        if (headers_) {
            curl_slist_free_all(headers_);
        }
        if (curl_) {
            curl_easy_cleanup(curl_);
        }
        curl_global_cleanup();
    }
    
    std::string makeRequest(const std::string& endpoint, const std::string& method = "GET", const std::string& data = "") {
        if (!curl_) {
            throw std::runtime_error("CURL not initialized");
        }
        
        std::string response;
        std::string url = base_url_ + endpoint;
        
        curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
        
        if (method == "POST") {
            curl_easy_setopt(curl_, CURLOPT_POST, 1L);
            if (!data.empty()) {
                curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
            }
        } else if (method == "DELETE") {
            curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");
        } else {
            curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
        }
        
        CURLcode res = curl_easy_perform(curl_);
        
        if (res != CURLE_OK) {
            throw std::runtime_error("CURL request failed: " + std::string(curl_easy_strerror(res)));
        }
        
        return response;
    }
    
    json getAccount() {
        try {
            std::string response = makeRequest("/v2/account");
            return json::parse(response);
        } catch (const std::exception& e) {
            std::cerr << "Error getting account: " << e.what() << std::endl;
            return json{};
        }
    }
    
    json getPositions() {
        try {
            std::string response = makeRequest("/v2/positions");
            return json::parse(response);
        } catch (const std::exception& e) {
            std::cerr << "Error getting positions: " << e.what() << std::endl;
            return json{};
        }
    }
    
    json getOrders() {
        try {
            std::string response = makeRequest("/v2/orders");
            return json::parse(response);
        } catch (const std::exception& e) {
            std::cerr << "Error getting orders: " << e.what() << std::endl;
            return json{};
        }
    }
    
    json submitOrder(const std::string& symbol, const std::string& side, const std::string& type, 
                     const std::string& time_in_force, double quantity, double price = 0.0) {
        try {
            json order_data = {
                {"symbol", symbol},
                {"side", side},
                {"type", type},
                {"time_in_force", time_in_force},
                {"qty", std::to_string(quantity)}
            };
            
            if (type == "limit" && price > 0) {
                order_data["limit_price"] = std::to_string(price);
            }
            
            std::string response = makeRequest("/v2/orders", "POST", order_data.dump());
            return json::parse(response);
        } catch (const std::exception& e) {
            std::cerr << "Error submitting order: " << e.what() << std::endl;
            return json{};
        }
    }
    
    json cancelOrder(const std::string& order_id) {
        try {
            std::string endpoint = "/v2/orders/" + order_id;
            std::string response = makeRequest(endpoint, "DELETE");
            return json::parse(response);
        } catch (const std::exception& e) {
            std::cerr << "Error canceling order: " << e.what() << std::endl;
            return json{};
        }
    }
    
    json getLatestQuote(const std::string& symbol) {
        try {
            CURL* data_curl = curl_easy_init();
            if (!data_curl) {
                throw std::runtime_error("Failed to initialize data CURL");
            }
            
            std::string response;
            std::string url = data_url_ + "/v2/stocks/" + symbol + "/quotes/latest";
            
            curl_easy_setopt(data_curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(data_curl, CURLOPT_HTTPHEADER, headers_);
            curl_easy_setopt(data_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(data_curl, CURLOPT_WRITEDATA, &response);
            
            CURLcode res = curl_easy_perform(data_curl);
            curl_easy_cleanup(data_curl);
            
            if (res != CURLE_OK) {
                throw std::runtime_error("Data request failed: " + std::string(curl_easy_strerror(res)));
            }
            
            return json::parse(response);
        } catch (const std::exception& e) {
            std::cerr << "Error getting quote for " << symbol << ": " << e.what() << std::endl;
            return json{};
        }
    }
    
    json getHistoricalBars(const std::string& symbol, const std::string& timeframe = "1Day", int limit = 100) {
        try {
            CURL* data_curl = curl_easy_init();
            if (!data_curl) {
                throw std::runtime_error("Failed to initialize data CURL");
            }
            
            std::string response;
            std::string url = data_url_ + "/v2/stocks/" + symbol + "/bars?timeframe=" + timeframe + "&limit=" + std::to_string(limit);
            
            curl_easy_setopt(data_curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(data_curl, CURLOPT_HTTPHEADER, headers_);
            curl_easy_setopt(data_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(data_curl, CURLOPT_WRITEDATA, &response);
            
            CURLcode res = curl_easy_perform(data_curl);
            curl_easy_cleanup(data_curl);
            
            if (res != CURLE_OK) {
                throw std::runtime_error("Historical data request failed: " + std::string(curl_easy_strerror(res)));
            }
            
            return json::parse(response);
        } catch (const std::exception& e) {
            std::cerr << "Error getting historical data for " << symbol << ": " << e.what() << std::endl;
            return json{};
        }
    }
    
    bool isMarketOpen() {
        try {
            std::string response = makeRequest("/v2/clock");
            json clock_data = json::parse(response);
            return clock_data["is_open"].get<bool>();
        } catch (const std::exception& e) {
            std::cerr << "Error checking market status: " << e.what() << std::endl;
            return false;
        }
    }
    
    json getMarketCalendar() {
        try {
            std::string response = makeRequest("/v2/calendar");
            return json::parse(response);
        } catch (const std::exception& e) {
            std::cerr << "Error getting market calendar: " << e.what() << std::endl;
            return json{};
        }
    }
};

class LivePaperTradingBot {
private:
    std::unique_ptr<AlpacaAPIClient> alpaca_client_;
    std::vector<std::string> universe_;
    std::map<std::string, double> current_prices_;
    std::map<std::string, std::vector<double>> price_history_;
    
    // Trading parameters
    double initial_capital_;
    double current_portfolio_value_;
    double cash_balance_;
    std::map<std::string, double> positions_;
    
    // Risk management
    double max_position_size_;
    double stop_loss_pct_;
    double take_profit_pct_;
    
    // Performance tracking
    std::vector<double> portfolio_values_;
    std::chrono::steady_clock::time_point start_time_;
    
public:
    LivePaperTradingBot(const std::string& api_key, const std::string& secret_key, 
                        double initial_capital = 100000.0)
        : initial_capital_(initial_capital), current_portfolio_value_(initial_capital),
          cash_balance_(initial_capital), max_position_size_(0.05), 
          stop_loss_pct_(0.05), take_profit_pct_(0.15) {
        
        alpaca_client_ = std::make_unique<AlpacaAPIClient>(api_key, secret_key, true);
        
        // Define trading universe - 2024 recommendations
        universe_ = {
            // AI/Technology Leaders
            "AAPL", "MSFT", "GOOGL", "AMZN", "NVDA", "META", "TSLA",
            // Growth Stocks
            "PLTR", "SNOW", "CRWD", "DDOG", "MDB", "NET", "OKTA",
            // ETFs for diversification
            "SPY", "QQQ", "VTI", "ARKK", "XLK", "SMH",
            // Defensive positions
            "JNJ", "PG", "KO", "WMT", "BRK.B"
        };
        
        start_time_ = std::chrono::steady_clock::now();
    }
    
    bool initialize() {
        std::cout << "Initializing Live Paper Trading Bot..." << std::endl;
        
        // Test API connection
        json account = alpaca_client_->getAccount();
        if (account.empty()) {
            std::cerr << "Failed to connect to Alpaca API" << std::endl;
            return false;
        }
        
        std::cout << "✅ Connected to Alpaca API" << std::endl;
        std::cout << "Account Status: " << account["status"].get<std::string>() << std::endl;
        std::cout << "Buying Power: $" << std::fixed << std::setprecision(2) 
                  << std::stod(account["buying_power"].get<std::string>()) << std::endl;
        
        // Check market status
        bool market_open = alpaca_client_->isMarketOpen();
        std::cout << "Market Status: " << (market_open ? "OPEN" : "CLOSED") << std::endl;
        
        // Initialize price data
        updateMarketData();
        
        return true;
    }
    
    void updateMarketData() {
        std::cout << "Updating market data..." << std::endl;
        
        for (const auto& symbol : universe_) {
            try {
                json quote = alpaca_client_->getLatestQuote(symbol);
                
                if (!quote.empty() && quote.contains("quote")) {
                    double bid = quote["quote"]["bid_price"].get<double>();
                    double ask = quote["quote"]["ask_price"].get<double>();
                    double mid_price = (bid + ask) / 2.0;
                    
                    current_prices_[symbol] = mid_price;
                    price_history_[symbol].push_back(mid_price);
                    
                    // Keep only last 50 prices for analysis
                    if (price_history_[symbol].size() > 50) {
                        price_history_[symbol].erase(price_history_[symbol].begin());
                    }
                    
                    std::cout << symbol << ": $" << std::fixed << std::setprecision(2) 
                              << mid_price << " (Bid: $" << bid << ", Ask: $" << ask << ")" << std::endl;
                }
                
                // Rate limiting
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
            } catch (const std::exception& e) {
                std::cerr << "Error updating " << symbol << ": " << e.what() << std::endl;
            }
        }
    }
    
    std::vector<TradingSignal> generateSignals() {
        std::vector<TradingSignal> signals;
        
        for (const auto& symbol : universe_) {
            if (price_history_[symbol].size() < 10) continue;
            
            // Calculate simple momentum
            const auto& prices = price_history_[symbol];
            double current_price = prices.back();
            double sma_5 = 0.0, sma_20 = 0.0;
            
            // 5-day SMA
            if (prices.size() >= 5) {
                for (size_t i = prices.size() - 5; i < prices.size(); ++i) {
                    sma_5 += prices[i];
                }
                sma_5 /= 5.0;
            }
            
            // 20-day SMA
            if (prices.size() >= 20) {
                for (size_t i = prices.size() - 20; i < prices.size(); ++i) {
                    sma_20 += prices[i];
                }
                sma_20 /= 20.0;
            }
            
            // Generate signal
            TradingSignal signal;
            signal.symbol = symbol;
            signal.price = current_price;
            signal.timestamp = std::chrono::system_clock::now();
            signal.strategy_name = "LiveMomentum";
            signal.confidence = 0.0;
            signal.type = SignalType::HOLD;
            
            // Momentum strategy
            if (sma_5 > sma_20 * 1.02 && current_price > sma_5) {
                signal.type = SignalType::BUY;
                signal.confidence = 0.7;
            } else if (sma_5 < sma_20 * 0.98 && current_price < sma_5) {
                signal.type = SignalType::SELL;
                signal.confidence = 0.7;
            }
            
            // Enhanced signals for AI/Tech stocks
            if (symbol == "NVDA" || symbol == "PLTR" || symbol == "TSLA") {
                signal.confidence *= 1.2; // Boost confidence for high-growth stocks
            }
            
            if (signal.type != SignalType::HOLD && signal.confidence > 0.6) {
                signals.push_back(signal);
            }
        }
        
        return signals;
    }
    
    void executeSignals(const std::vector<TradingSignal>& signals) {
        for (const auto& signal : signals) {
            try {
                double position_size = current_portfolio_value_ * max_position_size_;
                double quantity = std::floor(position_size / signal.price);
                
                if (quantity < 1) continue;
                
                std::string side = (signal.type == SignalType::BUY) ? "buy" : "sell";
                
                // Check if we already have a position
                bool has_position = positions_.count(signal.symbol) && positions_[signal.symbol] != 0;
                
                if (signal.type == SignalType::BUY && !has_position) {
                    // Place buy order
                    json order_result = alpaca_client_->submitOrder(
                        signal.symbol, "buy", "market", "day", quantity);
                    
                    if (!order_result.empty()) {
                        std::cout << "[ORDER] BUY " << signal.symbol 
                                  << " | Qty: " << quantity 
                                  << " | Price: $" << std::fixed << std::setprecision(2) << signal.price
                                  << " | Confidence: " << signal.confidence << std::endl;
                        
                        // Update position tracking
                        positions_[signal.symbol] = quantity;
                        cash_balance_ -= quantity * signal.price;
                    }
                } else if (signal.type == SignalType::SELL && has_position) {
                    // Place sell order
                    double sell_quantity = std::min(quantity, positions_[signal.symbol]);
                    
                    json order_result = alpaca_client_->submitOrder(
                        signal.symbol, "sell", "market", "day", sell_quantity);
                    
                    if (!order_result.empty()) {
                        std::cout << "[ORDER] SELL " << signal.symbol 
                                  << " | Qty: " << sell_quantity 
                                  << " | Price: $" << std::fixed << std::setprecision(2) << signal.price
                                  << " | Confidence: " << signal.confidence << std::endl;
                        
                        // Update position tracking
                        positions_[signal.symbol] -= sell_quantity;
                        cash_balance_ += sell_quantity * signal.price;
                    }
                }
                
                // Rate limiting
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                
            } catch (const std::exception& e) {
                std::cerr << "Error executing signal for " << signal.symbol << ": " << e.what() << std::endl;
            }
        }
    }
    
    void updatePortfolioValue() {
        double total_value = cash_balance_;
        
        for (const auto& [symbol, quantity] : positions_) {
            if (quantity > 0 && current_prices_.count(symbol)) {
                total_value += quantity * current_prices_[symbol];
            }
        }
        
        current_portfolio_value_ = total_value;
        portfolio_values_.push_back(total_value);
    }
    
    void printDashboard() {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::minutes>(now - start_time_);
        
        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << "LIVE PAPER TRADING DASHBOARD" << std::endl;
        std::cout << std::string(70, '=') << std::endl;
        
        std::cout << "Runtime: " << duration.count() << " minutes" << std::endl;
        std::cout << "Market Status: " << (alpaca_client_->isMarketOpen() ? "OPEN" : "CLOSED") << std::endl;
        
        std::cout << "\n💰 Portfolio Summary:" << std::endl;
        std::cout << "Initial Capital: $" << std::fixed << std::setprecision(2) << initial_capital_ << std::endl;
        std::cout << "Current Value: $" << current_portfolio_value_ << std::endl;
        std::cout << "Cash Balance: $" << cash_balance_ << std::endl;
        
        double total_return = ((current_portfolio_value_ - initial_capital_) / initial_capital_) * 100;
        std::cout << "Total Return: " << std::setprecision(2) << total_return << "%" << std::endl;
        
        std::cout << "\n📊 Current Positions:" << std::endl;
        for (const auto& [symbol, quantity] : positions_) {
            if (quantity > 0) {
                double current_price = current_prices_.count(symbol) ? current_prices_[symbol] : 0.0;
                double position_value = quantity * current_price;
                double weight = (position_value / current_portfolio_value_) * 100;
                
                std::cout << "  " << symbol << ": " << std::setprecision(0) << quantity 
                          << " shares @ $" << std::setprecision(2) << current_price 
                          << " = $" << position_value << " (" << weight << "%)" << std::endl;
            }
        }
        
        std::cout << "\n📈 Top Stock Prices:" << std::endl;
        std::vector<std::string> top_stocks = {"NVDA", "AAPL", "MSFT", "GOOGL", "PLTR", "TSLA"};
        for (const auto& symbol : top_stocks) {
            if (current_prices_.count(symbol)) {
                std::cout << "  " << symbol << ": $" << std::fixed << std::setprecision(2) 
                          << current_prices_[symbol] << std::endl;
            }
        }
        
        std::cout << std::string(70, '=') << std::endl;
    }
    
    void runTradingSession(int duration_minutes = 480) { // 8 hours default
        std::cout << "🚀 Starting Live Paper Trading Session" << std::endl;
        std::cout << "Duration: " << duration_minutes << " minutes" << std::endl;
        std::cout << "Universe: " << universe_.size() << " symbols" << std::endl;
        std::cout << "Max Position Size: " << (max_position_size_ * 100) << "%" << std::endl;
        std::cout << std::string(70, '-') << std::endl;
        
        if (!initialize()) {
            std::cerr << "Failed to initialize trading bot" << std::endl;
            return;
        }
        
        auto end_time = std::chrono::steady_clock::now() + std::chrono::minutes(duration_minutes);
        int cycle_count = 0;
        
        while (std::chrono::steady_clock::now() < end_time) {
            try {
                std::cout << "\n[CYCLE " << cycle_count << "] " << std::endl;
                
                // Update market data
                updateMarketData();
                
                // Generate signals
                auto signals = generateSignals();
                std::cout << "Generated " << signals.size() << " trading signals" << std::endl;
                
                // Execute signals
                if (!signals.empty()) {
                    executeSignals(signals);
                }
                
                // Update portfolio value
                updatePortfolioValue();
                
                // Print dashboard every 5 cycles
                if (cycle_count % 5 == 0) {
                    printDashboard();
                }
                
                cycle_count++;
                
                // Wait 5 minutes between cycles
                std::this_thread::sleep_for(std::chrono::minutes(5));
                
            } catch (const std::exception& e) {
                std::cerr << "Error in trading cycle: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::minutes(1));
            }
        }
        
        // Final dashboard
        printDashboard();
        
        std::cout << "\n🎉 Live Paper Trading Session Complete!" << std::endl;
        
        // Export results
        std::ofstream results("live_trading_results.csv");
        results << "Cycle,Portfolio_Value,Cash_Balance,Total_Return\n";
        for (size_t i = 0; i < portfolio_values_.size(); ++i) {
            double total_return = ((portfolio_values_[i] - initial_capital_) / initial_capital_) * 100;
            results << i << "," << portfolio_values_[i] << "," << cash_balance_ << "," << total_return << "\n";
        }
        results.close();
        
        std::cout << "Results exported to live_trading_results.csv" << std::endl;
    }
};

int main() {
    std::cout << "🚀 Live Paper Trading with Alpaca API" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    // Get API credentials from environment or user input
    std::string api_key, secret_key;
    
    const char* env_api_key = std::getenv("ALPACA_API_KEY");
    const char* env_secret_key = std::getenv("ALPACA_SECRET_KEY");
    
    if (env_api_key && env_secret_key) {
        api_key = env_api_key;
        secret_key = env_secret_key;
        std::cout << "Using API credentials from environment variables" << std::endl;
    } else {
        std::cout << "Enter your Alpaca API credentials:" << std::endl;
        std::cout << "API Key: ";
        std::cin >> api_key;
        std::cout << "Secret Key: ";
        std::cin >> secret_key;
    }
    
    try {
        // Create and run the trading bot
        LivePaperTradingBot bot(api_key, secret_key, 100000.0);
        
        // Run for 8 hours (or until market close)
        bot.runTradingSession(480);
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}