#include "gtest/gtest.h"
#include "mathcore/math.h" // Include the header for the code under test

// Test fixture for MathCore tests (optional, but good practice)
class MathCoreTest : public ::testing::Test {
protected:
    // Setup code here (if needed)
    void SetUp() override {}

    // Teardown code here (if needed)
    void TearDown() override {}
};

// Test case for the add function
TEST_F(MathCoreTest, Addition) {
    EXPECT_FLOAT_EQ(5.0f, mathcore::add(2.5f, 2.5f));
    EXPECT_FLOAT_EQ(0.0f, mathcore::add(0.0f, 0.0f));
    EXPECT_FLOAT_EQ(-5.0f, mathcore::add(-2.5f, -2.5f));
    EXPECT_FLOAT_EQ(0.0f, mathcore::add(5.0f, -5.0f));
}

// Test case for the subtract function
TEST_F(MathCoreTest, Subtraction) {
    EXPECT_FLOAT_EQ(0.0f, mathcore::subtract(2.5f, 2.5f));
    EXPECT_FLOAT_EQ(5.0f, mathcore::subtract(2.5f, -2.5f));
    EXPECT_FLOAT_EQ(-5.0f, mathcore::subtract(-2.5f, 2.5f));
    EXPECT_FLOAT_EQ(0.0f, mathcore::subtract(0.0f, 0.0f));
}

// Test case for the multiply function
TEST_F(MathCoreTest, Multiplication) {
    EXPECT_FLOAT_EQ(6.25f, mathcore::multiply(2.5f, 2.5f));
    EXPECT_FLOAT_EQ(0.0f, mathcore::multiply(10.0f, 0.0f));
    EXPECT_FLOAT_EQ(-6.25f, mathcore::multiply(2.5f, -2.5f));
    EXPECT_FLOAT_EQ(0.0f, mathcore::multiply(0.0f, 10.0f));
}

// Test case for the divide function
TEST_F(MathCoreTest, Division) {
    EXPECT_FLOAT_EQ(1.0f, mathcore::divide(2.5f, 2.5f));
    EXPECT_FLOAT_EQ(4.0f, mathcore::divide(10.0f, 2.5f));
    EXPECT_FLOAT_EQ(-4.0f, mathcore::divide(-10.0f, 2.5f));
    EXPECT_FLOAT_EQ(0.0f, mathcore::divide(0.0f, 10.0f));
}

// Test case for division by zero
TEST_F(MathCoreTest, DivisionByZero) {
    EXPECT_FLOAT_EQ(0.0f, mathcore::divide(10.0f, 0.0f));
    EXPECT_FLOAT_EQ(0.0f, mathcore::divide(-5.0f, 0.0f));
    EXPECT_FLOAT_EQ(0.0f, mathcore::divide(0.0f, 0.0f));
}

// You can add more TEST_F blocks for other functions or classes later 