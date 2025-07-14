#pragma once

#include <vector>
#include <map>
#include <cmath>

namespace mathcore {

namespace Statistics {
    // Descriptive statistics
    double mean(const std::vector<double>& data);
    double median(std::vector<double> data); // Note: sorts the data
    double mode(const std::vector<double>& data);
    double variance(const std::vector<double>& data, bool sample = true);
    double standardDeviation(const std::vector<double>& data, bool sample = true);
    double skewness(const std::vector<double>& data);
    double kurtosis(const std::vector<double>& data);
    
    // Robust statistics
    double trimmedMean(std::vector<double> data, double trim_fraction = 0.1);
    double interquartileRange(std::vector<double> data);
    double medianAbsoluteDeviation(std::vector<double> data);
    
    // Percentiles and quantiles
    double percentile(std::vector<double> data, double p);
    double quartile(std::vector<double> data, int q); // q = 1, 2, or 3
    std::vector<double> quantiles(std::vector<double> data, const std::vector<double>& probabilities);
    
    // Distribution fitting
    struct DistributionParameters {
        std::map<std::string, double> parameters;
        double goodness_of_fit;
        std::string distribution_type;
    };
    
    DistributionParameters fitNormal(const std::vector<double>& data);
    DistributionParameters fitExponential(const std::vector<double>& data);
    DistributionParameters fitUniform(const std::vector<double>& data);
    DistributionParameters fitGamma(const std::vector<double>& data);
    
    // Probability distributions - PDF
    namespace PDF {
        double normal(double x, double mean = 0.0, double std_dev = 1.0);
        double uniform(double x, double a = 0.0, double b = 1.0);
        double exponential(double x, double lambda = 1.0);
        double gamma(double x, double shape, double scale);
        double beta(double x, double alpha, double beta);
        double chi_squared(double x, int degrees_of_freedom);
        double t_distribution(double x, int degrees_of_freedom);
        double f_distribution(double x, int df1, int df2);
        double poisson(int k, double lambda);
        double binomial(int k, int n, double p);
    }
    
    // Probability distributions - CDF
    namespace CDF {
        double normal(double x, double mean = 0.0, double std_dev = 1.0);
        double uniform(double x, double a = 0.0, double b = 1.0);
        double exponential(double x, double lambda = 1.0);
        double gamma(double x, double shape, double scale);
        double beta(double x, double alpha, double beta);
        double chi_squared(double x, int degrees_of_freedom);
        double t_distribution(double x, int degrees_of_freedom);
        double f_distribution(double x, int df1, int df2);
        double poisson(int k, double lambda);
        double binomial(int k, int n, double p);
    }
    
    // Inverse CDF (Quantile functions)
    namespace InverseCDF {
        double normal(double p, double mean = 0.0, double std_dev = 1.0);
        double uniform(double p, double a = 0.0, double b = 1.0);
        double exponential(double p, double lambda = 1.0);
        double chi_squared(double p, int degrees_of_freedom);
        double t_distribution(double p, int degrees_of_freedom);
        double f_distribution(double p, int df1, int df2);
    }
    
    // Random number generation
    namespace Random {
        void setSeed(unsigned int seed);
        
        double uniform();
        double uniform(double a, double b);
        double normal(double mean = 0.0, double std_dev = 1.0);
        double exponential(double lambda = 1.0);
        double gamma(double shape, double scale);
        double beta(double alpha, double beta);
        double chi_squared(int degrees_of_freedom);
        
        int poisson(double lambda);
        int binomial(int n, double p);
        int geometric(double p);
        
        std::vector<double> sample(const std::vector<double>& population, size_t sample_size, bool replacement = false);
    }
    
    // Hypothesis testing
    namespace HypothesisTests {
        struct TestResult {
            double test_statistic;
            double p_value;
            double critical_value;
            bool reject_null;
            std::string test_type;
        };
        
        TestResult onesamples_t_test(const std::vector<double>& data, double population_mean, double alpha = 0.05);
        TestResult twosamples_t_test(const std::vector<double>& sample1, const std::vector<double>& sample2, 
                                   bool equal_variances = true, double alpha = 0.05);
        TestResult paired_t_test(const std::vector<double>& before, const std::vector<double>& after, double alpha = 0.05);
        
        TestResult chi_squared_test(const std::vector<int>& observed, const std::vector<int>& expected, double alpha = 0.05);
        TestResult anova(const std::vector<std::vector<double>>& groups, double alpha = 0.05);
        
        TestResult kolmogorov_smirnov_test(const std::vector<double>& data, 
                                         std::function<double(double)> cdf, double alpha = 0.05);
        TestResult anderson_darling_test(const std::vector<double>& data, const std::string& distribution = "normal");
    }
    
    // Correlation and regression
    namespace Correlation {
        double pearson(const std::vector<double>& x, const std::vector<double>& y);
        double spearman(std::vector<double> x, std::vector<double> y);
        double kendall_tau(const std::vector<double>& x, const std::vector<double>& y);
        
        struct RegressionResult {
            std::vector<double> coefficients;
            double r_squared;
            double adjusted_r_squared;
            std::vector<double> residuals;
            std::vector<double> standard_errors;
            std::vector<double> t_statistics;
            std::vector<double> p_values;
        };
        
        RegressionResult linear_regression(const std::vector<double>& x, const std::vector<double>& y);
        RegressionResult multiple_regression(const std::vector<std::vector<double>>& X, const std::vector<double>& y);
        RegressionResult polynomial_regression(const std::vector<double>& x, const std::vector<double>& y, int degree);
    }
    
    // Time series analysis
    namespace TimeSeries {
        std::vector<double> movingAverage(const std::vector<double>& data, int window_size);
        std::vector<double> exponentialSmoothing(const std::vector<double>& data, double alpha);
        
        struct ACFResult {
            std::vector<double> autocorrelations;
            std::vector<double> confidence_intervals;
            int max_lag;
        };
        
        ACFResult autocorrelation(const std::vector<double>& data, int max_lag);
        ACFResult partial_autocorrelation(const std::vector<double>& data, int max_lag);
        
        std::vector<double> differencing(const std::vector<double>& data, int order = 1);
        double adf_test(const std::vector<double>& data); // Augmented Dickey-Fuller test
    }
    
    // Multivariate statistics
    namespace Multivariate {
        std::vector<std::vector<double>> correlation_matrix(const std::vector<std::vector<double>>& data);
        std::vector<std::vector<double>> covariance_matrix(const std::vector<std::vector<double>>& data);
        
        struct PCAResult {
            std::vector<std::vector<double>> principal_components;
            std::vector<double> eigenvalues;
            std::vector<double> explained_variance_ratio;
            std::vector<std::vector<double>> loadings;
        };
        
        PCAResult principal_component_analysis(const std::vector<std::vector<double>>& data);
        
        // Clustering
        struct ClusterResult {
            std::vector<int> cluster_assignments;
            std::vector<std::vector<double>> centroids;
            double within_cluster_sum_of_squares;
        };
        
        ClusterResult k_means(const std::vector<std::vector<double>>& data, int k, int max_iterations = 100);
    }
    
    // Utility functions
    double covariance(const std::vector<double>& x, const std::vector<double>& y);
    std::vector<double> standardize(const std::vector<double>& data);
    std::vector<double> normalize(const std::vector<double>& data, double min_val = 0.0, double max_val = 1.0);
    
    // Outlier detection
    std::vector<bool> outliers_iqr(const std::vector<double>& data, double factor = 1.5);
    std::vector<bool> outliers_zscore(const std::vector<double>& data, double threshold = 3.0);
    std::vector<bool> outliers_modified_zscore(const std::vector<double>& data, double threshold = 3.5);
    
    // Bootstrap methods
    struct BootstrapResult {
        std::vector<double> bootstrap_estimates;
        double mean;
        double standard_error;
        std::pair<double, double> confidence_interval;
    };
    
    BootstrapResult bootstrap(const std::vector<double>& data, 
                            std::function<double(const std::vector<double>&)> statistic,
                            int num_bootstrap_samples = 1000,
                            double confidence_level = 0.95);
}

} // namespace mathcore 