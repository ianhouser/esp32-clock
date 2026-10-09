#pragma once

#include <Arduino.h>
#include "config.h"

struct WeatherData {
    float temperature = 0.0f;
    float apparentTemperature = 0.0f;
    int humidity = 0;
    int weatherCode = 0;
    const char* conditionText = "Pending";
    bool isValid = false;
    unsigned long lastFetchTime = 0;
};

class WeatherManager {
public:
    WeatherManager();

    // Initialize with location coordinates, temperature unit preference, and poll interval
    void begin(float latitude = DEFAULT_WEATHER_LAT,
               float longitude = DEFAULT_WEATHER_LON,
               bool useFahrenheit = DEFAULT_WEATHER_USE_FAHR,
               unsigned long updateIntervalMs = WEATHER_UPDATE_INTERVAL_MS);

    // Overload supporting string literals (e.g. from secrets.h "#define WEATHER_LAT \"34.25\"")
    void begin(const char* latStr, const char* lonStr,
               bool useFahrenheit = DEFAULT_WEATHER_USE_FAHR,
               unsigned long updateIntervalMs = WEATHER_UPDATE_INTERVAL_MS) {
        begin(latStr ? atof(latStr) : DEFAULT_WEATHER_LAT,
              lonStr ? atof(lonStr) : DEFAULT_WEATHER_LON,
              useFahrenheit, updateIntervalMs);
    }

    // Call regularly in Arduino loop(). Returns true if new weather data was fetched.
    bool update(bool isWiFiConnected);

    // Force immediate weather fetch on next update() when WiFi is connected
    void forceUpdate();

    // Query weather telemetry
    const WeatherData& getData() const { return _data; }
    bool hasValidData() const { return _data.isValid; }
    bool isUsingFahrenheit() const { return _useFahrenheit; }

    // Human-friendly description of WMO weather code
    static const char* getWeatherCodeDescription(int code);

private:
    float _latitude;
    float _longitude;
    bool _useFahrenheit;
    unsigned long _updateIntervalMs;
    unsigned long _lastAttemptTime;
    bool _forceUpdateRequested;

    WeatherData _data;

    bool fetchWeather();
};
