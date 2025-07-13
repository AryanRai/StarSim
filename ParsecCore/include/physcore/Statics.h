#pragma once

#include <vector>
#include <string>

namespace parsec {

// 3D Vector utilities (shared with other physics domains)
struct Vec3 {
    double x, y, z;
    
    Vec3(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
    
    Vec3 operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
    Vec3 operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
    Vec3 operator*(double scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
    Vec3 operator/(double scalar) const { return Vec3(x / scalar, y / scalar, z / scalar); }
    
    double magnitude() const;
    Vec3 normalized() const;
    double dot(const Vec3& other) const;
    Vec3 cross(const Vec3& other) const;
};

namespace Statics {
    // Force and moment calculations
    Vec3 calculateResultantForce(const std::vector<Vec3>& forces);
    Vec3 calculateResultantMoment(const std::vector<Vec3>& moments, const std::vector<Vec3>& force_positions, const Vec3& reference_point);
    
    // Equilibrium analysis
    bool isInEquilibrium(const std::vector<Vec3>& forces, const std::vector<Vec3>& moments, double tolerance = 1e-6);
    
    // Support reactions
    std::vector<Vec3> calculateSupportReactions(const std::vector<Vec3>& applied_forces, 
                                                const std::vector<Vec3>& force_positions,
                                                const std::vector<Vec3>& support_positions);
    
    // Stress analysis
    double calculateNormalStress(double force, double area);
    double calculateShearStress(double shear_force, double area);
    double calculateBendingStress(double moment, double distance_from_neutral, double moment_of_inertia);
    
    // Beam analysis
    double calculateDeflection(double load, double length, double young_modulus, double moment_of_inertia, const std::string& loading_type);
    double calculateMaxMoment(double load, double length, const std::string& loading_type);
    
    // Truss analysis
    std::vector<double> solveTrussForces(const std::vector<Vec3>& joint_positions,
                                        const std::vector<std::pair<int, int>>& member_connections,
                                        const std::vector<Vec3>& applied_loads,
                                        const std::vector<int>& support_joints);
    
    // Frame analysis
    struct FrameMember {
        int start_joint;
        int end_joint;
        double young_modulus;
        double area;
        double moment_of_inertia;
    };
    
    std::vector<double> solveFrameDisplacements(const std::vector<Vec3>& joint_positions,
                                               const std::vector<FrameMember>& members,
                                               const std::vector<Vec3>& applied_loads,
                                               const std::vector<int>& fixed_joints);
}

} // namespace parsec 