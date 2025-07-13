#pragma once

#include "physcore/Statics.h" // For Vec3
#include <vector>
#include <string>
#include <functional>

namespace parsec {

namespace PhysicsUtils {
    // Unit conversions
    double convertTemperature(double value, const std::string& from_unit, const std::string& to_unit);
    double convertPressure(double value, const std::string& from_unit, const std::string& to_unit);
    double convertLength(double value, const std::string& from_unit, const std::string& to_unit);
    double convertForce(double value, const std::string& from_unit, const std::string& to_unit);
    double convertEnergy(double value, const std::string& from_unit, const std::string& to_unit);
    double convertPower(double value, const std::string& from_unit, const std::string& to_unit);
    double convertMass(double value, const std::string& from_unit, const std::string& to_unit);
    
    // Numerical methods for physics
    double secantMethod(std::function<double(double)> func, double x0, double x1, double tolerance = 1e-6, int max_iterations = 100);
    double bisectionMethod(std::function<double(double)> func, double a, double b, double tolerance = 1e-6, int max_iterations = 100);
    double newtonRaphsonMethod(std::function<double(double)> func, std::function<double(double)> derivative, 
                              double initial_guess, double tolerance = 1e-6, int max_iterations = 100);
    
    // Integration methods
    double trapezoidalIntegration(std::function<double(double)> func, double a, double b, int num_intervals);
    double simpsonIntegration(std::function<double(double)> func, double a, double b, int num_intervals);
    double gaussianQuadrature(std::function<double(double)> func, double a, double b, int num_points);
    
    // Differentiation methods
    double numericalDerivative(std::function<double(double)> func, double x, double h = 1e-8);
    double centralDifference(std::function<double(double)> func, double x, double h = 1e-8);
    double forwardDifference(std::function<double(double)> func, double x, double h = 1e-8);
    double backwardDifference(std::function<double(double)> func, double x, double h = 1e-8);
    
    // Vector operations
    double angleBetweenVectors(const Vec3& a, const Vec3& b);
    Vec3 projectVector(const Vec3& vector, const Vec3& onto);
    Vec3 reflectVector(const Vec3& incident, const Vec3& normal);
    Vec3 rotateVector(const Vec3& vector, const Vec3& axis, double angle);
    
    // Statistical analysis for experimental data
    double calculateMean(const std::vector<double>& data);
    double calculateStandardDeviation(const std::vector<double>& data);
    double calculateVariance(const std::vector<double>& data);
    double calculateCorrelationCoefficient(const std::vector<double>& x, const std::vector<double>& y);
    
    // Interpolation methods
    double linearInterpolation(double x, double x1, double y1, double x2, double y2);
    double polynomialInterpolation(const std::vector<double>& x_points, const std::vector<double>& y_points, double x);
    double splineInterpolation(const std::vector<double>& x_points, const std::vector<double>& y_points, double x);
    
    // Curve fitting
    struct LinearFitResult {
        double slope;
        double intercept;
        double correlation_coefficient;
        double standard_error;
    };
    
    LinearFitResult linearLeastSquares(const std::vector<double>& x, const std::vector<double>& y);
    std::vector<double> polynomialFit(const std::vector<double>& x, const std::vector<double>& y, int degree);
    
    // Error analysis
    double propagateUncertaintyAddition(const std::vector<double>& uncertainties);
    double propagateUncertaintyMultiplication(const std::vector<double>& values, const std::vector<double>& uncertainties);
    double calculatePercentError(double experimental_value, double theoretical_value);
    double calculateRelativeError(double experimental_value, double theoretical_value);
    
    // Signal processing for physics data
    std::vector<double> movingAverage(const std::vector<double>& data, int window_size);
    std::vector<double> savitzkyGolayFilter(const std::vector<double>& data, int window_size, int polynomial_order);
    
    // Coordinate transformations
    struct SphericalCoordinates {
        double r, theta, phi;
    };
    
    struct CylindricalCoordinates {
        double rho, phi, z;
    };
    
    SphericalCoordinates cartesianToSpherical(const Vec3& cartesian);
    Vec3 sphericalToCartesian(const SphericalCoordinates& spherical);
    CylindricalCoordinates cartesianToCylindrical(const Vec3& cartesian);
    Vec3 cylindricalToCartesian(const CylindricalCoordinates& cylindrical);
    
    // Physics-specific utilities
    double calculateResonanceFrequency(double inductance, double capacitance);
    double calculateWaveSpeed(double frequency, double wavelength);
    double calculateDopplerShift(double source_frequency, double source_velocity, double observer_velocity, double wave_speed);
    
    // Data validation
    bool isPhysicallyReasonable(double value, const std::string& physical_quantity, const std::string& units = "");
    std::vector<double> removeOutliers(const std::vector<double>& data, double threshold_sigma = 2.0);
    
    // Constants and unit systems
    struct UnitSystem {
        std::string length_unit;
        std::string mass_unit;
        std::string time_unit;
        std::string temperature_unit;
    };
    
    double convertToSI(double value, const std::string& quantity, const UnitSystem& from_system);
    double convertFromSI(double value, const std::string& quantity, const UnitSystem& to_system);
}

} // namespace parsec 