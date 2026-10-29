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

static time_t portable_timegm(const struct tm* tm) {
    static const int daysBeforeMonth[] = {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
    };
    int year = tm->tm_year + 1900;
    int mon = tm->tm_mon; // 0..11
    if (mon < 0 || mon > 11) return (time_t)0;

    int y = year - (mon < 2 ? 1 : 0);
    int leapDays = (y / 4) - (y / 100) + (y / 400) - (1969 / 4 - 1969 / 100 + 1969 / 400);

    long days = (long)(year - 1970) * 365 + daysBeforeMonth[mon] + (tm->tm_mday - 1) + leapDays;
    return (time_t)(days * 86400L + tm->tm_hour * 3600L + tm->tm_min * 60L + tm->tm_sec);
}

bool CalendarManager::parseICalDateTime(const String& dtStr, ParsedEventTime& out) {
    String dt = dtStr;
    dt.trim();
    if (dt.length() < 8) return false;

    int tIdx = dt.indexOf('T');
    if (tIdx == -1) {
        // All-day event: YYYYMMDD
        out.isAllDay = true;
        int y = dt.substring(0, 4).toInt();
        int m = dt.substring(4, 6).toInt();
        int d = dt.substring(6, 8).toInt();
        if (m < 1 || m > 12 || d < 1 || d > 31) return false;

        out.localYear = y;
        out.localMonth = m;
        out.localDay = d;
        out.localHour = 0;
        out.localMin = 0;
        snprintf(out.localYmd, sizeof(out.localYmd), "%04d%02d%02d", y, m, d);

        struct tm tmLocal = {0};
        tmLocal.tm_year = y - 1900;
        tmLocal.tm_mon = m - 1;
        tmLocal.tm_mday = d;
        tmLocal.tm_hour = 0;
        tmLocal.tm_min = 0;
        tmLocal.tm_sec = 0;
        tmLocal.tm_isdst = -1;
        out.epoch = mktime(&tmLocal);
        return true;
    }

    out.isAllDay = false;
    int y = dt.substring(0, 4).toInt();
    int m = dt.substring(4, 6).toInt();
    int d = dt.substring(6, 8).toInt();
    if (m < 1 || m > 12 || d < 1 || d > 31) return false;

    if (dt.length() < (unsigned int)(tIdx + 5)) return false;
    int hour = dt.substring(tIdx + 1, tIdx + 3).toInt();
    int min = dt.substring(tIdx + 3, tIdx + 5).toInt();
    int sec = 0;
    if (dt.length() >= (unsigned int)(tIdx + 7) && isdigit(dt.charAt(tIdx + 5))) {
        sec = dt.substring(tIdx + 5, tIdx + 7).toInt();
    }

    bool isUtc = dt.endsWith("Z") || dt.endsWith("z");
    if (isUtc) {
        struct tm tmUtc = {0};
        tmUtc.tm_year = y - 1900;
        tmUtc.tm_mon = m - 1;
        tmUtc.tm_mday = d;
        tmUtc.tm_hour = hour;
        tmUtc.tm_min = min;
        tmUtc.tm_sec = sec;

        time_t utcEpoch = portable_timegm(&tmUtc);
        out.epoch = utcEpoch;

        struct tm tmLocal = {0};
        localtime_r(&utcEpoch, &tmLocal);

        out.localYear = tmLocal.tm_year + 1900;
        out.localMonth = tmLocal.tm_mon + 1;
        out.localDay = tmLocal.tm_mday;
        out.localHour = tmLocal.tm_hour;
        out.localMin = tmLocal.tm_min;
        snprintf(out.localYmd, sizeof(out.localYmd), "%04d%02d%02d",
                 out.localYear, out.localMonth, out.localDay);
    } else {
        out.localYear = y;
        out.localMonth = m;
        out.localDay = d;
        out.localHour = hour;
        out.localMin = min;
        snprintf(out.localYmd, sizeof(out.localYmd), "%04d%02d%02d", y, m, d);

        struct tm tmLocal = {0};
        tmLocal.tm_year = y - 1900;
        tmLocal.tm_mon = m - 1;
        tmLocal.tm_mday = d;
        tmLocal.tm_hour = hour;
        tmLocal.tm_min = min;
        tmLocal.tm_sec = sec;
        tmLocal.tm_isdst = -1;
        out.epoch = mktime(&tmLocal);
    }

    return true;
}

void CalendarManager::formatEventTime(const ParsedEventTime& evTime, String& outTimeStr, const char* currentDateYmd) {
    char timeBuf[16] = {0};
    if (!evTime.isAllDay) {
        int hour12 = evTime.localHour % 12;
        if (hour12 == 0) hour12 = 12;
        const char* ampm = (evTime.localHour >= 12) ? "PM" : "AM";
        snprintf(timeBuf, sizeof(timeBuf), "%d:%02d %s", hour12, evTime.localMin, ampm);
    }

    if (currentDateYmd && strcmp(evTime.localYmd, currentDateYmd) == 0) {
        outTimeStr = evTime.isAllDay ? "Today (All day)" : ("Today at " + String(timeBuf));
    } else {
        const char* mName = (evTime.localMonth >= 1 && evTime.localMonth <= 12) ? MONTH_NAMES[evTime.localMonth] : "Date";
        char dateBuf[32];
        if (!evTime.isAllDay) {
            snprintf(dateBuf, sizeof(dateBuf), "%s %d, %s", mName, evTime.localDay, timeBuf);
        } else {
            snprintf(dateBuf, sizeof(dateBuf), "%s %d", mName, evTime.localDay);
        }
        outTimeStr = dateBuf;
    }
}

void CalendarManager::formatEventTime(const String& dtstart, String& outTimeStr, const char* currentDateYmd) {
    ParsedEventTime parsed;
    if (!parseICalDateTime(dtstart, parsed)) {
        outTimeStr = "";
        return;
    }
    formatEventTime(parsed, outTimeStr, currentDateYmd);
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
    String curDtEnd = "";
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
            curDtEnd = "";
        } else if (line.startsWith("SUMMARY:")) {
            curSummary = line.substring(8);
        } else if (line.startsWith("DTSTART")) {
            int colon = line.indexOf(':');
            if (colon != -1) {
                curDtStart = line.substring(colon + 1);
            }
        } else if (line.startsWith("DTEND")) {
            int colon = line.indexOf(':');
            if (colon != -1) {
                curDtEnd = line.substring(colon + 1);
            }
        } else if (line.startsWith("END:VEVENT")) {
            if (curSummary.length() > 0 && curDtStart.length() >= 8) {
                sourceParsed++;

                ParsedEventTime parsedStart;
                if (parseICalDateTime(curDtStart, parsedStart)) {
                    time_t endEpoch = 0;
                    if (curDtEnd.length() >= 8) {
                        ParsedEventTime parsedEnd;
                        if (parseICalDateTime(curDtEnd, parsedEnd)) {
                            endEpoch = parsedEnd.epoch;
                        }
                    }
                    if (endEpoch == 0) {
                        endEpoch = parsedStart.isAllDay ? (parsedStart.epoch + 86400) : (parsedStart.epoch + 3600);
                    }

                    time_t nowEpoch = time(nullptr);
                    bool hasValidTime = (nowEpoch > 1704067200);

                    bool isFuture = false;
                    if (hasValidTime) {
                        isFuture = (endEpoch >= nowEpoch);
                    } else if (currentDateYmd && currentDateYmd[0]) {
                        isFuture = (strcmp(parsedStart.localYmd, currentDateYmd) >= 0);
                    } else {
                        isFuture = true;
                    }

                    if (isFuture) {
                        sourceFuture++;
                        CalendarEvent ev;
                        ev.summary = curSummary;
                        ev.rawStart = curDtStart;
                        ev.colorHex = colorHex;
                        ev.sourceName = source.name;
                        ev.startEpoch = parsedStart.epoch;
                        formatEventTime(parsedStart, ev.timeStr, currentDateYmd);
                        outEvents.push_back(ev);
                    }
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

    char todayYmdBuf[16] = {0};
    if (!currentDateYmd || currentDateYmd[0] == '\0') {
        time_t now = time(nullptr);
        if (now > 1704067200) {
            struct tm curTm;
            if (localtime_r(&now, &curTm)) {
                snprintf(todayYmdBuf, sizeof(todayYmdBuf), "%04d%02d%02d",
                         curTm.tm_year + 1900, curTm.tm_mon + 1, curTm.tm_mday);
                currentDateYmd = todayYmdBuf;
            }
        }
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
        return a.startEpoch < b.startEpoch;
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
