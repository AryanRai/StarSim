#pragma once

// Modular Physics Core - Import all physics domains
#include "physcore/Statics.h"
#include "physcore/Dynamics.h"
#include "physcore/Thermofluids.h"
#include "physcore/MaterialScience.h"
#include "physcore/Controls.h"
#include "physcore/PhysicsConstants.h"
#include "physcore/PhysicsUtils.h"

// This is the main PhysCore header that provides access to all physics domains.
// You can now import specific domains individually:
//
//   #include "physcore/Statics.h"          // For structural analysis
//   #include "physcore/Dynamics.h"         // For motion and energy calculations
//   #include "physcore/Thermofluids.h"     // For heat transfer and fluid mechanics
//   #include "physcore/MaterialScience.h"  // For material properties and analysis
//   #include "physcore/Controls.h"         // For control systems and PID
//   #include "physcore/PhysicsConstants.h" // For fundamental constants
//   #include "physcore/PhysicsUtils.h"     // For utilities and numerical methods
//
// Or import everything at once:
//
//   #include "physcore/PhysCore.h"         // Imports all physics domains

namespace parsec {
    // All physics domains are now available through their respective namespaces:
    // - Statics::*
    // - Dynamics::*
    // - Thermofluids::*
    // - MaterialScience::*
    // - Controls::*
    // - PhysicsConstants::*
    // - PhysicsUtils::*
    
    // Common data structures (Vec3, MaterialProperties) are defined in their respective domain headers
}

// Example usage:
//
// #include "physcore/Dynamics.h"
// #include "physcore/PhysicsConstants.h"
//
// double energy = parsec::Dynamics::calculateKineticEnergy(mass, velocity);
// double gravity = parsec::PhysicsConstants::GRAVITY_EARTH; 