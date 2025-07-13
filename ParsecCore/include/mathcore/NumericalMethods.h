#pragma once

#include <vector>
#include <functional>
#include <utility>

namespace mathcore {

namespace NumericalMethods {
    // Root finding methods
    namespace RootFinding {
        struct RootResult {
            double root;
            double function_value;
            int iterations;
            bool converged;
            std::string method;
        };
        
        RootResult bisection(std::function<double(double)> f, double a, double b, 
                           double tolerance = 1e-6, int max_iterations = 100);
        
        RootResult newton_raphson(std::function<double(double)> f, std::function<double(double)> df, 
                                double initial_guess, double tolerance = 1e-6, int max_iterations = 100);
        
        RootResult secant(std::function<double(double)> f, double x0, double x1, 
                        double tolerance = 1e-6, int max_iterations = 100);
        
        RootResult false_position(std::function<double(double)> f, double a, double b, 
                                double tolerance = 1e-6, int max_iterations = 100);
        
        RootResult brent(std::function<double(double)> f, double a, double b, 
                       double tolerance = 1e-6, int max_iterations = 100);
        
        // For polynomial roots
        std::vector<std::pair<double, double>> polynomial_roots(const std::vector<double>& coefficients);
    }
    
    // Numerical integration (quadrature)
    namespace Integration {
        struct IntegrationResult {
            double value;
            double error_estimate;
            int function_evaluations;
            bool converged;
            std::string method;
        };
        
        // Basic quadrature rules
        double rectangular(std::function<double(double)> f, double a, double b, int n);
        double trapezoidal(std::function<double(double)> f, double a, double b, int n);
        double simpson(std::function<double(double)> f, double a, double b, int n);
        double simpson_3_8(std::function<double(double)> f, double a, double b, int n);
        
        // Gaussian quadrature
        double gauss_legendre(std::function<double(double)> f, double a, double b, int n);
        double gauss_laguerre(std::function<double(double)> f, int n); // For [0, inf)
        double gauss_hermite(std::function<double(double)> f, int n);  // For (-inf, inf)
        
        // Adaptive methods
        IntegrationResult adaptive_simpson(std::function<double(double)> f, double a, double b, 
                                         double tolerance = 1e-6, int max_recursion = 20);
        
        IntegrationResult adaptive_gauss_kronrod(std::function<double(double)> f, double a, double b, 
                                               double tolerance = 1e-6, int max_subdivisions = 100);
        
        // Monte Carlo integration
        IntegrationResult monte_carlo(std::function<double(double)> f, double a, double b, 
                                    int num_samples = 100000);
        
        // Multi-dimensional integration
        double simpson_2d(std::function<double(double, double)> f, 
                         double x_min, double x_max, int nx,
                         double y_min, double y_max, int ny);
        
        IntegrationResult monte_carlo_nd(std::function<double(const std::vector<double>&)> f,
                                       const std::vector<double>& lower_bounds,
                                       const std::vector<double>& upper_bounds,
                                       int num_samples = 100000);
    }
    
    // Numerical differentiation
    namespace Differentiation {
        // First derivatives
        double forward_difference(std::function<double(double)> f, double x, double h = 1e-5);
        double backward_difference(std::function<double(double)> f, double x, double h = 1e-5);
        double central_difference(std::function<double(double)> f, double x, double h = 1e-5);
        
        // Higher-order derivatives
        double second_derivative(std::function<double(double)> f, double x, double h = 1e-5);
        double nth_derivative(std::function<double(double)> f, double x, int n, double h = 1e-5);
        
        // Gradient and Hessian for multivariable functions
        std::vector<double> gradient(std::function<double(const std::vector<double>&)> f,
                                   const std::vector<double>& x, double h = 1e-5);
        
        std::vector<std::vector<double>> hessian(std::function<double(const std::vector<double>&)> f,
                                                const std::vector<double>& x, double h = 1e-5);
        
        // Automatic differentiation (dual numbers)
        class DualNumber {
        public:
            double real, dual;
            
            DualNumber(double r = 0.0, double d = 0.0) : real(r), dual(d) {}
            
            DualNumber operator+(const DualNumber& other) const;
            DualNumber operator-(const DualNumber& other) const;
            DualNumber operator*(const DualNumber& other) const;
            DualNumber operator/(const DualNumber& other) const;
            
            DualNumber sin() const;
            DualNumber cos() const;
            DualNumber exp() const;
            DualNumber log() const;
            DualNumber pow(double exponent) const;
        };
    }
    
    // Ordinary Differential Equations (ODEs)
    namespace ODE {
        // Single ODE: dy/dx = f(x, y)
        struct ODEResult {
            std::vector<double> x_values;
            std::vector<double> y_values;
            bool success;
            std::string method;
        };
        
        ODEResult euler(std::function<double(double, double)> f, double x0, double y0, 
                       double x_final, double step_size);
        
        ODEResult runge_kutta_4(std::function<double(double, double)> f, double x0, double y0, 
                               double x_final, double step_size);
        
        ODEResult adams_bashforth(std::function<double(double, double)> f, double x0, double y0, 
                                 double x_final, double step_size, int order = 4);
        
        // System of ODEs: dy/dx = f(x, y) where y is a vector
        struct SystemODEResult {
            std::vector<double> x_values;
            std::vector<std::vector<double>> y_values;
            bool success;
            std::string method;
        };
        
        SystemODEResult euler_system(std::function<std::vector<double>(double, const std::vector<double>&)> f,
                                   double x0, const std::vector<double>& y0, 
                                   double x_final, double step_size);
        
        SystemODEResult runge_kutta_4_system(std::function<std::vector<double>(double, const std::vector<double>&)> f,
                                            double x0, const std::vector<double>& y0, 
                                            double x_final, double step_size);
        
        // Adaptive step size methods
        SystemODEResult runge_kutta_45(std::function<std::vector<double>(double, const std::vector<double>&)> f,
                                     double x0, const std::vector<double>& y0, 
                                     double x_final, double tolerance = 1e-6);
    }
    
    // Partial Differential Equations (PDEs) - Basic methods
    namespace PDE {
        // 1D Heat equation: du/dt = alpha * d²u/dx²
        std::vector<std::vector<double>> heat_equation_explicit(
            double alpha, double length, double time_final,
            int nx, int nt,
            std::function<double(double)> initial_condition,
            std::function<double(double)> boundary_left,
            std::function<double(double)> boundary_right
        );
        
        // 1D Wave equation: d²u/dt² = c² * d²u/dx²
        std::vector<std::vector<double>> wave_equation_explicit(
            double c, double length, double time_final,
            int nx, int nt,
            std::function<double(double)> initial_position,
            std::function<double(double)> initial_velocity,
            std::function<double(double)> boundary_left,
            std::function<double(double)> boundary_right
        );
        
        // 2D Laplace equation: d²u/dx² + d²u/dy² = 0
        std::vector<std::vector<double>> laplace_equation_jacobi(
            double x_min, double x_max, double y_min, double y_max,
            int nx, int ny,
            std::function<double(double, double)> boundary_condition,
            double tolerance = 1e-6, int max_iterations = 10000
        );
    }
    
    // Optimization
    namespace Optimization {
        struct OptimizationResult {
            std::vector<double> x_optimal;
            double f_optimal;
            int iterations;
            bool converged;
            std::string method;
        };
        
        // Unconstrained optimization
        OptimizationResult golden_section_search(std::function<double(double)> f, double a, double b, 
                                                double tolerance = 1e-6);
        
        OptimizationResult gradient_descent(std::function<double(const std::vector<double>&)> f,
                                          const std::vector<double>& x0,
                                          double learning_rate = 0.01, double tolerance = 1e-6, 
                                          int max_iterations = 1000);
        
        OptimizationResult newton_method(std::function<double(const std::vector<double>&)> f,
                                       const std::vector<double>& x0,
                                       double tolerance = 1e-6, int max_iterations = 100);
        
        OptimizationResult bfgs(std::function<double(const std::vector<double>&)> f,
                              const std::vector<double>& x0,
                              double tolerance = 1e-6, int max_iterations = 1000);
        
        OptimizationResult nelder_mead(std::function<double(const std::vector<double>&)> f,
                                      const std::vector<double>& x0,
                                      double tolerance = 1e-6, int max_iterations = 1000);
        
        // Constrained optimization (simple penalty method)
        OptimizationResult penalty_method(std::function<double(const std::vector<double>&)> objective,
                                         std::function<double(const std::vector<double>&)> constraint,
                                         const std::vector<double>& x0,
                                         double penalty_weight = 1.0,
                                         double tolerance = 1e-6, int max_iterations = 1000);
    }
    
    // Interpolation and approximation
    namespace Interpolation {
        // Polynomial interpolation
        std::function<double(double)> lagrange_interpolation(const std::vector<double>& x_points,
                                                           const std::vector<double>& y_points);
        
        std::function<double(double)> newton_interpolation(const std::vector<double>& x_points,
                                                         const std::vector<double>& y_points);
        
        // Spline interpolation
        std::function<double(double)> cubic_spline(const std::vector<double>& x_points,
                                                 const std::vector<double>& y_points);
        
        // 2D interpolation
        std::function<double(double, double)> bilinear_interpolation(
            const std::vector<double>& x_points,
            const std::vector<double>& y_points,
            const std::vector<std::vector<double>>& z_values
        );
        
        // Rational approximation (Padé approximants)
        struct RationalApproximation {
            std::vector<double> numerator_coeffs;
            std::vector<double> denominator_coeffs;
        };
        
        RationalApproximation pade_approximation(const std::vector<double>& series_coefficients,
                                                int numerator_degree, int denominator_degree);
    }
    
    // Special numerical algorithms
    namespace SpecialAlgorithms {
        // Fast Fourier Transform
        std::vector<std::pair<double, double>> fft(const std::vector<double>& real_input);
        std::vector<std::pair<double, double>> fft(const std::vector<std::pair<double, double>>& complex_input);
        
        // Convolution
        std::vector<double> convolution(const std::vector<double>& signal1, const std::vector<double>& signal2);
        
        // Matrix algorithms
        std::vector<std::vector<double>> matrix_power(const std::vector<std::vector<double>>& matrix, int power);
        double matrix_trace(const std::vector<std::vector<double>>& matrix);
        
        // Continued fractions
        std::vector<int> continued_fraction(double x, int max_terms = 20);
        double evaluate_continued_fraction(const std::vector<int>& cf_terms);
    }
}

} // namespace mathcore 