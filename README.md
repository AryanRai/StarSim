# StarSim
A complete physics engine consiting of tools, models, algorithms and software for real-world physics simulations and real-time calculations based on live, past and predicted data.


## ParsecCore

**ParsecCore** is the portable, embeddable C++ runtime engine designed for real-time physics simulation and computation within the StarSim ecosystem. It loads system descriptions from configuration files, solves the defined equations over time using numerical methods, and integrates with various platforms via an abstraction layer.

**Current Status:**
*   Loads models from `.starmodel.json` files (`ConfigManager`).
*   Parses equations using **muParser** (`EquationManager`). Requires equations solved for `d(var)/dt`.
*   Includes a basic **Forward Euler** solver (`SolverCore`).
*   Abstracts platform details via `IPlatform`.
*   Uses **CMake** for building and managing dependencies (`googletest`, `nlohmann/json`, `muParser`) via `FetchContent`.
*   Basic Windows implementation and unit tests are functional.

For more detailed information on building and using ParsecCore directly, see the [ParsecCore README](./ParsecCore/README.md).

## StarSim Framework (Frontend & Orchestration)

*(TODO: Describe the higher-level StarSim application, UI, visualization, model building tools, and how it interacts with ParsecCore and potentially Comms components)*

## Comms Integration

*(TODO: Detail how StarSim plans to integrate with components from the Comms Alpha v2.0 project for UI (AriesUI), data streaming (Stream Handler), hardware I/O (Engine), and instance management (HyperThreader), including the 'locked mode' concept)*
