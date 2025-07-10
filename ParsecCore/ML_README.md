# MLCore - Machine Learning Integration for ParsecCore

MLCore is an **optional machine learning layer** that extends ParsecCore with data collection, training, and prediction capabilities. It's designed to be non-intrusive and can be completely disabled if not needed.

## 🎯 Key Features

- **Optional Integration**: MLCore doesn't affect existing ParsecCore functionality
- **Real-time Data Collection**: Automatically collects simulation data during runtime
- **PID Optimization**: Built-in algorithms for optimizing PID controller parameters
- **System Prediction**: Simple ML models for predicting system behavior
- **Modular Design**: Easy to extend with new ML algorithms
- **JSON Configuration**: Human-readable ML model configuration

## 🏗️ Architecture

```
ParsecCore
├── SolverCore (Physics simulation)
├── EquationManager (Math expressions)
├── ConfigManager (Model loading)
└── MLCore (Optional ML layer)
    ├── Data Collection
    ├── Model Training
    ├── Prediction/Inference
    └── PID Optimization
```

## 📊 ML Model Types

### 1. PID Optimizer (`"pid_optimizer"`)
Optimizes PID controller parameters for better system control.

**Use Cases:**
- Servo motor control
- Temperature regulation
- Position control systems
- Any feedback control system

**Configuration:**
```json
{
  "model_name": "servo_pid_optimizer",
  "model_type": "pid_optimizer",
  "input_features": "position,velocity,setpoint",
  "output_targets": "control_signal",
  "hyperparameters": {
    "learning_rate": 0.005,
    "max_iterations": 500
  }
}
```

### 2. System Predictor (`"predictor"`)
Predicts future system behavior based on current state.

**Use Cases:**
- Predictive maintenance
- System monitoring
- Early warning systems
- Optimization planning

**Configuration:**
```json
{
  "model_name": "system_predictor",
  "model_type": "predictor",
  "input_features": "x,v,time",
  "output_targets": "x,v",
  "hyperparameters": {
    "prediction_horizon": 1.0,
    "window_size": 100
  }
}
```

## 🚀 Getting Started

### 1. Enable MLCore in ParsecCore

```cpp
#include "ParsecCore.h"

// Create ParsecCore instance
ParsecCore core(&platform);

// Load your physics model
core.loadModel("model.starmodel.json");

// Load ML configuration
core.loadMLConfiguration("ml_config.json");

// Enable data collection
core.enableMLDataCollection(true);
```

### 2. Create ML Configuration File

Create a JSON file (e.g., `ml_config.json`):

```json
{
  "settings": {
    "enabled": true,
    "data_collection_interval": 0.05,
    "max_data_points": 5000
  },
  "ml_models": [
    {
      "model_name": "my_pid_optimizer",
      "model_type": "pid_optimizer",
      "input_features": "error,error_rate",
      "output_targets": "control_output",
      "enabled": true
    }
  ]
}
```

### 3. Run Simulation with Data Collection

```cpp
// Run simulation loop
while (running) {
    core.tick();  // MLCore automatically collects data
    
    // Your simulation logic here
}
```

### 4. Train and Use ML Models

```cpp
// Train the model
core.trainMLModel("my_pid_optimizer");

// Get predictions/recommendations
auto prediction = core.runMLPrediction("my_pid_optimizer");

// Use PID recommendations
if (prediction.status == "success") {
    double kp = prediction.recommendations["kp"];
    double ki = prediction.recommendations["ki"];
    double kd = prediction.recommendations["kd"];
}
```

## 🔧 Configuration Options

### Global Settings
- `enabled`: Enable/disable MLCore
- `data_collection_interval`: Time between data collection (seconds)
- `max_data_points`: Maximum number of data points to store

### Per-Model Settings
- `model_name`: Unique identifier for the model
- `model_type`: Type of ML model (`"pid_optimizer"`, `"predictor"`)
- `input_features`: Comma-separated list of input variables
- `output_targets`: Comma-separated list of output variables
- `hyperparameters`: Model-specific parameters
- `enabled`: Enable/disable this specific model

## 📈 Advanced Usage

### Custom Callbacks

```cpp
// Set training callback
core.getMLCore()->setTrainingCallback([](const std::string& model_name, int epoch, double loss) {
    std::cout << "Training " << model_name << " - Loss: " << loss << std::endl;
});

// Set prediction callback
core.getMLCore()->setPredictionCallback([](const parsec::MLPrediction& prediction) {
    std::cout << "Prediction confidence: " << prediction.confidence << std::endl;
});
```

### Direct PID Optimization

```cpp
// Get collected data
auto data = core.getMLCore()->getCollectedData();

// Optimize PID parameters directly
auto pid_params = core.getMLCore()->optimizePIDParameters("position", "setpoint", data);

std::cout << "Kp: " << pid_params["kp"] << std::endl;
std::cout << "Ki: " << pid_params["ki"] << std::endl;
std::cout << "Kd: " << pid_params["kd"] << std::endl;
```

### Data Management

```cpp
// Get collected data size
size_t data_count = core.getMLCore()->getCollectedData().size();

// Clear collected data
core.getMLCore()->clearCollectedData();

// Process/preprocess data
core.getMLCore()->processCollectedData();
```

## 🔌 Integration Examples

### Example 1: Servo Motor Control
```cpp
// Load servo model and ML config
core.loadModel("servo_model.starmodel.json");
core.loadMLConfiguration("servo_ml_config.json");
core.enableMLDataCollection(true);

// Run servo control simulation
for (int i = 0; i < 10000; i++) {
    core.tick();
    
    // Every 1000 steps, optimize PID
    if (i % 1000 == 0) {
        core.trainMLModel("servo_pid_optimizer");
        auto prediction = core.runMLPrediction("servo_pid_optimizer");
        // Apply new PID parameters to servo
    }
}
```

### Example 2: Temperature Control
```cpp
// Configure for temperature control
core.loadModel("thermal_model.starmodel.json");
core.loadMLConfiguration("thermal_ml_config.json");

// Train model on collected data
core.trainMLModel("thermal_pid_optimizer");

// Get optimized parameters
auto prediction = core.runMLPrediction("thermal_pid_optimizer");
double kp = prediction.recommendations["kp"];
double ki = prediction.recommendations["ki"];
double kd = prediction.recommendations["kd"];
```

## 🛠️ Building with MLCore

MLCore is automatically included when building ParsecCore. No additional dependencies are required beyond the existing ParsecCore dependencies.

```bash
# Build ParsecCore with MLCore
mkdir build && cd build
cmake ..
cmake --build .
```

## 📝 Example Files

- `tests/ml_config_example.json` - Example ML configuration
- `main_ml_example.cpp` - Complete usage example
- `ML_README.md` - This documentation

## 🔮 Future Enhancements

### Planned Features
- **ONNX Runtime Integration**: Support for external ML models
- **TensorFlow Lite**: Embedded ML inference
- **Advanced Algorithms**: Kalman filters, neural networks
- **Model Export**: Save/load trained models
- **Real-time Adaptation**: Online learning capabilities

### PID Core Integration
MLCore is designed to work seamlessly with the planned **PID Core** component:
- MLCore optimizes PID parameters
- PID Core implements the actual control logic
- Real-time feedback loop for continuous optimization

## 🐛 Troubleshooting

### Common Issues

1. **MLCore not enabled**
   - Check that `"enabled": true` in ML configuration
   - Verify ML configuration file is loaded successfully

2. **No data collected**
   - Ensure `enableMLDataCollection(true)` is called
   - Check data collection interval settings

3. **Training fails**
   - Verify sufficient data points are collected
   - Check input/output feature names match simulation state

### Debug Output

Enable debug logging to see MLCore activity:
```cpp
// MLCore provides detailed logging through the platform
// Check console output for "[WIN] MLCore: ..." messages
```

## 🤝 Contributing

MLCore is designed to be extensible. To add new ML model types:

1. Add new model type to `MLModelConfig`
2. Implement training logic in `trainModel()`
3. Implement prediction logic in `predict()`
4. Add configuration validation
5. Update documentation

## 📜 License

MLCore is part of ParsecCore and follows the same license terms.

---

**MLCore** - Bringing machine learning to physics simulation! 🚀 