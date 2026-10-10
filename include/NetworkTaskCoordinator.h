#pragma once

#include <Arduino.h>
#include <vector>
#include <functional>

struct NetworkTask {
    String name;
    unsigned long intervalMs;
    unsigned long lastRunMs;
    unsigned long bootDelayMs;
    int priority; // lower number = higher priority
    bool forcePending;
    std::function<bool()> execute;
};

class NetworkTaskCoordinator {
public:
    static NetworkTaskCoordinator& getInstance();

    // Register a periodic network task (e.g. weather, calendar, email, notifications)
    void registerTask(const String& name,
                      unsigned long intervalMs,
                      unsigned long bootDelayMs,
                      int priority,
                      std::function<bool()> execute);

    // Request an immediate run of a specific task
    void requestRun(const String& name);

    // Call regularly in Arduino loop() when WiFi is connected
    void loop(bool isWiFiConnected);

    // Yield function to call during long network operations (e.g. streaming HTTPS)
    // Keeps LVGL rendering, ticks the clock, and feeds the watchdog
    static void yieldUI();

    // Set UI update callback for yield
    void setYieldCallback(std::function<void()> cb) { _yieldCallback = cb; }

    bool isBusy() const { return _isTaskRunning; }

private:
    NetworkTaskCoordinator();
    std::vector<NetworkTask> _tasks;
    unsigned long _bootTimeMs;
    unsigned long _lastTaskCompletionMs;
    static const unsigned long COOLDOWN_BETWEEN_TASKS_MS = 2500; // 2.5s between network tasks to settle TLS/heap
    bool _isTaskRunning;
    std::function<void()> _yieldCallback;
    static NetworkTaskCoordinator* s_instance;
};
