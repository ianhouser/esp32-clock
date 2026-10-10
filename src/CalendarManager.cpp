#include "CalendarManager.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <algorithm>

CalendarManager::CalendarManager()
    : _url(""),
      _maxEvents(3),
      _updateIntervalMs(15 * 60 * 1000UL),
      _lastAttemptTime(0),
      _forceUpdateRequested(false),
      _isValid(false),
      _statusText("Not Synced") {}

void CalendarManager::begin(const String& icalUrl, int maxEvents, unsigned long updateIntervalMs) {
    _url = icalUrl;
    _maxEvents = (maxEvents > 0) ? maxEvents : 3;
    _updateIntervalMs = updateIntervalMs;
    _forceUpdateRequested = true;
    Serial.printf("[CalendarManager] Configured with URL (%u chars), poll: %lu s\n",
                  _url.length(), _updateIntervalMs / 1000UL);
}

void CalendarManager::forceUpdate() {
    _forceUpdateRequested = true;
}

bool CalendarManager::update(bool isWiFiConnected, const char* currentDateYmd) {
    if (!isWiFiConnected || _url.length() == 0) {
        return false;
    }

    unsigned long now = millis();
    unsigned long interval = _isValid ? _updateIntervalMs : (60 * 1000UL);

    if (_forceUpdateRequested || (_lastAttemptTime == 0) || (now - _lastAttemptTime >= interval)) {
        _forceUpdateRequested = false;
        _lastAttemptTime = now;
        return fetchCalendar(currentDateYmd);
    }

    return false;
}

static const char* const MONTH_NAMES[] = {
    "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

void CalendarManager::formatEventTime(const String& dtstart, String& outTimeStr, const char* currentDateYmd) {
    if (dtstart.length() < 8) {
        outTimeStr = "";
        return;
    }

    String datePart = dtstart.substring(0, 8); // YYYYMMDD
    int month = dtstart.substring(4, 6).toInt();
    int day = dtstart.substring(6, 8).toInt();

    String timePart = "";
    int tIdx = dtstart.indexOf('T');
    if (tIdx != -1 && dtstart.length() >= (unsigned int)(tIdx + 5)) {
        int hour = dtstart.substring(tIdx + 1, tIdx + 3).toInt();
        int min = dtstart.substring(tIdx + 3, tIdx + 5).toInt();
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        int hour12 = hour % 12;
        if (hour12 == 0) hour12 = 12;
        char timeBuf[16];
        snprintf(timeBuf, sizeof(timeBuf), "%d:%02d %s", hour12, min, ampm);
        timePart = timeBuf;
    }

    if (currentDateYmd && datePart == currentDateYmd) {
        outTimeStr = (timePart.length() > 0) ? ("Today at " + timePart) : "Today (All day)";
    } else {
        const char* mName = (month >= 1 && month <= 12) ? MONTH_NAMES[month] : "Date";
        char dateBuf[32];
        if (timePart.length() > 0) {
            snprintf(dateBuf, sizeof(dateBuf), "%s %d, %s", mName, day, timePart.c_str());
        } else {
            snprintf(dateBuf, sizeof(dateBuf), "%s %d", mName, day);
        }
        outTimeStr = dateBuf;
    }
}

bool CalendarManager::fetchCalendar(const char* currentDateYmd) {
    Serial.println("[CalendarManager] Connecting to iCal stream over HTTPS...");
    _statusText = "Syncing...";

    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(10); // 10 seconds timeout

    HTTPClient http;
    if (!http.begin(client, _url)) {
        Serial.println("[CalendarManager] HTTPClient begin failed");
        _statusText = "Connect Err";
        return false;
    }

    http.setUserAgent("esp32-clock/1.0");
    http.setTimeout(10000);
    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[CalendarManager] HTTP GET failed: %d\n", httpCode);
        _statusText = (httpCode > 0) ? "HTTP Error" : "Timeout";
        http.end();
        return false;
    }

    WiFiClient* stream = http.getStreamPtr();
    if (!stream) {
        Serial.println("[CalendarManager] Stream pointer was null");
        http.end();
        return false;
    }

    std::vector<CalendarEvent> futureEvents;
    std::vector<CalendarEvent> recentEvents;

    String curSummary = "";
    String curDtStart = "";
    int totalEventsParsed = 0;
    unsigned long startStream = millis();

    while (http.connected() && (stream->available() || stream->connected())) {
        if (millis() - startStream > 20000) { // Safety timeout 20s
            Serial.println("[CalendarManager] Stream read exceeded safety timeout");
            break;
        }

        if (!stream->available()) {
            delay(10);
            continue;
        }

        String line = stream->readStringUntil('\n');
        line.trim();

        if (line.startsWith("BEGIN:VEVENT")) {
            curSummary = "";
            curDtStart = "";
        } else if (line.startsWith("SUMMARY:")) {
            curSummary = line.substring(8);
        } else if (line.startsWith("DTSTART")) {
            int colon = line.indexOf(':');
            if (colon != -1) {
                curDtStart = line.substring(colon + 1);
            }
        } else if (line.startsWith("END:VEVENT")) {
            if (curSummary.length() > 0 && curDtStart.length() >= 8) {
                totalEventsParsed++;

                CalendarEvent ev;
                ev.summary = curSummary;
                ev.rawStart = curDtStart;
                formatEventTime(curDtStart, ev.timeStr, currentDateYmd);

                String dateYmd = curDtStart.substring(0, 8);
                bool isFuture = (!currentDateYmd || dateYmd >= currentDateYmd);

                if (isFuture) {
                    futureEvents.push_back(ev);
                } else {
                    recentEvents.push_back(ev);
                    // Keep recentEvents bounded to last 5
                    if (recentEvents.size() > 5) {
                        recentEvents.erase(recentEvents.begin());
                    }
                }
            }
        }
    }

    http.end();
    Serial.printf("[CalendarManager] Finished parse: %d events found (%u future, %u recent)\n",
                  totalEventsParsed, futureEvents.size(), recentEvents.size());

    _events.clear();
    if (!futureEvents.empty()) {
        std::sort(futureEvents.begin(), futureEvents.end(), [](const CalendarEvent& a, const CalendarEvent& b) {
            return a.rawStart < b.rawStart;
        });
        size_t count = std::min((size_t)_maxEvents, futureEvents.size());
        for (size_t i = 0; i < count; i++) {
            _events.push_back(futureEvents[i]);
        }
        _statusText = "Synced";
    } else if (!recentEvents.empty()) {
        // When no future events exist in calendar feed, show the most recent event from the feed
        // Reverse order so latest is first
        for (int i = recentEvents.size() - 1; i >= 0 && _events.size() < (size_t)_maxEvents; i--) {
            CalendarEvent ev = recentEvents[i];
            ev.timeStr = ev.timeStr + " (Recent)";
            _events.push_back(ev);
        }
        _statusText = "Synced (Recent)";
    } else {
        _statusText = "No Events";
    }

    _isValid = true;
    return true;
}
