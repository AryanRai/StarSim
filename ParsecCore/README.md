# ParsecCore

**ParsecCore** is a portable, embeddable C++ runtime engine for real-time physics simulation and computation. It is designed to load system descriptions from configuration files, solve the defined equations over time, and integrate with various platforms.

This project forms the backend computation layer for the larger **StarSim** simulation framework but can also function independently.

## Project Structure

```
ParsecCore/
├── include/                    # Header files organized by component
│   ├── parsec/                # Core simulation headers
│   │   ├── ParsecCore.h
│   │   ├── ConfigManager.h
│   │   ├── EquationManager.h
│   │   ├── SolverCore.h
│   │   ├── ModelConfig.h
│   │   └── Variable.h
│   ├── mlcore/                # ML functionality headers
│   │   └── MLCore.h
│   ├── mathcore/              # Math utilities headers
│   │   └── math.h
│   └── platform/              # Platform abstraction headers
│       └── IPlatform.h
├── src/                       # Implementation files organized by component
│   ├── parsec/               # Core simulation implementations
│   │   ├── ParsecCore.cpp
│   │   ├── ConfigManager.cpp
│   │   ├── EquationManager.cpp
│   │   └── SolverCore.cpp
│   ├── mlcore/               # ML functionality implementations
│   │   └── MLCore.cpp
│   └── platform/             # Platform implementations (future)
├── examples/                  # Example applications
│   ├── main_windows.cpp      # Basic Windows example
│   └── main_ml_example.cpp   # ML demonstration example
├── tests/                     # Unit tests and test data
│   ├── config_manager_tests.cpp
│   ├── math_tests.cpp
│   ├── solver_core_tests.cpp
│   ├── test_model.starmodel.json
│   └── ml_config_example.json
├── CMakeLists.txt            # Build configuration
├── README.md                 # This file
└── ML_README.md             # ML Core documentation
```

## Current Status & Features

### Core Features
*   **Model Loading:** Loads system definitions (variables, constants, equations, solver settings) from JSON files (`.starmodel.json`).
*   **Equation Parsing:** Uses the **muParser** library to parse and evaluate mathematical expressions defined in the model's equations.
    *   *Current Convention:* Equations defining derivatives must be explicitly solved for the derivative term (e.g., `d(x)/dt = ...`).
*   **Numerical Solver:** Implements a basic **Forward Euler** integration method in `SolverCore` to step the simulation forward in time.
*   **Configuration Management:** `ConfigManager` handles reading and validating the JSON model files.
*   **Platform Abstraction:** Uses an `IPlatform` interface for platform-specific operations like logging and timing, enabling portability.

### ML Core Features (Optional)
*   **ML Integration:** Optional `MLCore` component for machine learning capabilities
*   **PID Optimization:** Automatic PID parameter tuning for control systems
*   **System Prediction:** Predictive modeling for maintenance and monitoring
*   **Real-time Data Collection:** Configurable data collection during simulation
*   **Model Training:** Support for training ML models on collected simulation data

### Build & Testing
*   **Unit Testing:** Uses **Google Test** framework integrated via CMake for comprehensive testing
*   **Build System:** Uses **CMake** for cross-platform building and dependency management (`FetchContent`)
*   **Cross-Platform:** Supports Windows, Linux, and embedded targets

## Core Components

### Simulation Core
*   **`ParsecCore`**: Main orchestrator class that loads models, manages components, drives simulation `tick()`, and provides state access
*   **`ConfigManager`**: Loads and parses `.starmodel.json` files using `nlohmann/json`
*   **`EquationManager`**: Parses equation strings from config using `muParser` and evaluates derivatives
*   **`SolverCore`**: Implements numerical integration methods (currently Forward Euler) to update simulation state

### Platform Layer
*   **`IPlatform`**: Interface for platform-specific functions (logging, timing)

### Data Structures
*   **`ModelConfig` / `Variable`**: Data structures representing the loaded model in memory
*   **`SimulationState`**: Map-based storage for current variable values

### ML Layer (Optional)
*   **`MLCore`**: Machine learning component for optimization and prediction
*   **`MLModelConfig`**: Configuration for ML models
*   **`MLDataPoint`**: Structure for training/inference data
*   **`MLPrediction`**: Structure for ML predictions and recommendations

### Math Utilities
*   **`mathcore`**: Header-only math utilities for common calculations

## Dependencies

*   **CMake** (version 3.14 or higher)
*   **C++17 compliant compiler** (e.g., MSVC on Windows, GCC, Clang)
*   **Git** (required by CMake FetchContent)

The following dependencies are fetched automatically by CMake using `FetchContent`:
*   **googletest** (v1.14.0): For unit testing
*   **nlohmann/json** (v3.11.3): For parsing JSON configuration files
*   **muParser** (v2.3.5): For parsing mathematical expressions in equations

## Build Instructions

1.  **Clone the repository** (if applicable).
2.  **Ensure Prerequisites** are installed (CMake, C++17 Compiler, Git).
3.  **Open a terminal** in the `ParsecCore` directory.
4.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```
5.  **Configure CMake** (this also downloads dependencies). On Windows, you might specify the generator:
    ```bash
    # For Visual Studio 2022 (example)
    cmake .. -G "Visual Studio 17 2022"

    # On Linux/macOS (usually finds default generator)
    # cmake .. 
    ```
6.  **Build the project:**
    ```bash
    cmake --build .
    ```
    This will compile the core components, tests, and example applications.

## Running

All commands should be run from the `ParsecCore/build` directory after building.

### Unit Tests
```bash
# Using CTest (shows summary)
ctest

# Using CTest (verbose output)
ctest --verbose 
```
*(Note: The internal `ParserTest` from muParser might show as "Not Run", which is expected).* 

### Example Applications

**Basic Windows Example:**
```bash
# The executable might be in a subdirectory like Debug or Release
./Debug/parsec_windows_app.exe 
```
This runs the simulation defined in `tests/test_model.starmodel.json` and logs the state periodically.

**ML Example:**
```bash
./Debug/parsec_ml_example.exe
```
This demonstrates ML Core functionality including data collection, model training, and prediction.

Press `Ctrl+C` to stop any application.

## Example Model Format (`.starmodel.json`)

The engine expects configuration files like `tests/test_model.starmodel.json`:

```json
{
  "model_name": "TestSpringMass",
  "version": "1.1",
  "metadata": {
    "author": "Test User",
    "description": "A simple test model"
  },
  "variables": [
    { "name": "x", "unit": "m", "type": "dynamic", "initial": 0.5 },
    { "name": "v", "unit": "m/s", "type": "dynamic", "initial": 0.0 },
    { "name": "m", "unit": "kg", "type": "constant", "value": 2.0 },
    { "name": "k", "unit": "N/m", "type": "constant", "value": 10.0 },
    { "name": "F_ext", "unit": "N", "type": "input", "placeholder": "external_force_stream" }
  ],
  "equations": [
    "d(v)/dt = (-k * x + F_ext) / m", 
    "d(x)/dt = v"
  ],
  "solver": {
    "method": "Euler",
    "dt": 0.001,
    "duration": 5.0
  },
  "outputs": ["x", "v"]
}
```

## ML Configuration (Optional)

For ML functionality, use configuration files like `tests/ml_config_example.json`. See `ML_README.md` for detailed ML Core documentation.

## Future Steps / TODO

*   Implement more sophisticated solvers (e.g., RK4) in `SolverCore`
*   Enhance `EquationManager` to handle algebraic equations or different formats
*   Develop the `PhysCore` component with specific physics domain models
*   Implement the `Communication Layer` for input/output handling (linking `INPUT` variables)
*   Create `IPlatform` implementations for other targets (Teensy, Linux)
*   Advanced ML models and optimization algorithms
*   Refine error handling and reporting

## Integration Examples

### Embedding in Other Applications

`ParsecCore` is designed to be embedded within other applications. The modular structure makes it easy to include only the components you need:

```cpp
#include "parsec/ParsecCore.h"
#include "platform/IPlatform.h"

// Implement your platform
class MyPlatform : public IPlatform {
    // ... implement virtual methods
};

// Use ParsecCore
MyPlatform platform;
ParsecCore core(&platform);
core.loadModel("my_model.starmodel.json");

// Optional: Enable ML features
core.loadMLConfiguration("ml_config.json");

// Run simulation
while (running) {
    core.tick();
    auto state = core.getCurrentState();
    // Process state...
}
```

### ROS 2 Integration

The organized structure makes ROS 2 integration straightforward. Include the `ParsecCore` directory as a subdirectory in your ROS 2 package and link against the appropriate targets. The platform abstraction layer allows easy integration with ROS 2 logging and timing systems. 