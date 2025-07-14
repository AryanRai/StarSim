# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

StarSim is a complete physics simulation engine with a C++ backend (ParsecCore) designed for real-time physics simulations and calculations. The project uses JSON-based model configuration and supports optional machine learning integration for optimization and prediction.

## Build Commands

All commands should be run from the `ParsecCore` directory unless otherwise specified.

**Build the project:**
```bash
cd ParsecCore
mkdir build && cd build
cmake ..
cmake --build .
```

**Run tests:**
```bash
cd ParsecCore/build
ctest
# Or for verbose output:
ctest --verbose
```

**Run example applications:**
```bash
cd ParsecCore/build
# Basic physics simulation
./Debug/parsec_windows_app.exe
# ML demonstration
./Debug/parsec_ml_example.exe
```

## Architecture Overview

### Core Components
- **ParsecCore**: Main orchestrator class that loads models, manages simulation tick(), and provides state access
- **ConfigManager**: Loads and parses `.starmodel.json` files using nlohmann/json
- **EquationManager**: Parses mathematical expressions using muParser library
- **SolverCore**: Implements numerical integration (currently Forward Euler method)
- **MLCore**: Optional machine learning layer for PID optimization and prediction

### Modular Structure
The project recently underwent major refactoring from monolithic to modular architecture:

**PhysCore modules:**
- Statics, Dynamics, Thermofluids, MaterialScience, Controls, PhysicsConstants, PhysicsUtils

**MathCore modules:**
- BasicMath, LinearAlgebra, Statistics, NumericalMethods

**MLCore modules:**
- MLDataStructures, DataCollection, PIDOptimizer, LinearRegression

### Directory Organization
```
ParsecCore/
├── include/                    # Headers organized by component
│   ├── parsec/                # Core simulation headers
│   ├── mlcore/                # ML functionality headers  
│   ├── mathcore/              # Math utilities headers
│   ├── physcore/              # Physics domain headers
│   ├── platform/              # Platform abstraction headers
│   ├── comms/                 # Communication layer headers
│   └── timing/                # Timing utilities headers
├── src/                       # Implementation files by component
├── examples/                  # Example applications
├── tests/                     # Unit tests with GoogleTest
└── CMakeLists.txt            # Build configuration
```

## Key Configuration Files

**Model files**: `.starmodel.json` format defines simulation variables, equations, and solver settings
- Example: `ParsecCore/tests/test_model.starmodel.json`
- Variables can be: dynamic (state), constant (parameters), or input (external data)
- Equations use muParser syntax with derivative notation: `d(x)/dt = v`

**ML configuration**: `ml_config.json` format for machine learning features
- Example: `ParsecCore/tests/ml_config_example.json`
- Supports PID optimization and system prediction models

## Dependencies

**Required:**
- CMake 3.14+
- C++17 compliant compiler
- Git (for FetchContent)

**Auto-fetched by CMake:**
- GoogleTest v1.14.0 (testing)
- nlohmann/json v3.11.3 (JSON parsing)
- muParser v2.3.5 (mathematical expressions)

## Platform Support

The project uses `IPlatform` interface for platform abstraction:
- Windows (Visual Studio 2022)
- Linux/macOS support
- Embedded systems (Teensy/Arduino via `main_teensy.ino`)

## Development Notes

**Recent changes:**
- ✅ **Completed modular architecture refactoring** from monolithic to modular design
- ✅ **Fixed all compilation errors** and reorganized codebase according to proposed structure
- ✅ **Implemented proper namespace organization** - mathcore::BasicMath::function() structure
- ✅ **Created complete MLCore class** with proper interfaces and implementations
- ✅ **All tests passing** (11/11 ParsecCore tests successful)
- Git status shows old monolithic files deleted and new modular structure in place

**Testing:**
- Comprehensive unit tests using GoogleTest framework
- CTest integration for test running
- Test files include model validation and component testing

**Cross-platform builds:**
- CMake handles platform detection
- Windows builds typically output to Debug/ or Release/ subdirectories
- Use cmake --build . for platform-agnostic building