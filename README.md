# StarSim
A complete physics engine consisting of tools, models, algorithms and software for real-world physics simulations and real-time calculations based on live, past and predicted data.

## Project Structure

```
StarSim/
├── ParsecCore/                 # C++ simulation engine
│   ├── include/               # Organized header files
│   │   ├── parsec/           # Core simulation headers
│   │   ├── mlcore/           # ML functionality headers
│   │   ├── mathcore/         # Math utilities headers
│   │   └── platform/         # Platform abstraction headers
│   ├── src/                  # Organized implementation files
│   │   ├── parsec/           # Core simulation implementations
│   │   ├── mlcore/           # ML functionality implementations
│   │   └── platform/         # Platform implementations
│   ├── examples/             # Example applications
│   ├── tests/                # Unit tests and test data
│   ├── CMakeLists.txt        # Build configuration
│   └── README.md             # ParsecCore documentation
├── Test/                      # Legacy test directory
└── README.md                  # This file
```

## ParsecCore

**ParsecCore** is the portable, embeddable C++ runtime engine designed for real-time physics simulation and computation within the StarSim ecosystem. It loads system descriptions from configuration files, solves the defined equations over time using numerical methods, and integrates with various platforms via an abstraction layer.

### Current Status:
*   **Organized Structure:** Clean separation of headers and implementations by component (parsec, mlcore, mathcore, platform)
*   **Model Loading:** Loads models from `.starmodel.json` files (`ConfigManager`)
*   **Equation Parsing:** Uses **muParser** for mathematical expressions (`EquationManager`)
*   **Numerical Solving:** Implements **Forward Euler** solver (`SolverCore`)
*   **Platform Abstraction:** Abstracts platform details via `IPlatform`
*   **ML Integration:** Optional `MLCore` for PID optimization and system prediction
*   **Build System:** Uses **CMake** with automatic dependency management (`googletest`, `nlohmann/json`, `muParser`)
*   **Examples:** Includes basic simulation and ML demonstration applications
*   **Testing:** Comprehensive unit tests for all core components

### Key Features:
- **Real-time Physics Simulation:** Forward Euler integration with configurable time steps
- **JSON Configuration:** Human-readable model definitions with variables, equations, and solver settings
- **ML-Enhanced Simulation:** Optional machine learning for PID optimization and predictive modeling
- **Cross-Platform:** Supports Windows, Linux, and embedded systems via platform abstraction
- **Modular Design:** Use only the components you need (core simulation, ML, math utilities)

For detailed information on building and using ParsecCore, see the [ParsecCore README](./ParsecCore/README.md).

For ML Core functionality and configuration, see the [ML README](./ParsecCore/ML_README.md).

## StarSim Framework (Frontend & Orchestration)

*(TODO: Describe the higher-level StarSim application, UI, visualization, model building tools, and how it interacts with ParsecCore and potentially Comms components)*

## Comms Integration

*(TODO: Detail how StarSim plans to integrate with components from the Comms Alpha v2.0 project for UI (AriesUI), data streaming (Stream Handler), hardware I/O (Engine), and instance management (HyperThreader), including the 'locked mode' concept)*

## Quick Start

1. **Navigate to ParsecCore:**
   ```bash
   cd ParsecCore
   ```

2. **Build the project:**
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

3. **Run tests:**
   ```bash
   ctest
   ```

4. **Run examples:**
   ```bash
   # Basic physics simulation
   ./Debug/parsec_windows_app.exe
   
   # ML demonstration
   ./Debug/parsec_ml_example.exe
   ```

## Integration Examples

### Embedding ParsecCore
```cpp
#include "parsec/ParsecCore.h"
#include "platform/IPlatform.h"

// Your platform implementation
class MyPlatform : public IPlatform {
    // ... implement virtual methods
};

// Use ParsecCore
MyPlatform platform;
ParsecCore core(&platform);
core.loadModel("model.starmodel.json");

// Optional ML features
core.loadMLConfiguration("ml_config.json");

// Run simulation
while (running) {
    core.tick();
    auto state = core.getCurrentState();
    // Process simulation results...
}
```

### Model Configuration
```json
{
  "model_name": "SpringMassSystem",
  "variables": [
    {"name": "x", "type": "dynamic", "initial": 0.5},
    {"name": "v", "type": "dynamic", "initial": 0.0},
    {"name": "k", "type": "constant", "value": 10.0}
  ],
  "equations": [
    "d(x)/dt = v",
    "d(v)/dt = -k * x"
  ],
  "solver": {"method": "Euler", "dt": 0.001}
}
```
