#pragma once

// Modular Math Core - Import all mathematical domains
#include "mathcore/BasicMath.h"
#include "mathcore/LinearAlgebra.h"
#include "mathcore/Statistics.h"
#include "mathcore/NumericalMethods.h"

// This is the main MathCore header that provides access to all mathematical domains.
// You can now import specific domains individually:
//
//   #include "mathcore/BasicMath.h"         // For arithmetic, trigonometry, constants
//   #include "mathcore/LinearAlgebra.h"     // For vectors, matrices, linear algebra algorithms
//   #include "mathcore/Statistics.h"        // For statistics, probability, hypothesis testing
//   #include "mathcore/NumericalMethods.h"  // For integration, differentiation, ODEs, optimization
//
// Or import everything at once:
//
//   #include "mathcore/math.h"              // Imports all mathematical domains

namespace mathcore {
    // All mathematical domains are now available through their respective namespaces:
    // - BasicMath::*         (arithmetic, trigonometry, special functions)
    // - LinearAlgebra::*     (vectors, matrices, decompositions)
    // - Statistics::*        (descriptive stats, distributions, hypothesis tests)
    // - NumericalMethods::*  (root finding, integration, ODEs, optimization)
    
    // Legacy functions for backward compatibility
    inline double add(double a, double b) {
        return BasicMath::add(a, b);
    }

    inline double subtract(double a, double b) {
        return BasicMath::subtract(a, b);
    }

    inline double multiply(double a, double b) {
        return BasicMath::multiply(a, b);
    }

    inline double divide(double a, double b) {
        return BasicMath::divide(a, b);
    }
}

// Example usage:
//
// #include "mathcore/LinearAlgebra.h"
// #include "mathcore/Statistics.h"
//
// auto vector = mathcore::LinearAlgebra::DynamicVector({1.0, 2.0, 3.0});
// double mean_val = mathcore::Statistics::mean({1.0, 2.0, 3.0, 4.0, 5.0});
