#pragma once

#include <Arduino.h>
#include <vector>

struct CalendarEvent {
    String summary;
    String timeStr;
    String rawStart;
};

class CalendarManager {
public:
    CalendarManager();

    void begin(const String& icalUrl, int maxEvents = 3, unsigned long updateIntervalMs = 15 * 60 * 1000UL);
    bool update(bool isWiFiConnected, const char* currentDateYmd = nullptr);
    void forceUpdate();
    bool executeFetch(const char* currentDateYmd = nullptr) { return fetchCalendar(currentDateYmd); }

    bool hasValidData() const { return _isValid; }
    const std::vector<CalendarEvent>& getEvents() const { return _events; }
    const String& getStatusText() const { return _statusText; }

private:
    String _url;
    int _maxEvents;
    unsigned long _updateIntervalMs;
    unsigned long _lastAttemptTime;
    bool _forceUpdateRequested;
    bool _isValid;
    String _statusText;

    std::vector<CalendarEvent> _events;

    bool fetchCalendar(const char* currentDateYmd);
    void formatEventTime(const String& dtstart, String& outTimeStr, const char* currentDateYmd);
};
