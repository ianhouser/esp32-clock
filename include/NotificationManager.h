#pragma once

#include <Arduino.h>
#include <vector>
#include <functional>

struct NotificationItem {
    String id;
    String source;     // e.g. "ntfy", "discord", "phone", "local"
    String title;
    String message;
    unsigned long receivedAtMs;
    int timeoutSec;
    bool read;
};

class NotificationManager {
public:
    static NotificationManager& getInstance();

    void begin();
    void loop();

    // Ingest new notification
    void addNotification(const String& title, const String& message,
                         const String& source = "ntfy", int timeoutSec = 30,
                         const String& id = "");

    // Clear / Dismiss
    void dismissActiveAlert();
    void clearAll();

    // Query state
    bool hasActiveAlert() const;
    const NotificationItem* getActiveAlert() const;
    int getUnreadCount() const { return _unreadCount; }
    const std::vector<NotificationItem>& getHistory() const { return _history; }

    // Register callback when notification state changes (for UI)
    void onNotificationUpdate(std::function<void()> cb) { _onUpdate = cb; }

private:
    NotificationManager();
    static NotificationManager* s_instance;

    std::vector<NotificationItem> _history;
    static const size_t MAX_HISTORY = 10;

    int _activeAlertIdx;
    unsigned long _activeAlertStartMs;
    int _unreadCount;

    std::function<void()> _onUpdate;
};
