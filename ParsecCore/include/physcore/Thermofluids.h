#pragma once

#include <string>

namespace parsec {

namespace Thermofluids {
    // Thermodynamics
    double calculateIdealGasPressure(double density, double temperature, double gas_constant);
    double calculateHeatTransfer(double thermal_conductivity, double area, double temperature_difference, double thickness);
    double calculateConvectiveHeatTransfer(double heat_transfer_coefficient, double area, double temperature_difference);
    double calculateRadiativeHeatTransfer(double emissivity, double stefan_boltzmann_constant, double area, 
                                         double hot_temperature, double cold_temperature);
    
    // Fluid mechanics
    double calculateReynoldsNumber(double density, double velocity, double characteristic_length, double viscosity);
    double calculatePressureDrop(double friction_factor, double length, double diameter, double density, double velocity);
    double calculateBernoulliPressure(double density, double velocity, double height, double gravity = 9.81);
    
    // Flow calculations
    double calculateVolumetricFlowRate(double velocity, double area);
    double calculateMassFlowRate(double density, double velocity, double area);
    double calculateFlowVelocity(double volumetric_flow_rate, double area);
    
    // Heat transfer coefficients
    double calculateNusseltNumber(double reynolds_number, double prandtl_number, const std::string& geometry);
    double calculateHeatTransferCoefficient(double nusselt_number, double thermal_conductivity, double characteristic_length);
    
    // Fluid properties
    double calculateDynamicViscosity(double temperature, const std::string& fluid_type);
    double calculateKinematicViscosity(double dynamic_viscosity, double density);
    double calculatePrandtlNumber(double specific_heat, double viscosity, double thermal_conductivity);
    
    // Advanced thermodynamics
    double calculateEnthalpy(double internal_energy, double pressure, double volume);
    double calculateEntropy(double heat_transfer, double temperature);
    double calculateGibbsFreeEnergy(double enthalpy, double temperature, double entropy);
    
    // Phase change
    double calculateBoilingPoint(double pressure, const std::string& substance);
    double calculateMeltingPoint(double pressure, const std::string& substance);
    double calculateLatentHeat(const std::string& substance, const std::string& phase_change);
    
    // Combustion
    double calculateFlameTemperature(const std::string& fuel, const std::string& oxidizer, 
                                   double fuel_flow_rate, double oxidizer_flow_rate);
    double calculateCombustionEfficiency(double actual_heat_release, double theoretical_heat_release);
    
    // Compressible flow
    double calculateMachNumber(double velocity, double speed_of_sound);
    double calculateChokedFlowRate(double area, double pressure, double temperature, double gas_constant);
    double calculateShockWaveProperties(double mach_number_upstream, double gamma);
}

} // namespace parsec 