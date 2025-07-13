#pragma once

#include "mlcore/MLDataStructures.h"
#include "parsec/ModelConfig.h"
#include <vector>
#include <memory>
#include <functional>

namespace parsec {

namespace MLAlgorithms {

class DataCollector {
public:
    explicit DataCollector(const ModelConfig& config);
    
    // Core data collection functionality
    void collectData(const SimulationState& currentState, double timestamp);
    void setCollectionInterval(double interval_seconds);
    void setMaxDataPoints(size_t max_points);
    
    // Data access and management
    const std::vector<MLDataPoint>& getCollectedData() const;
    std::vector<MLDataPoint> getDataInTimeRange(double start_time, double end_time) const;
    void clearCollectedData();
    
    // Data preprocessing and filtering
    void enablePreprocessing(const MLPreprocessingConfig& config);
    void addDataFilter(std::function<bool(const MLDataPoint&)> filter);
    void addDataTransform(std::function<MLDataPoint(const MLDataPoint&)> transform);
    
    // Feature engineering
    void enableFeatureEngineering(bool enable_derivatives = true, 
                                 bool enable_rolling_stats = true,
                                 int window_size = 10);
    
    // Real-time data streaming
    struct StreamingConfig {
        bool enabled = false;
        double buffer_duration = 60.0;  // seconds
        bool auto_cleanup = true;
        size_t max_buffer_size = 10000;
    };
    
    void enableStreaming(const StreamingConfig& config);
    std::vector<MLDataPoint> getRecentData(double duration_seconds) const;
    
    // Data quality monitoring
    struct DataQualityMetrics {
        double completeness_ratio = 1.0;  // Fraction of non-missing values
        double consistency_score = 1.0;   // Measure of data consistency
        int outlier_count = 0;
        double sampling_rate_deviation = 0.0;
        std::map<std::string, double> feature_quality_scores;
    };
    
    DataQualityMetrics assessDataQuality() const;
    void enableQualityMonitoring(bool enable, double monitoring_interval = 10.0);
    
    // Data export and persistence
    bool exportToCSV(const std::string& filepath) const;
    bool exportToJSON(const std::string& filepath) const;
    bool saveToFile(const std::string& filepath) const;
    bool loadFromFile(const std::string& filepath);
    
    // Callbacks for data events
    void setDataCollectedCallback(std::function<void(const MLDataPoint&)> callback);
    void setQualityCallback(std::function<void(const DataQualityMetrics&)> callback);
    void setBufferFullCallback(std::function<void(size_t)> callback);
    
    // Configuration
    struct CollectionConfig {
        std::vector<std::string> input_variables;
        std::vector<std::string> output_variables;
        std::vector<std::string> target_variables;
        double sampling_interval = 0.1;
        bool adaptive_sampling = false;
        double min_change_threshold = 0.001;
    };
    
    void setCollectionConfig(const CollectionConfig& config);
    CollectionConfig getCollectionConfig() const;
    
private:
    const ModelConfig& modelConfig_;
    std::vector<MLDataPoint> collectedData_;
    CollectionConfig config_;
    StreamingConfig streamingConfig_;
    MLPreprocessingConfig preprocessingConfig_;
    
    // Configuration
    double dataCollectionInterval_ = 0.1;
    size_t maxDataPoints_ = 10000;
    double lastCollectionTime_ = 0.0;
    bool preprocessingEnabled_ = false;
    bool qualityMonitoringEnabled_ = false;
    double qualityMonitoringInterval_ = 10.0;
    double lastQualityCheck_ = 0.0;
    
    // Callbacks
    std::function<void(const MLDataPoint&)> dataCollectedCallback_;
    std::function<void(const DataQualityMetrics&)> qualityCallback_;
    std::function<void(size_t)> bufferFullCallback_;
    
    // Data filters and transforms
    std::vector<std::function<bool(const MLDataPoint&)>> dataFilters_;
    std::vector<std::function<MLDataPoint(const MLDataPoint&)>> dataTransforms_;
    
    // Feature engineering state
    bool featureEngineeringEnabled_ = false;
    bool enableDerivatives_ = false;
    bool enableRollingStats_ = false;
    int rollingWindowSize_ = 10;
    std::vector<MLDataPoint> recentData_;  // For derivative calculation
    
    // Internal helper methods
    MLDataPoint createDataPoint(const SimulationState& state, double timestamp) const;
    bool shouldCollectData(double currentTime) const;
    MLDataPoint preprocessDataPoint(const MLDataPoint& point) const;
    void applyFeatureEngineering(MLDataPoint& point);
    void manageBufferSize();
    void checkDataQuality();
    
    // Preprocessing implementations
    MLDataPoint normalizeFeatures(const MLDataPoint& point) const;
    MLDataPoint standardizeFeatures(const MLDataPoint& point) const;
    bool isOutlier(const MLDataPoint& point) const;
    
    // Feature engineering implementations
    void addDerivativeFeatures(MLDataPoint& point);
    void addRollingStatistics(MLDataPoint& point);
    
    // Utility methods
    std::vector<std::string> parseFeatureList(const std::string& feature_string) const;
    bool validateDataPoint(const MLDataPoint& point) const;
};

} // namespace MLAlgorithms

} // namespace parsec 