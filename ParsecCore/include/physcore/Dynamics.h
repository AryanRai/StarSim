#pragma once

#include "physcore/Statics.h" // For Vec3
#include <vector>
#include <string>

namespace parsec {

namespace Dynamics {
    // Kinematics
    Vec3 calculateVelocity(const Vec3& initial_velocity, const Vec3& acceleration, double time);
    Vec3 calculatePosition(const Vec3& initial_position, const Vec3& initial_velocity, const Vec3& acceleration, double time);
    
    // Newton's laws
    Vec3 calculateNetForce(double mass, const Vec3& acceleration);
    Vec3 calculateAcceleration(const Vec3& net_force, double mass);
    
    // Rotational dynamics
    double calculateAngularVelocity(double initial_angular_velocity, double angular_acceleration, double time);
    double calculateTorque(double moment_of_inertia, double angular_acceleration);
    double calculateAngularMomentum(double moment_of_inertia, double angular_velocity);
    
    // Energy calculations
    double calculateKineticEnergy(double mass, const Vec3& velocity);
    double calculatePotentialEnergy(double mass, double height, double gravity = 9.81);
    double calculateRotationalKineticEnergy(double moment_of_inertia, double angular_velocity);
    
    // Oscillations
    double calculateSpringForce(double spring_constant, double displacement);
    double calculateDampingForce(double damping_coefficient, double velocity);
    double calculateNaturalFrequency(double spring_constant, double mass);
    double calculateDampedFrequency(double natural_frequency, double damping_ratio);
    
    // Projectile motion
    Vec3 calculateProjectilePosition(const Vec3& initial_position, const Vec3& initial_velocity, double time, double gravity = 9.81);
    double calculateProjectileRange(double initial_velocity, double launch_angle, double gravity = 9.81);
    double calculateMaxHeight(double initial_velocity, double launch_angle, double gravity = 9.81);
    
    // Orbital mechanics
    double calculateOrbitalVelocity(double gravitational_parameter, double orbital_radius);
    double calculateEscapeVelocity(double gravitational_parameter, double radius);
    double calculateOrbitalPeriod(double semi_major_axis, double gravitational_parameter);
    
    // Collision dynamics
    struct CollisionResult {
        Vec3 velocity1_final;
        Vec3 velocity2_final;
        double kinetic_energy_lost;
    };
    
    CollisionResult calculateElasticCollision(double mass1, const Vec3& velocity1, 
                                             double mass2, const Vec3& velocity2);
    CollisionResult calculateInelasticCollision(double mass1, const Vec3& velocity1, 
                                               double mass2, const Vec3& velocity2, 
                                               double coefficient_of_restitution);
    
    // Rigid body dynamics
    struct RigidBody {
        double mass;
        Vec3 position;
        Vec3 velocity;
        Vec3 angular_velocity;
        double moment_of_inertia_xx, moment_of_inertia_yy, moment_of_inertia_zz;
    };
    
    void updateRigidBodyState(RigidBody& body, const Vec3& applied_force, 
                             const Vec3& applied_torque, double time_step);
}

} // namespace parsec 