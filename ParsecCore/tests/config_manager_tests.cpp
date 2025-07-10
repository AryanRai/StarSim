#include "gtest/gtest.h"
#include "parsec/ConfigManager.h"
#include "parsec/ModelConfig.h"
#include <fstream>
#include <filesystem> // Requires C++17

// Helper function to get the path to the test file
std::string getTestFilePath(const std::string& filename) {
    // Assumes tests are run from the build directory
    std::filesystem::path current_path = std::filesystem::current_path();
    std::filesystem::path test_file_path = current_path / ".." / "tests" / filename;
    return test_file_path.string();
}

// Test fixture for ConfigManager tests
class ConfigManagerTest : public ::testing::Test {
protected:
    parsec::ConfigManager manager;
    std::string valid_test_file = getTestFilePath("test_model.starmodel.json");
    std::string invalid_json_file = getTestFilePath("invalid_json.starmodel.json");
    std::string missing_file = getTestFilePath("nonexistent_file.json");

    // Create a dummy file with invalid JSON content for testing parse errors
    void SetUp() override {
        std::ofstream ofs(invalid_json_file);
        if (ofs) {
            ofs << "{\"invalid_json\": }"; // Malformed JSON
            ofs.close();
        }
    }

    void TearDown() override {
        // Clean up the dummy invalid JSON file
        std::filesystem::remove(invalid_json_file);
    }
};

// Test loading a valid model file
TEST_F(ConfigManagerTest, LoadValidModel) {
    auto configOpt = manager.loadModel(valid_test_file);
    ASSERT_TRUE(configOpt.has_value());
    auto& config = configOpt.value();

    EXPECT_EQ(config.model_name, "TestSpringMass");
    EXPECT_EQ(config.version, "1.1");
    EXPECT_EQ(config.metadata["author"], "Test User");
    ASSERT_EQ(config.variables.size(), 5);
    EXPECT_EQ(config.variables[0].name, "x");
    EXPECT_EQ(config.variables[0].type, parsec::VariableType::DYNAMIC);
    ASSERT_TRUE(std::holds_alternative<double>(config.variables[0].value));
    EXPECT_DOUBLE_EQ(std::get<double>(config.variables[0].value), 0.5);

    EXPECT_EQ(config.variables[2].name, "m");
    EXPECT_EQ(config.variables[2].type, parsec::VariableType::CONSTANT);
    ASSERT_TRUE(std::holds_alternative<double>(config.variables[2].value));
    EXPECT_DOUBLE_EQ(std::get<double>(config.variables[2].value), 2.0);

    EXPECT_EQ(config.variables[4].name, "F_ext");
    EXPECT_EQ(config.variables[4].type, parsec::VariableType::INPUT);
    ASSERT_TRUE(std::holds_alternative<std::string>(config.variables[4].value));
    EXPECT_EQ(std::get<std::string>(config.variables[4].value), "external_force_stream");

    ASSERT_EQ(config.equations.size(), 2);
    EXPECT_EQ(config.equations[0], "d(v)/dt = (-k * x + F_ext) / m");
    EXPECT_EQ(config.equations[1], "d(x)/dt = v");

    EXPECT_EQ(config.solver.method, "Euler");
    EXPECT_DOUBLE_EQ(config.solver.dt, 0.001);
    EXPECT_DOUBLE_EQ(config.solver.duration, 5.0);

    ASSERT_EQ(config.outputs.size(), 2);
    EXPECT_EQ(config.outputs[0], "x");
}

// Test loading a non-existent file
TEST_F(ConfigManagerTest, LoadMissingFile) {
    auto configOpt = manager.loadModel(missing_file);
    EXPECT_FALSE(configOpt.has_value());
}

// Test loading a file with invalid JSON format
TEST_F(ConfigManagerTest, LoadInvalidJsonFile) {
    auto configOpt = manager.loadModel(invalid_json_file);
    EXPECT_FALSE(configOpt.has_value());
} 