#include "CalendarManager.h"
#include "NetworkTaskCoordinator.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <algorithm>

static const char* MONTH_NAMES[] = {
    "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

uint32_t CalendarManager::parseHexColor(const String& hexStr) {
    if (hexStr.length() == 0) return 0x3B82F6;
    const char* str = hexStr.c_str();
    if (str[0] == '#') str++;
    return (uint32_t)strtoul(str, nullptr, 16);
}

CalendarManager::CalendarManager()
    : _maxEvents(3),
      _updateIntervalMs(15 * 60 * 1000UL),
      _lastAttemptTime(0),
      _forceUpdateRequested(false),
      _isValid(false),
      _statusText("Not Synced") {}

void CalendarManager::begin(const std::vector<CalendarSource>& sources, int maxEvents, unsigned long updateIntervalMs) {
    _sources = sources;
    _maxEvents = (maxEvents > 0) ? maxEvents : 3;
    _updateIntervalMs = updateIntervalMs;
    _forceUpdateRequested = true;
    _isValid = false;
    Serial.printf("[CalendarManager] Configured with %d source(s), poll: %lu s\n",
                  (int)_sources.size(), _updateIntervalMs / 1000UL);
}

void CalendarManager::begin(const String& icalUrl, int maxEvents, unsigned long updateIntervalMs) {
    _sources.clear();
    if (icalUrl.length() > 0) {
        CalendarSource def;
        def.name = "Primary";
        def.url = icalUrl;
        def.color = "#3B82F6";
        _sources.push_back(def);
    }
    begin(_sources, maxEvents, updateIntervalMs);
}

void CalendarManager::forceUpdate() {
    _forceUpdateRequested = true;
}

bool CalendarManager::update(bool isWiFiConnected, const char* currentDateYmd) {
    if (!isWiFiConnected || _sources.empty()) {
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

bool CalendarManager::fetchSource(const CalendarSource& source, const char* currentDateYmd, std::vector<CalendarEvent>& outEvents) {
    if (source.url.length() == 0) {
        return false;
    }

    Serial.printf("[CalendarManager] Streaming feed '%s' (%s)...\n", source.name.c_str(), source.color.c_str());

    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(10);

    HTTPClient http;
    if (!http.begin(client, source.url)) {
        Serial.printf("[CalendarManager] HTTPClient begin failed for '%s'\n", source.name.c_str());
        return false;
    }

    http.setUserAgent("esp32-clock/1.0");
    http.setTimeout(10000);
    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[CalendarManager] HTTP GET failed for '%s': %d\n", source.name.c_str(), httpCode);
        http.end();
        return false;
    }

    WiFiClient* stream = http.getStreamPtr();
    if (!stream) {
        Serial.printf("[CalendarManager] Stream pointer null for '%s'\n", source.name.c_str());
        http.end();
        return false;
    }

    uint32_t colorHex = parseHexColor(source.color);
    String curSummary = "";
    String curDtStart = "";
    int sourceParsed = 0;
    int sourceFuture = 0;
    unsigned long startStream = millis();
    unsigned long lastYield = millis();

    while (http.connected() && (stream->available() || stream->connected())) {
        if (millis() - startStream > 25000) { // Safety timeout 25s per stream
            Serial.printf("[CalendarManager] Stream '%s' exceeded timeout\n", source.name.c_str());
            break;
        }

        if (millis() - lastYield > 40) {
            NetworkTaskCoordinator::yieldUI();
            lastYield = millis();
        }

        if (!stream->available()) {
            delay(5);
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
                sourceParsed++;

                String dateYmd = curDtStart.substring(0, 8);
                bool isFuture = (!currentDateYmd || dateYmd >= currentDateYmd);

                if (isFuture) {
                    sourceFuture++;
                    CalendarEvent ev;
                    ev.summary = curSummary;
                    ev.rawStart = curDtStart;
                    ev.colorHex = colorHex;
                    ev.sourceName = source.name;
                    formatEventTime(curDtStart, ev.timeStr, currentDateYmd);
                    outEvents.push_back(ev);
                }
            }
        }
    }

    http.end();
    Serial.printf("[CalendarManager] Feed '%s': %d events parsed, %d upcoming retained\n",
                  source.name.c_str(), sourceParsed, sourceFuture);
    return true;
}

bool CalendarManager::fetchCalendar(const char* currentDateYmd) {
    if (_sources.empty()) {
        _statusText = "No Sources";
        _events.clear();
        return false;
    }

    Serial.printf("[CalendarManager] Starting aggregation across %d calendar feed(s)...\n", (int)_sources.size());
    _statusText = "Syncing...";

    std::vector<CalendarEvent> allFutureEvents;
    bool atLeastOneSuccess = false;

    for (size_t i = 0; i < _sources.size(); ++i) {
        if (i > 0) {
            // Buffer between feeds to allow TLS cleanup and UI render
            for (int k = 0; k < 4; ++k) {
                NetworkTaskCoordinator::yieldUI();
                delay(50);
            }
        }

        bool ok = fetchSource(_sources[i], currentDateYmd, allFutureEvents);
        if (ok) {
            atLeastOneSuccess = true;
        }
    }

    if (!atLeastOneSuccess) {
        _statusText = "Sync Failed";
        return false;
    }

    // Sort all merged future events chronologically
    std::sort(allFutureEvents.begin(), allFutureEvents.end(), [](const CalendarEvent& a, const CalendarEvent& b) {
        return a.rawStart < b.rawStart;
    });

    _events.clear();
    if (!allFutureEvents.empty()) {
        size_t count = std::min((size_t)_maxEvents, allFutureEvents.size());
        for (size_t i = 0; i < count; i++) {
            _events.push_back(allFutureEvents[i]);
        }
        _isValid = true;
        _statusText = String(_events.size()) + " Events";
    } else {
        _isValid = true;
        _statusText = "No Upcoming Events";
    }

    Serial.printf("[CalendarManager] Aggregation complete: %u total future events, %u selected for display\n",
                  allFutureEvents.size(), _events.size());

    return true;
}
