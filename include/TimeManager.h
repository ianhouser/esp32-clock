#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <esp_sntp.h>

enum class TimeSyncState {
    IDLE,
    CONNECTING_WIFI,
    WAITING_NTP,
    SYNCHRONIZED,
    ERROR_NO_WIFI,
    ERROR_TIMEOUT
};

class TimeManager {
public:
    TimeManager();

    // Initialize with WiFi credentials, NTP servers, and POSIX TZ string
    void begin(const char* ssid, const char* password,
               const char* tz = "PST8PDT,M3.2.0,M11.1.0",
               const char* ntp1 = "pool.ntp.org",
               const char* ntp2 = "time.nist.gov",
               const char* ntp3 = "time.google.com");

    // Periodic state machine update - call from loop()
    void update();

    // Force an immediate NTP resynchronization
    void forceSync();

    // Query states
    bool isConnected() const;
    bool isSynchronized() const;
    TimeSyncState getState() const;
    const char* getStateString() const;

    // Time accessors
    bool getLocalTime(struct tm& timeinfo) const;
    time_t getEpoch() const;
    String getFormattedTime(const char* format = "%H:%M:%S") const;
    String getFormattedDate(const char* format = "%A, %B %d, %Y") const;

    // Network / Diagnostic stats
    int8_t getRSSI() const;
    IPAddress getLocalIP() const;
    const char* getSSID() const;
    uint32_t getLastSyncMillis() const;
    time_t getLastSyncEpoch() const;

    // Internal SNTP callback handler
    static void sntpSyncCallback(struct timeval* tv);

private:
    const char* _ssid;
    const char* _password;
    const char* _tz;
    const char* _ntp1;
    const char* _ntp2;
    const char* _ntp3;

    TimeSyncState _state;
    unsigned long _lastWifiAttempt;
    unsigned long _wifiAttemptStart;
    unsigned long _lastNtpRequest;
    
    static volatile bool _sntpUpdated;
    static time_t _lastSyncEpoch;
    static uint32_t _lastSyncMillis;

    void connectWiFi();
    void configureSNTP();
};
