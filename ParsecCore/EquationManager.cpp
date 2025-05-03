#include "parsec/EquationManager.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <set>

namespace parsec {

// Helper to trim whitespace
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

EquationManager::EquationManager(const ModelConfig& config)
    : modelConfig_(config) {
    parseAndStoreEquations();
}

void EquationManager::parseAndStoreEquations() {
    std::set<std::string> allReferencedVars; // Keep track of all variables needed by parsers

    for (const std::string& eqStr : modelConfig_.equations) {
        std::size_t equalsPos = eqStr.find('=');
        if (equalsPos == std::string::npos) {
            std::cerr << "Warning: Skipping invalid equation string (missing '='): " << eqStr << std::endl;
            continue;
        }

        std::string lhs = trim(eqStr.substr(0, equalsPos));
        std::string rhs = trim(eqStr.substr(equalsPos + 1));

        // Basic check if LHS looks like a derivative d(var)/dt
        if (lhs.rfind(")/dt") == lhs.length() - 4 && lhs.find("d(") == 0) {
             ParsedEquation parsedEq;
             parsedEq.derivativeName = lhs;

             try {
                 parsedEq.parser.SetExpr(rhs);

                 // Find variables used in the expression
                 mu::varmap_type usedVars = parsedEq.parser.GetUsedVar();
                 for (auto const& [name, pointer] : usedVars) {
                     // We don't store the pointer from GetUsedVar directly.
                     // Instead, we store the *name* and will link a pointer
                     // to our own storage (parserVariableValues_) later.
                     allReferencedVars.insert(name);
                     // Store null initially, will link in evaluateDerivatives
                     parsedEq.variablePointers[name] = nullptr;
                 }

                 // Pre-check expression (optional but good for early errors)
                 // Note: This requires variables to be defined, which we do later.
                 // parsedEq.parser.Eval();

                 parsedDerivativeEquations_.push_back(std::move(parsedEq));

             } catch (mu::ParserError &e) {
                 std::cerr << "muParser Error parsing equation: '" << eqStr << "'\n"
                           << "  Error: " << e.GetMsg() << "\n";
             } catch (const std::exception& e) {
                 std::cerr << "Error setting up parser for equation: '" << eqStr << "'\n"
                           << "  Error: " << e.what() << "\n";
             }

        } else {
             // Handle algebraic equations later if needed
             std::cerr << "Warning: Skipping non-derivative equation (or improperly formatted): " << eqStr << std::endl;
        }
    }

    // Initialize our internal storage for all variables referenced by parsers
    for(const std::string& varName : allReferencedVars) {
        parserVariableValues_[varName] = 0.0; // Initialize to 0
    }

     // Now, link the variable pointers in each parser to our storage
     for(auto& parsedEq : parsedDerivativeEquations_) {
         for(auto const& [name, pointer_placeholder] : parsedEq.variablePointers) {
             // Find the corresponding value storage
             auto it = parserVariableValues_.find(name);
             if (it != parserVariableValues_.end()) {
                 // Define the variable in this specific parser instance, pointing to our storage
                 try {
                    parsedEq.parser.DefineVar(name, &(it->second));
                    parsedEq.variablePointers[name] = &(it->second); // Store the actual pointer now
                 } catch (mu::ParserError &e) {
                     std::cerr << "muParser Error defining variable '" << name << "' for equation: '"
                               << parsedEq.derivativeName << " = ...'\n"
                               << "  Error: " << e.GetMsg() << "\n";
                 }
             } else {
                 // This shouldn't happen if allReferencedVars logic is correct
                 std::cerr << "Internal Error: Variable '" << name << "' referenced by parser but not found in internal storage." << std::endl;
             }
         }
     }
}


std::map<std::string, double> EquationManager::evaluateDerivatives(const std::map<std::string, double>& currentState) {
    std::map<std::string, double> derivatives;

    // 1. Update the values in our internal storage using the current simulation state
    for(auto& [name, valueRef] : parserVariableValues_) {
        auto it = currentState.find(name);
        if (it != currentState.end()) {
            valueRef = it->second;
        } else {
            // Variable needed by parser not found in current state!
            // This could be a constant defined in the config, or an error.
            // For now, try finding it in the config as a constant.
            bool foundConstant = false;
            for(const auto& var : modelConfig_.variables) {
                if (var.name == name && var.type == VariableType::CONSTANT && std::holds_alternative<double>(var.value)) {
                    valueRef = std::get<double>(var.value);
                    foundConstant = true;
                    break;
                }
            }
            if (!foundConstant) {
                 std::cerr << "Warning: Variable '" << name
                           << "' needed by an equation parser was not found in the current simulation state or constants. Using previous/default value: "
                           << valueRef << std::endl;
                 // Keep the existing value in valueRef (might be 0.0 or from previous step)
            }
        }
    }

    // 2. Evaluate each parsed derivative equation
    for (auto& parsedEq : parsedDerivativeEquations_) {
        try {
            // Variables linked via pointers are already updated
            double result = parsedEq.parser.Eval();
            derivatives[parsedEq.derivativeName] = result;
        } catch (mu::ParserError &e) {
            std::cerr << "muParser Error evaluating equation for '" << parsedEq.derivativeName << "':\n"
                      << "  Expression: " << parsedEq.parser.GetExpr() << "\n"
                      << "  Error: " << e.GetMsg() << " at position " << e.GetPos() << " (Token: '" << e.GetToken() << "')\n";
            // Decide on error handling: return partial results, throw, set to NaN? Returning 0 for now.
            derivatives[parsedEq.derivativeName] = 0.0;
        } catch (const std::exception& e) {
            std::cerr << "Standard Error evaluating equation for '" << parsedEq.derivativeName << "': " << e.what() << std::endl;
             derivatives[parsedEq.derivativeName] = 0.0;
        }
    }

    return derivatives;
}

const std::vector<std::string>& EquationManager::getEquationStrings() const {
    return modelConfig_.equations;
}

const std::vector<Variable>& EquationManager::getVariables() const {
    return modelConfig_.variables;
}

// Remove the old temporary helper
// double EquationManager::getVariableValue(const std::map<std::string, double>& state, const std::string& name) const { ... }

} // namespace parsec 