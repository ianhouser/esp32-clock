# Implementation Plan: Live Weather Telemetry Ingestion via Open-Meteo REST API

- **Issue:** [#7 — feat: live weather telemetry ingestion via Open-Meteo REST API](https://github.com/ianhouser/esp32-clock/issues/7)
- **Branch:** `feat/#7-weather-telemetry`
- **Milestone:** `v0.1-mvp`
- **Status:** In Progress
- **Created:** 2026-10-09

---

## 1. Overview & Objectives

Now that the core ESP32 clock boots reliably, synchronizes time via SNTP, and features automatic day/night dimming, this feature implements **live weather telemetry ingestion**.

Using the free [Open-Meteo REST API](https://open-meteo.com) (which requires no API key or account), the clock queries local outdoor temperature, relative humidity, apparent temperature, and weather condition codes over WiFi. The data is rendered on the ST7789 320x240 display in a dedicated **WEATHER** card alongside network and system status.

---

## 2. Technical Design

### 2.1 Open-Meteo REST API Integration

- **Endpoint**: `http://api.open-meteo.com/v1/forecast`
- **Query Parameters**:
  - `latitude=<lat>`
  - `longitude=<lon>`
  - `current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code`
  - `temperature_unit=fahrenheit` (or `celsius`)
- **Transport**: Standard HTTP via ESP32 `HTTPClient` + `WiFiClient` (plain HTTP avoids the heavy ~30KB+ mbedTLS handshake buffer, preserving heap memory and CPU cycles).
- **JSON Parser**: `ArduinoJson` (v7) with filter or compact document to parse only the `current` object.

### 2.2 WMO Weather Interpretation

WMO Weather interpretation codes (0–99) mapped to concise strings:
- `0`: "Clear"
- `1`: "Mainly Clear"
- `2`: "Partly Cloudy"
- `3`: "Overcast"
- `45`, `48`: "Foggy"
- `51`, `53`, `55`: "Drizzle"
- `61`, `63`, `65`: "Rain"
- `71`, `73`, `75`: "Snow"
- `80`, `81`, `82`: "Showers"
- `95`, `96`, `99`: "Storm"

### 2.3 Subsystem Architecture (`WeatherManager`)

A dedicated class in `include/WeatherManager.h` and `src/WeatherManager.cpp`:

```cpp
struct WeatherData {
    float temperature = 0.0f;
    float apparentTemperature = 0.0f;
    int humidity = 0;
    int weatherCode = 0;
    const char* conditionText = "Unknown";
    bool isValid = false;
    unsigned long lastFetchTime = 0;
};

class WeatherManager {
public:
    WeatherManager();
    void begin(float latitude, float longitude, bool useFahrenheit = true, unsigned long updateIntervalMs = 900000);
    bool update(bool isWiFiConnected); // Non-blocking state check, returns true if fresh data arrived
    const WeatherData& getData() const;
    bool hasValidData() const;
    void forceUpdate();
    ...
};
```

### 2.4 Display UI Layout (320x240 Landscape)

- **Header Banner** (`Y=0..28`): Title, Day/Night pill badge, and WiFi dBm signal indicator.
- **Main Clock Card** (`Y=34..138`): Large 7-segment digital time `HH:MM:SS` + Full Date.
- **Bottom Row Cards** (`Y=144..232`):
  - **Left Card (`WEATHER`)**:
    - Title: `WEATHER`
    - Line 1: `Temp: 72°F (Partly Cloudy)`
    - Line 2: `Feels: 70°F`
    - Line 3: `Humidity: 58%`
  - **Right Card (`SYSTEM`)**:
    - Title: `SYSTEM`
    - Line 1: `WiFi: <SSID>`
    - Line 2: `IP: <Local IP>`
    - Line 3: `Sync: Locked (NTP)`

### 2.5 Configuration & Overrides

In `include/config.h`:
```cpp
#define DEFAULT_WEATHER_LAT      37.7749f   // Default: San Francisco, CA
#define DEFAULT_WEATHER_LON      -122.4194f
#define DEFAULT_WEATHER_USE_FAHR true
#define WEATHER_UPDATE_INTERVAL  (15 * 60 * 1000UL) // 15 minutes
```
In `include/secrets.h` / `include/secrets.h.example`:
Allow users to optionally specify custom coordinates:
```cpp
#define WEATHER_LAT 34.0522
#define WEATHER_LON -118.2437
```

---

## 3. Implementation Steps

1. **Step 1**: Add `bblanchon/ArduinoJson@^7.0.0` to `platformio.ini` `lib_deps`.
2. **Step 2**: Add weather configuration definitions to `include/config.h` and `include/secrets.h.example`.
3. **Step 3**: Implement `include/WeatherManager.h` and `src/WeatherManager.cpp`.
4. **Step 4**: Integrate `WeatherManager` into `src/main.cpp` and update bottom card layout to render live weather.
5. **Step 5**: Verify compilation with `pio run` and update `compile_commands.json` for IDE intellisense.
6. **Step 6**: Update `docs/INDEX.md` and `docs/agent-handoff.md`.
