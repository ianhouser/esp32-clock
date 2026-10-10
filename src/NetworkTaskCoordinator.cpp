#include "NetworkTaskCoordinator.h"

NetworkTaskCoordinator* NetworkTaskCoordinator::s_instance = nullptr;

NetworkTaskCoordinator& NetworkTaskCoordinator::getInstance() {
    if (!s_instance) {
        s_instance = new NetworkTaskCoordinator();
    }
    return *s_instance;
}

NetworkTaskCoordinator::NetworkTaskCoordinator()
    : _bootTimeMs(millis())
    , _lastTaskCompletionMs(0)
    , _isTaskRunning(false)
    , _yieldCallback(nullptr) {
    s_instance = this;
}

void NetworkTaskCoordinator::registerTask(const String& name,
                                          unsigned long intervalMs,
                                          unsigned long bootDelayMs,
                                          int priority,
                                          std::function<bool()> execute) {
    // Check if task already exists and update
    for (auto& task : _tasks) {
        if (task.name == name) {
            task.intervalMs = intervalMs;
            task.bootDelayMs = bootDelayMs;
            task.priority = priority;
            task.execute = execute;
            return;
        }
    }

    NetworkTask task;
    task.name = name;
    task.intervalMs = intervalMs;
    task.lastRunMs = 0;
    task.bootDelayMs = bootDelayMs;
    task.priority = priority;
    task.forcePending = false;
    task.execute = execute;
    _tasks.push_back(task);
    Serial.printf("[TaskCoordinator] Registered network task '%s' (interval: %lu ms, bootDelay: %lu ms, priority: %d)\n",
                  name.c_str(), intervalMs, bootDelayMs, priority);
}

void NetworkTaskCoordinator::requestRun(const String& name) {
    for (auto& task : _tasks) {
        if (task.name == name) {
            task.forcePending = true;
            Serial.printf("[TaskCoordinator] Immediate run requested for task '%s'\n", name.c_str());
            return;
        }
    }
}

void NetworkTaskCoordinator::loop(bool isWiFiConnected) {
    if (!isWiFiConnected || _isTaskRunning) {
        return;
    }

    unsigned long now = millis();

    // Respect cooldown period between tasks to allow TLS cleanup and heap recovery
    if (_lastTaskCompletionMs > 0 && (now - _lastTaskCompletionMs < COOLDOWN_BETWEEN_TASKS_MS)) {
        return;
    }

    // Find highest priority due task
    int bestIndex = -1;
    int bestPriority = 999999;

    for (size_t i = 0; i < _tasks.size(); ++i) {
        auto& task = _tasks[i];
        bool isDue = false;

        if (task.forcePending) {
            isDue = true;
        } else if (now >= (_bootTimeMs + task.bootDelayMs)) {
            if (task.lastRunMs == 0 || (now - task.lastRunMs >= task.intervalMs)) {
                isDue = true;
            }
        }

        if (isDue && task.priority < bestPriority) {
            bestPriority = task.priority;
            bestIndex = (int)i;
        }
    }

    if (bestIndex < 0) {
        return;
    }

    auto& activeTask = _tasks[bestIndex];
    _isTaskRunning = true;

    Serial.printf("[TaskCoordinator] >>> Executing task '%s' (Heap: %u B)\n",
                  activeTask.name.c_str(), ESP.getFreeHeap());

    unsigned long taskStart = millis();
    bool success = false;
    if (activeTask.execute) {
        success = activeTask.execute();
    }
    unsigned long duration = millis() - taskStart;

    activeTask.lastRunMs = millis();
    activeTask.forcePending = false;
    _lastTaskCompletionMs = millis();
    _isTaskRunning = false;

    Serial.printf("[TaskCoordinator] <<< Finished task '%s' in %lu ms (status: %s, Heap: %u B)\n",
                  activeTask.name.c_str(), duration, success ? "OK" : "FAIL", ESP.getFreeHeap());
}

void NetworkTaskCoordinator::yieldUI() {
    if (s_instance && s_instance->_yieldCallback) {
        s_instance->_yieldCallback();
    }
    delay(2);
}
