#include "NotificationManager.h"
#include <Arduino.h>

NotificationManager* NotificationManager::s_instance = nullptr;

NotificationManager& NotificationManager::getInstance() {
    if (!s_instance) {
        s_instance = new NotificationManager();
    }
    return *s_instance;
}

NotificationManager::NotificationManager()
    : _activeAlertIdx(-1),
      _activeAlertStartMs(0),
      _unreadCount(0),
      _onUpdate(nullptr) {}

void NotificationManager::begin() {
    _history.clear();
    _activeAlertIdx = -1;
    _unreadCount = 0;
}

void NotificationManager::loop() {
    // Check if active alert has timed out
    if (_activeAlertIdx >= 0 && _activeAlertIdx < (int)_history.size()) {
        const auto& alert = _history[_activeAlertIdx];
        unsigned long elapsedSec = (millis() - _activeAlertStartMs) / 1000;
        if (elapsedSec >= (unsigned long)alert.timeoutSec) {
            _activeAlertIdx = -1;
            if (_onUpdate) {
                _onUpdate();
            }
        }
    }
}

void NotificationManager::addNotification(const String& title, const String& message,
                                         const String& source, int timeoutSec,
                                         const String& id) {
    NotificationItem item;
    item.id = id.length() > 0 ? id : String(millis());
    item.source = source.length() > 0 ? source : "ntfy";
    item.title = title.length() > 0 ? title : "Notification";
    item.message = message;
    item.receivedAtMs = millis();
    item.timeoutSec = timeoutSec > 0 ? timeoutSec : 30;
    item.read = false;

    // Prepend or insert at front of history
    _history.insert(_history.begin(), item);
    while (_history.size() > MAX_HISTORY) {
        _history.pop_back();
    }

    _unreadCount++;
    _activeAlertIdx = 0;
    _activeAlertStartMs = millis();

    Serial.printf("[NotificationManager] Ingested alert from '%s': %s - %s (timeout: %ds)\n",
                  item.source.c_str(), item.title.c_str(), item.message.c_str(), item.timeoutSec);

    if (_onUpdate) {
        _onUpdate();
    }
}

void NotificationManager::dismissActiveAlert() {
    if (_activeAlertIdx >= 0) {
        _activeAlertIdx = -1;
        if (_onUpdate) {
            _onUpdate();
        }
    }
}

void NotificationManager::clearAll() {
    _history.clear();
    _activeAlertIdx = -1;
    _unreadCount = 0;
    if (_onUpdate) {
        _onUpdate();
    }
}

bool NotificationManager::hasActiveAlert() const {
    if (_activeAlertIdx >= 0 && _activeAlertIdx < (int)_history.size()) {
        unsigned long elapsedSec = (millis() - _activeAlertStartMs) / 1000;
        return elapsedSec < (unsigned long)_history[_activeAlertIdx].timeoutSec;
    }
    return false;
}

const NotificationItem* NotificationManager::getActiveAlert() const {
    if (hasActiveAlert()) {
        return &_history[_activeAlertIdx];
    }
    return nullptr;
}
