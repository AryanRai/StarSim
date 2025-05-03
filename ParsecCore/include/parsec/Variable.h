#pragma once

#include <string>
#include <variant>

namespace parsec {

enum class VariableType {
    DYNAMIC,
    CONSTANT,
    INPUT // Potentially add other types later (e.g., OUTPUT)
};

struct Variable {
    std::string name;
    std::string unit;
    VariableType type = VariableType::DYNAMIC;
    // Use double for potential precision needs in physics calculations
    std::variant<double, std::string> value; // Store initial value for CONSTANT/DYNAMIC, maybe placeholder string for INPUT

    // Add helper methods later if needed (e.g., to get value as double)
};

} // namespace parsec 