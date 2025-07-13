#pragma once

#include <vector>
#include <string>
#include <map>
#include <functional>
#include <memory>
#include <chrono>
#include <thread>
#include <atomic>
#include <queue>

namespace parsec {

// Forward declarations
class IPlatform;

// Time measurement units
enum class TimeUnit {
    NANOSECONDS,
    MICROSECONDS,
    MILLISECONDS,
    SECONDS,
    MINUTES,
    HOURS
};

// Timing modes
enum class TimingMode {
    REAL_TIME,          // Match wall clock time
    SIMULATION_TIME,    // Run as fast as possible
    STEPPED,            // Manual step control
    EXTERNAL_SYNC,      // Synchronize with external signal
    DETERMINISTIC       // Fixed time steps regardless of computation time
};

// Timer configuration
struct TimerConfig {
    std::string name;                  // Timer identifier
    double interval;                   // Timer interval in seconds
    TimeUnit unit = TimeUnit::SECONDS; // Time unit for interval
    bool enabled = true;
    bool repeating = true;             // One-shot vs repeating
    double phase_offset = 0.0;         // Phase offset for synchronization
    int priority = 0;                  // Timer priority (higher = more important)
};

// Timer event structure
struct TimerEvent {
    std::string timer_name;
    double scheduled_time;    // When the event was supposed to fire
    double actual_time;       // When the event actually fired
    double jitter;            // Difference between scheduled and actual
    size_t event_id;          // Unique event identifier
    bool missed = false;      // Whether this event was missed due to timing constraints
};

// Timing statistics
struct TimingStats {
    std::string timer_name;
    size_t events_fired = 0;
    size_t events_missed = 0;
    double average_jitter = 0.0;
    double max_jitter = 0.0;
    double min_jitter = 0.0;
    double average_interval = 0.0;
    double last_event_time = 0.0;
    double total_runtime = 0.0;
};

// Real-time constraints
struct RealTimeConstraints {
    double max_computation_time = 0.01;  // Maximum time allowed for computation per cycle
    double deadline_tolerance = 0.001;   // How much lateness is acceptable
    bool enforce_deadlines = false;      // Whether to enforce hard real-time constraints
    bool skip_on_overrun = false;        // Skip cycles if we're behind schedule
    int max_catch_up_cycles = 3;         // Maximum number of cycles to catch up
};

// Callback function types
using TimerCallback = std::function<void(const TimerEvent& event)>;
using TimingErrorCallback = std::function<void(const std::string& timer_name, const std::string& error)>;
using ScheduleCallback = std::function<void(double simulation_time, double real_time)>;

// High-resolution timer interface
class ITimer {
public:
    virtual ~ITimer() = default;
    
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool isRunning() const = 0;
    
    virtual void setInterval(double interval_seconds) = 0;
    virtual double getInterval() const = 0;
    
    virtual void setCallback(TimerCallback callback) = 0;
    virtual TimingStats getStats() const = 0;
    virtual void resetStats() = 0;
    
    virtual std::string getName() const = 0;
};

// Platform-specific timer implementations
class HighResolutionTimer : public ITimer {
public:
    explicit HighResolutionTimer(const std::string& name, IPlatform* platform);
    ~HighResolutionTimer();
    
    bool start() override;
    bool stop() override;
    bool isRunning() const override;
    
    void setInterval(double interval_seconds) override;
    double getInterval() const override;
    
    void setCallback(TimerCallback callback) override;
    TimingStats getStats() const override;
    void resetStats() override;
    
    std::string getName() const override { return name_; }
    
private:
    std::string name_;
    IPlatform* platform_;
    TimerConfig config_;
    TimerCallback callback_;
    
    std::atomic<bool> running_ = false;
    std::thread timer_thread_;
    TimingStats stats_;
    
    void timerWorker();
    void fireEvent();
    void updateStats(const TimerEvent& event);
};

// Schedule manager for complex timing scenarios
class ScheduleManager {
public:
    explicit ScheduleManager(IPlatform* platform);
    
    // Schedule management
    bool addScheduleEntry(const std::string& name, double time, TimerCallback callback);
    bool removeScheduleEntry(const std::string& name);
    void clearSchedule();
    
    // Time-based events
    bool scheduleAt(const std::string& name, double absolute_time, TimerCallback callback);
    bool scheduleAfter(const std::string& name, double delay_seconds, TimerCallback callback);
    bool scheduleRepeating(const std::string& name, double interval, TimerCallback callback);
    
    // Execution control
    void executeSchedule(double current_time);
    void executeUntil(double end_time);
    void executePendingEvents();
    
    // Query functions
    std::vector<std::string> getPendingEvents() const;
    double getNextEventTime() const;
    size_t getPendingEventCount() const;
    
private:
    IPlatform* platform_;
    
    struct ScheduleEntry {
        std::string name;
        double time;
        TimerCallback callback;
        bool repeating = false;
        double interval = 0.0;
    };
    
    std::priority_queue<ScheduleEntry> schedule_;
    std::map<std::string, ScheduleEntry> named_entries_;
};

// Main Timing Layer class
class TimingLayer {
public:
    explicit TimingLayer(IPlatform* platform);
    ~TimingLayer();
    
    // Configuration
    bool loadTimingConfiguration(const std::string& timing_config_path);
    void setTimingMode(TimingMode mode);
    TimingMode getTimingMode() const;
    
    // Real-time constraints
    void setRealTimeConstraints(const RealTimeConstraints& constraints);
    RealTimeConstraints getRealTimeConstraints() const;
    
    // Timer management
    bool createTimer(const TimerConfig& config);
    bool removeTimer(const std::string& timer_name);
    bool startTimer(const std::string& timer_name);
    bool stopTimer(const std::string& timer_name);
    void startAllTimers();
    void stopAllTimers();
    
    // Timer configuration
    bool setTimerInterval(const std::string& timer_name, double interval_seconds);
    bool setTimerCallback(const std::string& timer_name, TimerCallback callback);
    bool enableTimer(const std::string& timer_name, bool enabled = true);
    
    // Simulation time control
    void setSimulationTimeStep(double dt);
    double getSimulationTimeStep() const;
    void setSimulationTime(double time);
    double getSimulationTime() const;
    double getRealTime() const;
    
    // Step control (for STEPPED mode)
    void step(); // Single step
    void step(size_t num_steps); // Multiple steps
    bool isWaitingForStep() const;
    
    // Synchronization
    void waitForNextTick();
    void synchronizeWith(const std::string& external_signal);
    bool isSynchronized() const;
    
    // Statistics and monitoring
    std::vector<TimingStats> getAllTimingStats() const;
    TimingStats getTimingStats(const std::string& timer_name) const;
    void resetAllStats();
    bool isRealTimeViolated() const;
    double getCurrentJitter() const;
    
    // Callbacks
    void setScheduleCallback(ScheduleCallback callback);
    void setTimingErrorCallback(TimingErrorCallback callback);
    
    // Schedule management
    ScheduleManager* getScheduleManager();
    
    // Control
    void start();
    void stop();
    bool isRunning() const;
    void pause();
    void resume();
    bool isPaused() const;
    
    // Time utilities
    static double convertTime(double value, TimeUnit from, TimeUnit to);
    static std::string timeUnitToString(TimeUnit unit);
    static TimeUnit stringToTimeUnit(const std::string& unit_str);
    
private:
    IPlatform* platform_;
    TimingMode mode_ = TimingMode::REAL_TIME;
    RealTimeConstraints constraints_;
    
    // Simulation time tracking
    double simulation_time_ = 0.0;
    double simulation_time_step_ = 0.001; // 1ms default
    double last_real_time_ = 0.0;
    
    // Timer management
    std::map<std::string, std::unique_ptr<ITimer>> timers_;
    std::map<std::string, TimerConfig> timer_configs_;
    
    // Schedule management
    std::unique_ptr<ScheduleManager> schedule_manager_;
    
    // State tracking
    bool running_ = false;
    bool paused_ = false;
    std::atomic<bool> waiting_for_step_ = false;
    
    // Threading
    std::thread master_thread_;
    std::atomic<bool> master_running_ = false;
    
    // Callbacks
    ScheduleCallback scheduleCallback_;
    TimingErrorCallback errorCallback_;
    
    // Internal workers
    void masterTimingLoop();
    void processRealTimeMode();
    void processSimulationTimeMode();
    void processSteppedMode();
    void processExternalSyncMode();
    void processDeterministicMode();
    
    // Real-time constraint checking
    bool checkRealTimeConstraints(double computation_time);
    void handleRealTimeViolation(const std::string& violation_type);
    
    // Synchronization helpers
    void waitForPreciseInterval(double interval_seconds);
    double getHighResolutionTime() const;
    
    // Statistics
    void updateGlobalStats();
    
    // Error handling
    void handleTimingError(const std::string& timer_name, const std::string& error);
    
    // Validation
    bool validateTimerConfig(const TimerConfig& config) const;
};

} // namespace parsec 