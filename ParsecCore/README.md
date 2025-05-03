# ParsecCore

**ParsecCore** is a portable, embeddable C++ runtime engine for real-time physics simulation and computation. It is designed to load system descriptions from configuration files, solve the defined equations over time, and integrate with various platforms.

This project forms the backend computation layer for the larger **StarSim** simulation framework but can also function independently.

## Current Status & Features

*   **Model Loading:** Loads system definitions (variables, constants, equations, solver settings) from JSON files (`.starmodel.json`).
*   **Equation Parsing:** Uses the **muParser** library to parse and evaluate mathematical expressions defined in the model's equations.
    *   *Current Convention:* Equations defining derivatives must be explicitly solved for the derivative term (e.g., `d(x)/dt = ...`).
*   **Numerical Solver:** Implements a basic **Forward Euler** integration method in `SolverCore` to step the simulation forward in time.
*   **Configuration Management:** `ConfigManager` handles reading and validating the JSON model files.
*   **Platform Abstraction:** Uses an `IPlatform` interface (`IPlatform.h`) for platform-specific operations like logging and timing, enabling portability.
    *   Includes a basic `WindowsPlatform` implementation (`main_windows.cpp`).
*   **Unit Testing:** Uses **Google Test** framework (`googletest`) integrated via CMake for unit testing core components (`mathcore`, `ConfigManager`, `SolverCore`).
*   **Build System:** Uses **CMake** for cross-platform building and dependency management (`FetchContent`).

## Core Components

*   **`ParsecCore`**: The main orchestrator class. Loads models, manages managers, drives the simulation `tick()`, and provides state access.
*   **`IPlatform`**: Interface for platform-specific functions (logging, timing).
*   **`ConfigManager`**: Loads and parses `.starmodel.json` files using `nlohmann/json`.
*   **`ModelConfig` / `Variable`**: Data structures representing the loaded model in memory.
*   **`EquationManager`**: Parses equation strings from the config using `muParser` and evaluates derivatives.
*   **`SolverCore`**: Implements numerical integration methods (currently Forward Euler) to update the simulation state.
*   **`mathcore`**: Basic header-only math utilities (currently simple arithmetic).

## Dependencies

*   **CMake** (version 3.14 or higher)
*   **C++17 compliant compiler** (e.g., MSVC on Windows, GCC, Clang)
*   **Git** (required by CMake FetchContent)

The following dependencies are fetched automatically by CMake using `FetchContent`:
*   **googletest** (v1.14.0): For unit testing.
*   **nlohmann/json** (v3.11.3): For parsing JSON configuration files.
*   **muParser** (v2.3.5): For parsing mathematical expressions in equations.

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
    This will compile the core components, the test executable, and the example Windows application.

## Running

All commands should be run from the `ParsecCore/build` directory after building.

*   **Run Unit Tests:**
    ```bash
    # Using CTest (shows summary)
    ctest

    # Using CTest (verbose output)
    ctest --verbose 
    ```
    *(Note: The internal `ParserTest` from muParser might show as "Not Run", which is expected).* 

*   **Run Example Windows Application:**
    This runs the simulation defined in `tests/test_model.starmodel.json` using the basic Windows platform implementation and logs the state periodically.
    ```bash
    # The executable might be in a subdirectory like Debug or Release
    ./Debug/parsec_windows_app.exe 
    ```
    Press `Ctrl+C` to stop the application.

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

## Future Steps / TODO

*   Implement more sophisticated solvers (e.g., RK4) in `SolverCore`.
*   Enhance `EquationManager` to handle algebraic equations or different formats.
*   Develop the `PhysCore` component with specific physics domain models.
*   Implement the `Communication Layer` for input/output handling (linking `INPUT` variables).
*   Create `IPlatform` implementations for other targets (Teensy, Linux).
*   Integrate `MLCore` (e.g., ONNX Runtime).
*   Refine error handling and reporting. 