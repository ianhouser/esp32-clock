# KidClock — Architecture

## System Overview

KidClock is a standalone WiFi-connected device running on an ESP32-C3 Super Mini. It drives a 2" TFT display showing time and weather, and serves a web control panel for configuration.

```
┌─────────────────────────────────────────────┐
│              ESP32-C3 Super Mini            │
│                                             │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  │
│  │  Screen   │  │  Time    │  │ Weather  │  │
│  │  Manager  │  │ Manager  │  │ Client   │  │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘  │
│       │              │              │        │
│  ┌────▼─────┐  ┌────▼─────┐  ┌────▼─────┐  │
│  │ TFT_eSPI │  │   NTP    │  │Open-Meteo│  │
│  │ Display  │  │  Sync    │  │  HTTPS   │  │
│  └────┬─────┘  └──────────┘  └──────────┘  │
│       │                                      │
│  ┌────▼─────────────────────────────────┐   │
│  │   ESPAsyncWebServer (port 80)        │   │
│  │   REST API + Static files (LittleFS) │   │
│  └──────────────────────────────────────┘   │
│                                             │
│  ┌──────────────────────────────────────┐   │
│  │   Settings (LittleFS JSON)           │   │
│  │   WiFi Manager (captive portal)      │   │
│  │   OTA Update Service                 │   │
│  └──────────────────────────────────────┘   │
└─────────────────────────────────────────────┘
         │              │              │
    SPI bus         WiFi (STA)    WiFi (AP fallback)
         │              │              │
    ┌────▼────┐   ┌─────▼─────┐  ┌────▼──────┐
    │ 2" TFT  │   │ NTP / API │  │  Phone /  │
    │ ST7789  │   │ Servers   │  │  Browser  │
    └─────────┘   └───────────┘  └───────────┘
```

## Module Responsibilities

| Module | File(s) | Responsibility |
|--------|---------|---------------|
| **main** | `src/main.cpp` | Setup, main loop, screen manager orchestration |
| **display** | `src/display.cpp/h` | TFT_eSPI initialization, sprite helpers, brightness control |
| **screen_manager** | `src/screen_manager.cpp/h` | Manages active screen, transitions, night mode switching |
| **clock_screen** | `src/screens/clock_screen.cpp/h` | Renders time, date, day-of-week |
| **weather_screen** | `src/screens/weather_screen.cpp/h` | Renders current weather overlay or detail view |
| **time_manager** | `src/time_manager.cpp/h` | NTP sync, timezone handling, time formatting |
| **weather** | `src/weather.cpp/h` | Open-Meteo HTTPS client, JSON parsing, caching |
| **wifi_manager** | `src/wifi_manager.cpp/h` | STA connection, AP fallback, captive portal |
| **web_server** | `src/web_server.cpp/h` | REST API endpoints, serves static web UI from LittleFS |
| **settings** | `src/settings.cpp/h` | Load/save JSON config to LittleFS, defaults |
| **config** | `include/config.h` | Pin definitions, constants, compile-time defaults |

## Screen Architecture

Each screen implements a common interface:

```cpp
class Screen {
public:
    virtual void setup() = 0;           // One-time init
    virtual void update() = 0;          // Called every loop iteration
    virtual void draw(TFT_eSprite&) = 0; // Render to sprite buffer
    virtual ~Screen() = default;
};
```

The `ScreenManager` holds a stack/map of screens and delegates `update()` and `draw()` calls. Night mode is handled by switching to a `NightScreen` variant or by passing a night-mode flag to the active screen's `draw()`.

## Data Flow

### Time
```
Boot → WiFi connect → configTime(gmtOffset, dstOffset, ntpServer)
     → localtime() available → re-sync every 6 hours
```

### Weather
```
Boot → WiFi connect → fetch Open-Meteo (HTTPS GET, JSON)
     → parse temp + WMO weather code → cache in RAM
     → re-fetch every 15 minutes
     → display reads from cache
```

### Settings
```
LittleFS:/config.json → loaded at boot → held in RAM struct
Web UI changes → POST /api/settings → update RAM + write LittleFS
```

### Night Mode
```
time_manager provides current hour → screen_manager checks against
settings.night_start / settings.night_end → switches draw style
Future: ambient light sensor provides lux → override time-based trigger
```

## Pin Allocation

| GPIO | Assignment | Notes |
|------|-----------|-------|
| 2 | TFT DC | Data/Command (strapping pin, safe for output) |
| 3 | TFT RST | Reset |
| 6 | TFT SCK | SPI clock |
| 7 | TFT MOSI | SPI data |
| 10 | TFT CS | Chip select |
| 4 | _Reserved_ | Future I²C SDA (sensors) |
| 5 | _Reserved_ | Future I²C SCL (sensors) |
| 0 | _Reserved_ | Future RGBIC LED data |
| 1 | _Reserved_ | Future ambient light sensor (ADC) |
| 8 | Onboard LED | Status indicator |
| 9 | BOOT button | Do not use for peripherals |

## REST API (Phase 1)

| Method | Endpoint | Body | Purpose |
|--------|----------|------|---------|
| GET | `/api/settings` | — | Return current settings JSON |
| POST | `/api/settings` | JSON | Update settings, save to LittleFS |
| GET | `/api/status` | — | WiFi status, uptime, free heap, current time |
| GET | `/api/weather` | — | Return cached weather data |
| GET | `/` | — | Serve web UI (index.html from LittleFS) |

## Dependencies (PlatformIO libs)

| Library | Purpose |
|---------|---------|
| TFT_eSPI | ST7789 display driver |
| ESPAsyncWebServer | Async HTTP server |
| AsyncTCP | TCP layer for async web server |
| ArduinoJson | JSON parsing/serialization |
| LittleFS (built-in) | Filesystem for config + web assets |
| WiFi (built-in) | Network connectivity |
| HTTPClient (built-in) | Open-Meteo API calls |
| ArduinoOTA (built-in) | Over-the-air firmware updates |
