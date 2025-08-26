# StarSim - Comms Alpha v3.0 Integration

This document describes how to use the integration between StarSim's ParsecCore and Comms Alpha v3.0.

## Architecture Overview

The integration follows this architecture:

1. **Stream Handler (WebSocket Server)**: Provides real-time communication between components
2. **ParsecCore InputManager (C++)**: Connects to Stream Handler to send/receive simulation data
3. **AriesUI Physics Widgets**: Display simulation data in the UI

```
+-------------------+        +-------------------+        +-------------------+
|                   |        |                   |        |                   |
|  ParsecCore C++   | <----> |  Stream Handler   | <----> |  AriesUI Physics  |
|  (InputManager)   |        |  (WebSocket)      |        |  Widgets          |
|                   |        |                   |        |                   |
+-------------------+        +-------------------+        +-------------------+
```

## Building ParsecCore

To build the ParsecCore library with InputManager support:

```bash
# Navigate to ParsecCore directory
cd int/StarSim/ParsecCore

# Create build directory
mkdir -p build && cd build

# Configure and build
cmake ..
cmake --build .
```

## Running the Integration Example

The integration example demonstrates a spring-mass system simulation with real-time visualization:

```bash
# Navigate to examples directory
cd int/StarSim/examples

# Run the integration example
python stream_handler_integration.py
```

This will:
1. Start the Stream Handler WebSocket server
2. Run the C++ InputManager example that simulates a spring-mass system
3. Send simulation data to the Stream Handler

## Using AriesUI to Visualize the Simulation

To visualize the simulation in AriesUI:

1. Start AriesUI:
   ```bash
   cd ui/ariesUI
   npm run dev
   ```

2. Load the StarSim demo layout:
   - Open AriesUI in your browser
   - Click on "Layouts" in the sidebar
   - Select "StarSim Physics Demo"

3. Connect to the Stream Handler:
   - Click on the connection icon in the top right
   - Enter `ws://localhost:3000` as the WebSocket URL
   - Click "Connect"

## Creating Your Own Physics Simulations

To create your own physics simulations using this integration:

1. **Create a C++ simulation using InputManager**:
   ```cpp
   #include "parsec/InputManager.h"
   
   int main() {
       // Create input manager
       parsec::InputManager input_manager("my_simulation");
       
       // Initialize and connect
       input_manager.initialize("ws://localhost:3000");
       
       // Register streams
       input_manager.registerStream("position", "Position", "float", "m");
       
       // Update simulation status
       input_manager.updateStatus("running");
       
       // Simulation loop
       for (int i = 0; i < 1000; ++i) {
           // Update simulation
           // ...
           
           // Send data to Stream Handler
           input_manager.updateStreamValue("position", position);
           
           // Sleep to control simulation speed
           std::this_thread::sleep_for(std::chrono::milliseconds(10));
       }
       
       // Update simulation status
       input_manager.updateStatus("completed");
       
       return 0;
   }
   ```

2. **Create a custom AriesUI layout**:
   - Create a new layout JSON file in `Public/AriesMods/layouts/`
   - Add physics widgets that connect to your simulation's streams
   - Configure the widgets to display your simulation data

## Available Physics Widgets

The following physics widgets are available in AriesUI:

1. **PhysicsValueMonitor**: Displays a single value with unit and trend indicator
2. **PhysicsChart**: Real-time chart for physics values over time
3. **PhysicsVectorField**: 2D vector field visualization
4. **PhysicsControlPanel**: Control panel for simulation parameters

## Stream Handler Protocol

The Stream Handler uses a WebSocket protocol with JSON messages:

### Registration Message
```json
{
  "type": "physics_simulation",
  "action": "register",
  "simulation_id": "my_simulation",
  "config": {
    "solver": "RK4",
    "dt": 0.001
  }
}
```

### Stream Update Message
```json
{
  "type": "physics_simulation",
  "action": "update",
  "simulation_id": "my_simulation",
  "stream_id": "position",
  "data": {
    "value": 1.234,
    "timestamp": "2025-07-17 12:00:00.123"
  }
}
```

### Status Update Message
```json
{
  "type": "physics_simulation",
  "action": "status",
  "simulation_id": "my_simulation",
  "status": "running"
}
```

### Control Command Message
```json
{
  "type": "physics_simulation",
  "action": "control",
  "simulation_id": "my_simulation",
  "command": "set_parameter",
  "params": {
    "name": "spring_constant",
    "value": "15.0"
  }
}
```