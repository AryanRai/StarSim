#pragma once

#include "tradingcore/TradingDataStructures.h"
#include "tradingcore/static/TradingConstants.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <future>
#include <functional>

namespace parsec {
namespace tradingcore {

// Forward declarations
class APIClient;

class MarketDataProvider {
public:
    virtual ~MarketDataProvider() = default;
    
    // Synchronous data fetching
    virtual HistoricalData getHistoricalData(const std::string& symbol, 
                                            const std::string& timeframe,
                                            int bars = 1000) = 0;
    
    virtual MarketDataPoint getCurrentPrice(const std::string& symbol) = 0;
    
    virtual std::map<std::string, MarketDataPoint> getCurrentPrices(
        const std::vector<std::string>& symbols) = 0;
    
    // Asynchronous data fetching
    virtual std::future<HistoricalData> getHistoricalDataAsync(const std::string& symbol,
                                                              const std::string& timeframe,
                                                              int bars = 1000) = 0;
    
    // Real-time data streaming
    virtual bool startRealTimeStream(const std::vector<std::string>& symbols,
                                   MarketDataCallback callback) = 0;
    
    virtual void stopRealTimeStream() = 0;
    
    // Configuration and status
    virtual bool isConnected() const = 0;
    virtual std::string getProviderName() const = 0;
    virtual APIConfig getConfig() const = 0;
    virtual void setConfig(const APIConfig& config) = 0;
};

class AlpacaDataProvider : public MarketDataProvider {
public:
    explicit AlpacaDataProvider(const APIConfig& config);
    ~AlpacaDataProvider() override;
    
    // MarketDataProvider interface
    HistoricalData getHistoricalData(const std::string& symbol,
                                   const std::string& timeframe,
                                   int bars = 1000) override;
    
    MarketDataPoint getCurrentPrice(const std::string& symbol) override;
    
    std::map<std::string, MarketDataPoint> getCurrentPrices(
        const std::vector<std::string>& symbols) override;
    
    std::future<HistoricalData> getHistoricalDataAsync(const std::string& symbol,
                                                      const std::string& timeframe,
                                                      int bars = 1000) override;
    
    bool startRealTimeStream(const std::vector<std::string>& symbols,
                           MarketDataCallback callback) override;
    
    void stopRealTimeStream() override;
    
    bool isConnected() const override;
    std::string getProviderName() const override { return "Alpaca"; }
    APIConfig getConfig() const override { return config_; }
    void setConfig(const APIConfig& config) override;
    
private:
    APIConfig config_;
    std::unique_ptr<APIClient> api_client_;
    bool streaming_active_;
    MarketDataCallback stream_callback_;
    
    // Helper methods
    std::string buildBarsURL(const std::string& symbol, const std::string& timeframe, int bars);
    std::string buildLatestQuoteURL(const std::string& symbol);
    HistoricalData parseHistoricalResponse(const std::string& json_response);
    MarketDataPoint parseQuoteResponse(const std::string& json_response);
    void handleStreamMessage(const std::string& message);
};

class OandaDataProvider : public MarketDataProvider {
public:
    explicit OandaDataProvider(const APIConfig& config);
    ~OandaDataProvider() override;
    
    // MarketDataProvider interface
    HistoricalData getHistoricalData(const std::string& symbol,
                                   const std::string& timeframe,
                                   int bars = 1000) override;
    
    MarketDataPoint getCurrentPrice(const std::string& symbol) override;
    
    std::map<std::string, MarketDataPoint> getCurrentPrices(
        const std::vector<std::string>& symbols) override;
    
    std::future<HistoricalData> getHistoricalDataAsync(const std::string& symbol,
                                                      const std::string& timeframe,
                                                      int bars = 1000) override;
    
    bool startRealTimeStream(const std::vector<std::string>& symbols,
                           MarketDataCallback callback) override;
    
    void stopRealTimeStream() override;
    
    bool isConnected() const override;
    std::string getProviderName() const override { return "OANDA"; }
    APIConfig getConfig() const override { return config_; }
    void setConfig(const APIConfig& config) override;
    
private:
    APIConfig config_;
    std::unique_ptr<APIClient> api_client_;
    bool streaming_active_;
    MarketDataCallback stream_callback_;
    
    // Helper methods
    std::string buildCandlesURL(const std::string& instrument, const std::string& granularity, int count);
    std::string buildPricingURL(const std::vector<std::string>& instruments);
    HistoricalData parseCandlesResponse(const std::string& json_response);
    std::map<std::string, MarketDataPoint> parsePricingResponse(const std::string& json_response);
    std::string convertTimeframe(const std::string& timeframe);
};

// Factory for creating market data providers
class MarketDataFactory {
public:
    static std::unique_ptr<MarketDataProvider> createProvider(const std::string& provider_name,
                                                             const APIConfig& config);
    
    static std::vector<std::string> getAvailableProviders();
    
private:
    static std::map<std::string, std::function<std::unique_ptr<MarketDataProvider>(const APIConfig&)>> providers_;
};

// Main market data manager class
class MarketDataManager {
public:
    explicit MarketDataManager(const MarketDataConfig& config);
    ~MarketDataManager();
    
    // Configuration
    void setConfig(const MarketDataConfig& config);
    MarketDataConfig getConfig() const { return config_; }
    
    void addProvider(const std::string& name, std::unique_ptr<MarketDataProvider> provider);
    void removeProvider(const std::string& name);
    MarketDataProvider* getProvider(const std::string& name = "");
    
    // Data fetching methods
    HistoricalData getHistoricalData(const std::string& symbol,
                                   const std::string& timeframe = "",
                                   int bars = 0,
                                   const std::string& provider = "");
    
    MarketDataPoint getCurrentPrice(const std::string& symbol,
                                  const std::string& provider = "");
    
    std::map<std::string, MarketDataPoint> getCurrentPrices(
        const std::vector<std::string>& symbols,
        const std::string& provider = "");
    
    // Batch operations
    std::map<std::string, HistoricalData> getMultipleHistoricalData(
        const std::vector<std::string>& symbols,
        const std::string& timeframe = "",
        int bars = 0,
        const std::string& provider = "");
    
    // Real-time streaming
    bool startRealTimeStream(const std::vector<std::string>& symbols,
                           MarketDataCallback callback,
                           const std::string& provider = "");
    
    void stopRealTimeStream(const std::string& provider = "");
    
    // Data validation and quality checks
    bool validateData(const HistoricalData& data);
    bool validateDataPoint(const MarketDataPoint& point);
    
    // Data caching (optional)
    void enableCaching(bool enable = true);
    void clearCache();
    void setCacheExpiry(int seconds);
    
    // Error handling
    void setErrorCallback(ErrorCallback callback);
    
    // Status and diagnostics
    bool isProviderConnected(const std::string& provider = "");
    std::vector<std::string> getAvailableProviders();
    std::map<std::string, std::string> getProviderStatus();
    
private:
    MarketDataConfig config_;
    std::map<std::string, std::unique_ptr<MarketDataProvider>> providers_;
    std::string default_provider_;
    
    // Caching
    bool caching_enabled_;
    int cache_expiry_seconds_;
    std::map<std::string, std::pair<HistoricalData, Timestamp>> cache_;
    
    // Error handling
    ErrorCallback error_callback_;
    
    // Helper methods
    std::string generateCacheKey(const std::string& symbol, const std::string& timeframe, int bars);
    bool isCacheValid(const Timestamp& cached_time);
    void logError(const std::string& error_code, const std::string& message);
    MarketDataProvider* selectProvider(const std::string& requested_provider = "");
};

// Utility functions for market data operations
namespace MarketDataUtils {
    // Data processing
    std::vector<Price> extractPrices(const HistoricalData& data, const std::string& price_type = "close");
    HistoricalData resample(const HistoricalData& data, const std::string& new_timeframe);
    HistoricalData fillMissingData(const HistoricalData& data);
    
    // Data validation
    bool hasGaps(const HistoricalData& data);
    std::vector<int> findOutliers(const HistoricalData& data, double threshold = 3.0);
    bool isMarketHours(const Timestamp& ts);
    
    // Data conversion
    std::string convertSymbolFormat(const std::string& symbol, const std::string& from_provider, const std::string& to_provider);
    std::string convertTimeframeFormat(const std::string& timeframe, const std::string& provider);
    
    // Technical indicators (basic implementations)
    std::vector<double> calculateSMA(const std::vector<Price>& prices, int period);
    std::vector<double> calculateEMA(const std::vector<Price>& prices, int period);
    std::vector<double> calculateRSI(const std::vector<Price>& prices, int period = 14);
    std::vector<double> calculateATR(const HistoricalData& data, int period = 14);
    
    // Market statistics
    double calculateVolatility(const std::vector<Price>& prices, int period = 20);
    double calculateCorrelation(const std::vector<Price>& prices1, const std::vector<Price>& prices2);
    std::map<std::string, double> calculateBasicStats(const std::vector<Price>& prices);
}

} // namespace tradingcore
} // namespace parsec