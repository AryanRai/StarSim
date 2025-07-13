#pragma once

namespace parsec {

// Physics utilities and constants
namespace PhysicsConstants {
    constexpr double GRAVITY_EARTH = 9.80665;        // m/s²
    constexpr double STEFAN_BOLTZMANN = 5.67037e-8;  // W/(m²·K⁴)
    constexpr double GAS_CONSTANT_UNIVERSAL = 8.314; // J/(mol·K)
    constexpr double BOLTZMANN_CONSTANT = 1.38065e-23; // J/K
    constexpr double AVOGADRO_NUMBER = 6.02214e23;   // 1/mol
    constexpr double SPEED_OF_LIGHT = 299792458;     // m/s
    constexpr double PLANCK_CONSTANT = 6.62607e-34;  // J·s
    constexpr double PI = 3.14159265358979323846;
    
    // Additional fundamental constants
    constexpr double ELECTRON_CHARGE = 1.60218e-19;  // C
    constexpr double ELECTRON_MASS = 9.10938e-31;    // kg
    constexpr double PROTON_MASS = 1.67262e-27;      // kg
    constexpr double NEUTRON_MASS = 1.67493e-27;     // kg
    
    // Vacuum properties
    constexpr double VACUUM_PERMEABILITY = 4.0e-7 * PI; // H/m
    constexpr double VACUUM_PERMITTIVITY = 8.85418e-12; // F/m
    constexpr double VACUUM_IMPEDANCE = 376.730313668;  // Ω
    
    // Atomic units
    constexpr double BOHR_RADIUS = 5.29177e-11;      // m
    constexpr double HARTREE_ENERGY = 4.35974e-18;   // J
    constexpr double ATOMIC_MASS_UNIT = 1.66054e-27; // kg
    
    // Standard conditions
    constexpr double STANDARD_PRESSURE = 101325;     // Pa
    constexpr double STANDARD_TEMPERATURE = 273.15;  // K
    constexpr double STANDARD_DENSITY_AIR = 1.225;   // kg/m³
    constexpr double STANDARD_DENSITY_WATER = 1000;  // kg/m³
    
    // Conversion factors
    constexpr double DEGREES_TO_RADIANS = PI / 180.0;
    constexpr double RADIANS_TO_DEGREES = 180.0 / PI;
    constexpr double KELVIN_TO_CELSIUS = 273.15;
    constexpr double PSI_TO_PASCAL = 6894.76;
    constexpr double ATM_TO_PASCAL = 101325;
    constexpr double BAR_TO_PASCAL = 100000;
    
    // Material property constants
    constexpr double STEEL_DENSITY = 7850;           // kg/m³
    constexpr double ALUMINUM_DENSITY = 2700;        // kg/m³
    constexpr double COPPER_DENSITY = 8960;          // kg/m³
    constexpr double CONCRETE_DENSITY = 2400;        // kg/m³
    
    // Fluid property constants
    constexpr double WATER_DYNAMIC_VISCOSITY = 1e-3; // Pa·s at 20°C
    constexpr double AIR_DYNAMIC_VISCOSITY = 1.8e-5; // Pa·s at 20°C
    constexpr double WATER_THERMAL_CONDUCTIVITY = 0.6; // W/(m·K)
    constexpr double AIR_THERMAL_CONDUCTIVITY = 0.025; // W/(m·K)
}

} // namespace parsec 