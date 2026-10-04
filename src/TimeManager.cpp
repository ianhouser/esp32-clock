#include "TimeManager.h"

volatile bool TimeManager::_sntpUpdated = false;
time_t TimeManager::_lastSyncEpoch = 0;
uint32_t TimeManager::_lastSyncMillis = 0;

void TimeManager::sntpSyncCallback(struct timeval* tv) {
    _sntpUpdated = true;
    _lastSyncMillis = millis();
    if (tv) {
        _lastSyncEpoch = tv->tv_sec;
    } else {
        _lastSyncEpoch = time(nullptr);
    }
    Serial.printf("[TimeManager] SNTP time synchronized! Current epoch: %lld\n", (long long)_lastSyncEpoch);
}

TimeManager::TimeManager()
    : _ssid(nullptr),
      _password(nullptr),
      _tz("PST8PDT,M3.2.0,M11.1.0"),
      _ntp1("pool.ntp.org"),
      _ntp2("time.nist.gov"),
      _ntp3("time.google.com"),
      _state(TimeSyncState::IDLE),
      _lastWifiAttempt(0),
      _wifiAttemptStart(0),
      _lastNtpRequest(0) {}

void TimeManager::begin(const char* ssid, const char* password,
                        const char* tz,
                        const char* ntp1,
                        const char* ntp2,
                        const char* ntp3) {
    _ssid = ssid;
    _password = password;
    if (tz && strlen(tz) > 0) _tz = tz;
    if (ntp1 && strlen(ntp1) > 0) _ntp1 = ntp1;
    if (ntp2 && strlen(ntp2) > 0) _ntp2 = ntp2;
    if (ntp3 && strlen(ntp3) > 0) _ntp3 = ntp3;

    Serial.println("[TimeManager] Initializing subsystem...");
    Serial.printf("[TimeManager] Target SSID: %s\n", _ssid ? _ssid : "(null)");
    Serial.printf("[TimeManager] POSIX Timezone: %s\n", _tz);
    Serial.printf("[TimeManager] Primary NTP: %s\n", _ntp1);

    // Register SNTP callback before configuring time
    sntp_set_time_sync_notification_cb(TimeManager::sntpSyncCallback);

    connectWiFi();
}

void TimeManager::connectWiFi() {
    if (!_ssid || strlen(_ssid) == 0 || strcmp(_ssid, "YOUR_WIFI_SSID") == 0) {
        Serial.println("[TimeManager] Notice: Placeholder or empty SSID configured. Running in offline/unprovisioned mode.");
        _state = TimeSyncState::ERROR_NO_WIFI;
        return;
    }

    Serial.printf("[TimeManager] Connecting to WiFi SSID '%s'...\n", _ssid);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);
    _wifiAttemptStart = millis();
    _lastWifiAttempt = millis();
    _state = TimeSyncState::CONNECTING_WIFI;
}

void TimeManager::configureSNTP() {
    Serial.println("[TimeManager] Configuring SNTP servers and timezone...");
    configTzTime(_tz, _ntp1, _ntp2, _ntp3);
    _lastNtpRequest = millis();
    _state = TimeSyncState::WAITING_NTP;
}

void TimeManager::forceSync() {
    if (isConnected()) {
        configureSNTP();
    }
}

void TimeManager::update() {
    wl_status_t wifiStatus = WiFi.status();

    switch (_state) {
        case TimeSyncState::IDLE:
            break;

        case TimeSyncState::CONNECTING_WIFI:
            if (wifiStatus == WL_CONNECTED) {
                Serial.printf("[TimeManager] WiFi Connected! IP: %s | RSSI: %d dBm\n",
                              WiFi.localIP().toString().c_str(), WiFi.RSSI());
                configureSNTP();
            } else if (millis() - _wifiAttemptStart > 15000) {
                Serial.println("[TimeManager] WiFi connection attempt timed out.");
                _state = TimeSyncState::ERROR_TIMEOUT;
                _lastWifiAttempt = millis();
            }
            break;

        case TimeSyncState::WAITING_NTP:
            if (wifiStatus != WL_CONNECTED) {
                Serial.println("[TimeManager] WiFi dropped while waiting for NTP.");
                _state = TimeSyncState::ERROR_NO_WIFI;
                _lastWifiAttempt = millis();
                break;
            }

            if (_sntpUpdated) {
                _sntpUpdated = false;
                _state = TimeSyncState::SYNCHRONIZED;
                Serial.println("[TimeManager] Transitioned to SYNCHRONIZED state.");
            } else if (millis() - _lastNtpRequest > 20000) {
                // Retry NTP if taking longer than 20 seconds
                Serial.println("[TimeManager] NTP response timed out, re-requesting...");
                configureSNTP();
            }
            break;

        case TimeSyncState::SYNCHRONIZED:
            if (wifiStatus != WL_CONNECTED) {
                Serial.println("[TimeManager] WiFi connection lost. Time keeping continues via internal RTC.");
                _state = TimeSyncState::ERROR_NO_WIFI;
                _lastWifiAttempt = millis();
            } else if (_sntpUpdated) {
                _sntpUpdated = false; // Acknowledge periodic background re-syncs
            }
            break;

        case TimeSyncState::ERROR_NO_WIFI:
        case TimeSyncState::ERROR_TIMEOUT:
            // Retry connection every 15 seconds
            if (_ssid && strlen(_ssid) > 0 && strcmp(_ssid, "YOUR_WIFI_SSID") != 0) {
                if (millis() - _lastWifiAttempt > 15000) {
                    Serial.println("[TimeManager] Attempting WiFi reconnection...");
                    connectWiFi();
                }
            }
            break;
    }
}

bool TimeManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

bool TimeManager::isSynchronized() const {
    time_t now = time(nullptr);
    // Any epoch greater than 2024-01-01 (1704067200) indicates valid time was received
    return now > 1704067200;
}

TimeSyncState TimeManager::getState() const {
    return _state;
}

const char* TimeManager::getStateString() const {
    switch (_state) {
        case TimeSyncState::IDLE:            return "Idle";
        case TimeSyncState::CONNECTING_WIFI: return "Connecting WiFi";
        case TimeSyncState::WAITING_NTP:      return "Syncing NTP";
        case TimeSyncState::SYNCHRONIZED:    return "Synchronized";
        case TimeSyncState::ERROR_NO_WIFI:   return "Offline";
        case TimeSyncState::ERROR_TIMEOUT:   return "WiFi Timeout";
        default:                             return "Unknown";
    }
}

bool TimeManager::getLocalTime(struct tm& timeinfo) const {
    return ::getLocalTime(&timeinfo, 10);
}

time_t TimeManager::getEpoch() const {
    return time(nullptr);
}

String TimeManager::getFormattedTime(const char* format) const {
    struct tm timeinfo;
    if (!getLocalTime(timeinfo)) {
        return String("--:--:--");
    }
    char buf[64];
    strftime(buf, sizeof(buf), format, &timeinfo);
    return String(buf);
}

String TimeManager::getFormattedDate(const char* format) const {
    struct tm timeinfo;
    if (!getLocalTime(timeinfo)) {
        return String("No Date Available");
    }
    char buf[64];
    strftime(buf, sizeof(buf), format, &timeinfo);
    return String(buf);
}

int8_t TimeManager::getRSSI() const {
    return isConnected() ? WiFi.RSSI() : 0;
}

IPAddress TimeManager::getLocalIP() const {
    return isConnected() ? WiFi.localIP() : IPAddress(0, 0, 0, 0);
}

const char* TimeManager::getSSID() const {
    return _ssid ? _ssid : "";
}

uint32_t TimeManager::getLastSyncMillis() const {
    return _lastSyncMillis;
}

time_t TimeManager::getLastSyncEpoch() const {
    return _lastSyncEpoch;
}
