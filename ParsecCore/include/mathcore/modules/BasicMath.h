#pragma once

#include <cmath>
#include <algorithm>
#include <vector>

namespace mathcore {

namespace BasicMath {
    // Basic arithmetic operations (header-only for performance)
    inline double add(double a, double b) {
        return a + b;
    }

    inline double subtract(double a, double b) {
        return a - b;
    }

    inline double multiply(double a, double b) {
        return a * b;
    }

    inline double divide(double a, double b) {
        return (b != 0.0) ? a / b : 0.0;  // Prevent divide-by-zero
    }

    // Power and exponential functions
    inline double power(double base, double exponent) {
        return std::pow(base, exponent);
    }

    inline double sqrt(double value) {
        return std::sqrt(value);
    }

    inline double cbrt(double value) {
        return std::cbrt(value);
    }

    inline double exp(double value) {
        return std::exp(value);
    }

    inline double ln(double value) {
        return std::log(value);
    }

    inline double log10(double value) {
        return std::log10(value);
    }

    inline double log2(double value) {
        return std::log2(value);
    }

    // Trigonometric functions
    inline double sin(double angle) {
        return std::sin(angle);
    }

    inline double cos(double angle) {
        return std::cos(angle);
    }

    inline double tan(double angle) {
        return std::tan(angle);
    }

    inline double asin(double value) {
        return std::asin(value);
    }

    inline double acos(double value) {
        return std::acos(value);
    }

    inline double atan(double value) {
        return std::atan(value);
    }

    inline double atan2(double y, double x) {
        return std::atan2(y, x);
    }

    // Hyperbolic functions
    inline double sinh(double value) {
        return std::sinh(value);
    }

    inline double cosh(double value) {
        return std::cosh(value);
    }

    inline double tanh(double value) {
        return std::tanh(value);
    }

    // Rounding and comparison functions
    inline double abs(double value) {
        return std::abs(value);
    }

    inline double floor(double value) {
        return std::floor(value);
    }

    inline double ceil(double value) {
        return std::ceil(value);
    }

    inline double round(double value) {
        return std::round(value);
    }

    inline double min(double a, double b) {
        return std::min(a, b);
    }

    inline double max(double a, double b) {
        return std::max(a, b);
    }

    inline double clamp(double value, double min_val, double max_val) {
        return std::clamp(value, min_val, max_val);
    }

    // Modular arithmetic
    inline double mod(double a, double b) {
        return std::fmod(a, b);
    }

    inline double remainder(double a, double b) {
        return std::remainder(a, b);
    }

    // Sign and comparison utilities
    inline int sign(double value) {
        return (value > 0) ? 1 : (value < 0) ? -1 : 0;
    }

    inline bool isEqual(double a, double b, double tolerance = 1e-9) {
        return std::abs(a - b) < tolerance;
    }

    inline bool isZero(double value, double tolerance = 1e-9) {
        return std::abs(value) < tolerance;
    }

    // Constants
    constexpr double PI = 3.14159265358979323846;
    constexpr double E = 2.71828182845904523536;
    constexpr double SQRT_2 = 1.41421356237309504880;
    constexpr double SQRT_3 = 1.73205080756887729353;
    constexpr double GOLDEN_RATIO = 1.61803398874989484820;
    constexpr double EULER_GAMMA = 0.57721566490153286060;

    // Degree/Radian conversion
    constexpr double DEG_TO_RAD = PI / 180.0;
    constexpr double RAD_TO_DEG = 180.0 / PI;

    inline double degreesToRadians(double degrees) {
        return degrees * DEG_TO_RAD;
    }

    inline double radiansToDegrees(double radians) {
        return radians * RAD_TO_DEG;
    }

    // Special functions
    double factorial(int n);
    double gamma(double x);
    double beta(double a, double b);
    double erf(double x);
    double erfc(double x);

    // Combinatorics
    double combination(int n, int k);
    double permutation(int n, int k);

    // Number theory
    int gcd(int a, int b);
    int lcm(int a, int b);
    bool isPrime(int n);
    std::vector<int> primeFactors(int n);
}

} // namespace mathcore 