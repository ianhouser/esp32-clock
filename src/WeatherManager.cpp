#include "WeatherManager.h"
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

WeatherManager::WeatherManager()
    : _latitude(DEFAULT_WEATHER_LAT),
      _longitude(DEFAULT_WEATHER_LON),
      _useFahrenheit(DEFAULT_WEATHER_USE_FAHR),
      _updateIntervalMs(WEATHER_UPDATE_INTERVAL_MS),
      _lastAttemptTime(0),
      _forceUpdateRequested(true) {}

void WeatherManager::begin(float latitude, float longitude, bool useFahrenheit, unsigned long updateIntervalMs) {
    _latitude = latitude;
    _longitude = longitude;
    _useFahrenheit = useFahrenheit;
    _updateIntervalMs = updateIntervalMs;
    _lastAttemptTime = 0;
    _forceUpdateRequested = true; // Trigger fetch on first WiFi connection
    Serial.printf("[WeatherManager] Initialized for Lat: %.4f, Lon: %.4f (%s, poll: %lu s)\n",
                  _latitude, _longitude, _useFahrenheit ? "°F" : "°C", _updateIntervalMs / 1000UL);
}

void WeatherManager::forceUpdate() {
    _forceUpdateRequested = true;
}

bool WeatherManager::update(bool isWiFiConnected) {
    if (!isWiFiConnected) {
        return false;
    }

    unsigned long now = millis();
    unsigned long interval = _data.isValid ? _updateIntervalMs : (10 * 1000UL);

    if (_forceUpdateRequested || (now - _lastAttemptTime >= interval) || (_lastAttemptTime == 0)) {
        _forceUpdateRequested = false;
        _lastAttemptTime = now;
        return fetchWeather();
    }

    return false;
}

bool WeatherManager::fetchWeather() {
    WiFiClient client;
    HTTPClient http;

    char url[320];
    snprintf(url, sizeof(url),
             "http://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f&current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code&daily=temperature_2m_max,temperature_2m_min,weather_code&forecast_days=3%s",
             _latitude, _longitude, _useFahrenheit ? "&temperature_unit=fahrenheit" : "");

    Serial.printf("[WeatherManager] Querying Open-Meteo: %s\n", url);
    _data.statusText = "Contacting API...";

    if (!http.begin(client, url)) {
        Serial.println("[WeatherManager] HTTP begin failed");
        _data.statusText = "Net Connect Err";
        return false;
    }

    http.setUserAgent("esp32-clock/1.0");
    http.setTimeout(7000);
    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[WeatherManager] HTTP GET failed with code: %d (%s)\n",
                      httpCode, http.errorToString(httpCode).c_str());
        _data.statusText = (httpCode > 0) ? "HTTP Error" : "Conn Timeout";
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    if (payload.length() == 0) {
        Serial.println("[WeatherManager] HTTP response payload was empty");
        _data.statusText = "Empty Response";
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        Serial.printf("[WeatherManager] JSON parsing error: %s (payload length: %u)\n",
                      error.c_str(), payload.length());
        _data.statusText = "JSON Parse Err";
        return false;
    }

    JsonObject current = doc["current"];
    if (current.isNull()) {
        Serial.println("[WeatherManager] Response missing 'current' weather object");
        _data.statusText = "Data Format Err";
        return false;
    }

    _data.temperature = current["temperature_2m"] | 0.0f;
    _data.apparentTemperature = current["apparent_temperature"] | 0.0f;
    _data.humidity = current["relative_humidity_2m"] | 0;
    _data.weatherCode = current["weather_code"] | 0;
    _data.conditionText = getWeatherCodeDescription(_data.weatherCode);

    // Parse Daily High & Low temperatures (Today = index 0)
    if (!doc["daily"]["temperature_2m_max"].isNull()) {
        _data.tempMax = doc["daily"]["temperature_2m_max"][0] | _data.temperature;
        _data.tempMin = doc["daily"]["temperature_2m_min"][0] | _data.temperature;

        // Parse Next 2 Days Forecast (index 1 = tomorrow, index 2 = day after)
        for (int i = 0; i < 2; i++) {
            int dayIdx = i + 1;
            if (!doc["daily"]["temperature_2m_max"][dayIdx].isNull()) {
                _data.forecast[i].tempMax = doc["daily"]["temperature_2m_max"][dayIdx] | 0.0f;
                _data.forecast[i].tempMin = doc["daily"]["temperature_2m_min"][dayIdx] | 0.0f;
                _data.forecast[i].weatherCode = doc["daily"]["weather_code"][dayIdx] | 0;
                _data.forecast[i].isValid = true;
            } else {
                _data.forecast[i].isValid = false;
            }
        }
    } else {
        _data.tempMax = _data.temperature;
        _data.tempMin = _data.temperature;
        _data.forecast[0].isValid = false;
        _data.forecast[1].isValid = false;
    }

    _data.statusText = "OK";
    _data.isValid = true;
    _data.lastFetchTime = millis();

    Serial.printf("[WeatherManager] Success: %.1f%s (%s), Today H: %.1f%s L: %.1f%s\n",
                  _data.temperature, _useFahrenheit ? "F" : "C",
                  _data.conditionText,
                  _data.tempMax, _useFahrenheit ? "F" : "C",
                  _data.tempMin, _useFahrenheit ? "F" : "C");
    if (_data.forecast[0].isValid) {
        Serial.printf("[WeatherManager] Tomorrow: H: %.1f%s L: %.1f%s (code %d)\n",
                      _data.forecast[0].tempMax, _useFahrenheit ? "F" : "C",
                      _data.forecast[0].tempMin, _useFahrenheit ? "F" : "C",
                      _data.forecast[0].weatherCode);
    }

    return true;
}

const char* WeatherManager::getWeatherCodeDescription(int code) {
    switch (code) {
        case 0:  return "Clear Sky";
        case 1:  return "Mainly Clear";
        case 2:  return "Partly Cloudy";
        case 3:  return "Overcast";
        case 45: return "Fog";
        case 48: return "Depositing Fog";
        case 51: return "Light Drizzle";
        case 53: return "Moderate Drizzle";
        case 55: return "Dense Drizzle";
        case 56:
        case 57: return "Freezing Drizzle";
        case 61: return "Slight Rain";
        case 63: return "Moderate Rain";
        case 65: return "Heavy Rain";
        case 66:
        case 67: return "Freezing Rain";
        case 71: return "Slight Snow";
        case 73: return "Moderate Snow";
        case 75: return "Heavy Snow";
        case 77: return "Snow Grains";
        case 80: return "Light Showers";
        case 81: return "Mod Showers";
        case 82: return "Violent Showers";
        case 85:
        case 86: return "Snow Showers";
        case 95: return "Thunderstorm";
        case 96:
        case 99: return "Storm w/ Hail";
        default: return "Unknown";
    }
}
