#pragma once

#include "parsec/ModelConfig.h"
#include <vector>
#include <string>
#include <map>
#include <memory> // For unique_ptr
#include "muParser.h" // Include muParser header

namespace parsec {

// Forward declaration (if needed, maybe for State struct)
// struct SimulationState;

// Structure to hold a parsed derivative equation
struct ParsedEquation {
    std::string derivativeName; // e.g., "d(x)/dt"
    mu::Parser parser;          // The muParser instance for the expression
    // Pointers to the variables used in this expression's parser
    // The keys are variable names (e.g., "x", "k"), values are pointers for muParser
    std::map<std::string, double*> variablePointers;
};

class EquationManager {
public:
    explicit EquationManager(const ModelConfig& config);

    // Placeholder for evaluating derivatives - will become more complex
    // Takes the current state (mapping variable names to values)
    // Returns a map of derivative names (e.g., "d(x)/dt") to their calculated values
    std::map<std::string, double> evaluateDerivatives(const std::map<std::string, double>& currentState);

    // Placeholder for evaluating algebraic equations (if any)
    // void evaluateAlgebraic(std::map<std::string, double>& currentState);

    const std::vector<std::string>& getEquationStrings() const;
    const std::vector<Variable>& getVariables() const;

private:
    const ModelConfig& modelConfig_; // Store a reference to the loaded config
    std::vector<ParsedEquation> parsedDerivativeEquations_;
    // Need a place to store the *values* that the parser variables point to.
    // This map holds the current value for each variable name referenced by any parser.
    // Note: This assumes variable names are unique across all equations.
    std::map<std::string, double> parserVariableValues_;

    // Helper to parse equation strings during construction
    void parseAndStoreEquations();

    // Temporary helper for the TwoMassSpring example in evaluateDerivatives
    double getVariableValue(const std::map<std::string, double>& state, const std::string& name) const;
};

} // namespace parsec 