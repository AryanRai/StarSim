#include "gtest/gtest.h"
#include "parsec/SolverCore.h"
#include "parsec/EquationManager.h"
#include "parsec/ModelConfig.h"
#include "ConfigManager.h"
#include <filesystem> // Requires C++17
#include <memory> // For unique_ptr
#include <cmath> // For std::abs

// Helper function to get the path to the test file (copied from config_manager_tests)
std::string getTestModelFilePath() {
    std::filesystem::path current_path = std::filesystem::current_path(); // Assumes run from build dir
    std::filesystem::path test_file_path = current_path / ".." / "tests" / "test_model.starmodel.json";
    return test_file_path.string();
}

class SolverCoreTest : public ::testing::Test {
protected:
    parsec::ConfigManager configManager;
    std::optional<parsec::ModelConfig> configOpt;
    std::unique_ptr<parsec::EquationManager> eqManager;
    std::unique_ptr<parsec::SolverCore> solverCore;
    parsec::SimulationState initialState;

    void SetUp() override {
        // Load the model configuration
        configOpt = configManager.loadModel(getTestModelFilePath());
        ASSERT_TRUE(configOpt.has_value()) << "Failed to load test model JSON for SolverCore tests.";

        // Create managers
        // Note: EquationManager uses muParser which reads configOpt.value().equations
        eqManager = std::make_unique<parsec::EquationManager>(configOpt.value());
        solverCore = std::make_unique<parsec::SolverCore>(configOpt.value(), *eqManager);

        // Get the initial state
        initialState = solverCore->initializeState();
    }

    void TearDown() override {
        // Clean up unique_ptrs automatically
    }
};

// Test if the initial state is populated correctly
TEST_F(SolverCoreTest, Initialization) {
    ASSERT_NE(solverCore, nullptr);

    // Check if all variables from the config are in the initial state
    ASSERT_EQ(initialState.size(), configOpt.value().variables.size());

    // Check specific initial values based on test_model.starmodel.json
    EXPECT_DOUBLE_EQ(initialState["x"], 0.5);     // Initial dynamic value
    EXPECT_DOUBLE_EQ(initialState["v"], 0.0);     // Initial dynamic value
    EXPECT_DOUBLE_EQ(initialState["m"], 2.0);     // Constant value
    EXPECT_DOUBLE_EQ(initialState["k"], 10.0);    // Constant value
    EXPECT_DOUBLE_EQ(initialState["F_ext"], 0.0); // Input, initialized to 0.0 by SolverCore
}

// Test one step of the Euler solver
TEST_F(SolverCoreTest, EulerStep) {
    ASSERT_NE(solverCore, nullptr);
    ASSERT_TRUE(configOpt.has_value());

    parsec::SimulationState currentState = initialState;
    double dt = solverCore->getTimeStep(); // Get dt from config (0.001)
    EXPECT_DOUBLE_EQ(dt, 0.001);

    // Manually set input for this step (if needed, here F_ext starts at 0)
    // currentState["F_ext"] = 0.0;

    // Expected values BEFORE step:
    double x0 = 0.5;
    double v0 = 0.0;
    double m = 2.0;
    double k = 10.0;
    double F_ext0 = 0.0;

    // Calculate expected derivatives at t=0
    // dv/dt = (-k*x + F_ext) / m = (-10.0 * 0.5 + 0.0) / 2.0 = -5.0 / 2.0 = -2.5
    // dx/dt = v = 0.0
    double expected_dvdt = -2.5;
    double expected_dxdt = 0.0;

    // Calculate expected values AFTER one Euler step:
    // x1 = x0 + dt * dx/dt = 0.5 + 0.001 * 0.0 = 0.5
    // v1 = v0 + dt * dv/dt = 0.0 + 0.001 * (-2.5) = -0.0025
    double expected_x1 = 0.5;
    double expected_v1 = -0.0025;

    // Perform the step
    solverCore->step(currentState);

    // Check the results using EXPECT_NEAR for floating-point comparisons
    EXPECT_NEAR(currentState["x"], expected_x1, 1e-9);
    EXPECT_NEAR(currentState["v"], expected_v1, 1e-9);

    // Verify constants and inputs haven't changed (unless inputs were meant to)
    EXPECT_DOUBLE_EQ(currentState["m"], m);
    EXPECT_DOUBLE_EQ(currentState["k"], k);
    EXPECT_DOUBLE_EQ(currentState["F_ext"], F_ext0); // Input wasn't changed externally
}

// Optional: Test multiple steps or step with non-zero input
TEST_F(SolverCoreTest, EulerStepWithInput) {
    ASSERT_NE(solverCore, nullptr);
    parsec::SimulationState currentState = initialState;
    double dt = solverCore->getTimeStep();

    // Set an input force
    currentState["F_ext"] = 1.0; 

    // Expected values BEFORE step:
    double x0 = 0.5;
    double v0 = 0.0;
    double m = 2.0;
    double k = 10.0;
    double F_ext1 = 1.0;

    // Calculate expected derivatives at t=0 with F_ext=1.0
    // dv/dt = (-k*x + F_ext) / m = (-10.0 * 0.5 + 1.0) / 2.0 = -4.0 / 2.0 = -2.0
    // dx/dt = v = 0.0
    double expected_dvdt = -2.0;
    double expected_dxdt = 0.0;

    // Calculate expected values AFTER one Euler step:
    // x1 = x0 + dt * dx/dt = 0.5 + 0.001 * 0.0 = 0.5
    // v1 = v0 + dt * dv/dt = 0.0 + 0.001 * (-2.0) = -0.002
    double expected_x1 = 0.5;
    double expected_v1 = -0.002;

    // Perform the step
    solverCore->step(currentState);

    // Check the results
    EXPECT_NEAR(currentState["x"], expected_x1, 1e-9);
    EXPECT_NEAR(currentState["v"], expected_v1, 1e-9);
    EXPECT_DOUBLE_EQ(currentState["F_ext"], F_ext1); // Input should remain as we set it
} 