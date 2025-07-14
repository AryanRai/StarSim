#pragma once

#include <string>
#include <vector>

namespace parsec {

// Material properties for physics calculations
struct MaterialProperties {
    double density;           // kg/m³
    double young_modulus;     // Pa
    double poisson_ratio;     // dimensionless
    double thermal_conductivity; // W/(m·K)
    double specific_heat;     // J/(kg·K)
    double coefficient_of_expansion; // 1/K
    std::string material_name;
};

namespace MaterialScience {
    // Stress-strain relationships
    double calculateStrain(double stress, double young_modulus);
    double calculateStress(double strain, double young_modulus);
    
    // Material property calculations
    double calculateShearModulus(double young_modulus, double poisson_ratio);
    double calculateBulkModulus(double young_modulus, double poisson_ratio);
    
    // Thermal expansion
    double calculateThermalExpansion(double initial_length, double coefficient_of_expansion, double temperature_change);
    double calculateThermalStress(double young_modulus, double coefficient_of_expansion, double temperature_change);
    
    // Fatigue analysis
    double calculateStressAmplitude(double maximum_stress, double minimum_stress);
    double calculateMeanStress(double maximum_stress, double minimum_stress);
    double calculateStressRatio(double minimum_stress, double maximum_stress);
    
    // Material database
    MaterialProperties getMaterialProperties(const std::string& material_name);
    std::vector<std::string> getAvailableMaterials();
    
    // Advanced material properties
    double calculateCreepStrain(double stress, double temperature, double time, const std::string& material);
    double calculateFractureStrength(const std::string& material, double crack_length, double stress_intensity_factor);
    
    // Composite materials
    struct CompositeProperties {
        double fiber_volume_fraction;
        MaterialProperties fiber_properties;
        MaterialProperties matrix_properties;
    };
    
    MaterialProperties calculateCompositeProperties(const CompositeProperties& composite);
    
    // Crystal structure
    double calculateLatticeParameter(const std::string& crystal_structure, double atomic_radius);
    double calculatePackingFactor(const std::string& crystal_structure);
    
    // Phase diagrams
    double calculateSolidusTemperature(const std::string& alloy, double composition);
    double calculateLiquidusTemperature(const std::string& alloy, double composition);
    
    // Hardness testing
    double convertHardness(double hardness_value, const std::string& from_scale, const std::string& to_scale);
    double calculateIndentationHardness(double applied_force, double indentation_area);
}

} // namespace parsec 