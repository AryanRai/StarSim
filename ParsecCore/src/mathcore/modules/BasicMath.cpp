#include "mathcore/modules/BasicMath.h"
#include <algorithm>
#include <numeric>
#include <cmath>

namespace mathcore {
namespace BasicMath {

// Special functions implementations
double factorial(int n) {
    if (n < 0) return 0.0;
    if (n <= 1) return 1.0;
    
    double result = 1.0;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

double gamma(double x) {
    // Simplified gamma function approximation using Stirling's formula
    if (x <= 0) return 0.0;
    if (x < 1) return gamma(x + 1) / x;
    
    // For x >= 1, use Stirling's approximation
    return std::sqrt(2 * PI / x) * std::pow(x / E, x);
}

double beta(double a, double b) {
    // Beta function: B(a,b) = Γ(a)Γ(b)/Γ(a+b)
    if (a <= 0 || b <= 0) return 0.0;
    return gamma(a) * gamma(b) / gamma(a + b);
}

double erf(double x) {
    // Error function approximation using Abramowitz and Stegun
    if (x == 0.0) return 0.0;
    
    double sign = (x >= 0) ? 1.0 : -1.0;
    x = std::abs(x);
    
    // Constants for approximation
    double a1 = 0.254829592;
    double a2 = -0.284496736;
    double a3 = 1.421413741;
    double a4 = -1.453152027;
    double a5 = 1.061405429;
    double p = 0.3275911;
    
    double t = 1.0 / (1.0 + p * x);
    double y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * std::exp(-x * x);
    
    return sign * y;
}

double erfc(double x) {
    // Complementary error function
    return 1.0 - erf(x);
}

// Combinatorics
double combination(int n, int k) {
    if (k < 0 || k > n) return 0.0;
    if (k == 0 || k == n) return 1.0;
    
    // Use symmetry property: C(n,k) = C(n,n-k)
    if (k > n - k) k = n - k;
    
    double result = 1.0;
    for (int i = 0; i < k; ++i) {
        result = result * (n - i) / (i + 1);
    }
    return result;
}

double permutation(int n, int k) {
    if (k < 0 || k > n) return 0.0;
    if (k == 0) return 1.0;
    
    double result = 1.0;
    for (int i = 0; i < k; ++i) {
        result *= (n - i);
    }
    return result;
}

// Number theory
int gcd(int a, int b) {
    a = std::abs(a);
    b = std::abs(b);
    
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int lcm(int a, int b) {
    if (a == 0 || b == 0) return 0;
    return std::abs(a * b) / gcd(a, b);
}

bool isPrime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return false;
        }
    }
    return true;
}

std::vector<int> primeFactors(int n) {
    std::vector<int> factors;
    
    if (n <= 1) return factors;
    
    // Handle factor 2
    while (n % 2 == 0) {
        factors.push_back(2);
        n /= 2;
    }
    
    // Handle odd factors
    for (int i = 3; i * i <= n; i += 2) {
        while (n % i == 0) {
            factors.push_back(i);
            n /= i;
        }
    }
    
    // If n is a prime greater than 2
    if (n > 2) {
        factors.push_back(n);
    }
    
    return factors;
}

} // namespace BasicMath
} // namespace mathcore