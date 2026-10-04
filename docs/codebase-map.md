# esp32-clock — Codebase Map

> Where code and key files live. Updated as modules are implemented.

## Status: Pre-implementation (scaffold only)

Source files will be created during Phase 1 development. This map will be updated as each module is implemented.

## Planned Module Map

| Module | Header | Source | Purpose |
|--------|--------|--------|---------|
| config | `include/config.h` | — | Pin defs, constants |
| display setup | `include/User_Setup.h` | — | TFT_eSPI compile-time config |
| main | — | `src/main.cpp` | Entry point, setup/loop |
| display | `src/display.h` | `src/display.cpp` | TFT init, sprites, brightness |
| screen_manager | `src/screen_manager.h` | `src/screen_manager.cpp` | Screen lifecycle, night mode |
| screen (base) | `src/screens/screen.h` | — | Abstract screen interface |
| clock_screen | `src/screens/clock_screen.h` | `src/screens/clock_screen.cpp` | Clock face rendering |
| weather_screen | `src/screens/weather_screen.h` | `src/screens/weather_screen.cpp` | Weather display |
| time_manager | `src/time_manager.h` | `src/time_manager.cpp` | NTP, timezone, formatting |
| weather | `src/weather.h` | `src/weather.cpp` | Open-Meteo client |
| wifi_manager | `src/wifi_manager.h` | `src/wifi_manager.cpp` | WiFi STA + AP portal |
| web_server | `src/web_server.h` | `src/web_server.cpp` | REST API, static files |
| settings | `src/settings.h` | `src/settings.cpp` | LittleFS JSON config |

## Web UI (LittleFS)

| File | Purpose |
|------|---------|
| `data/index.html` | Control panel HTML |
| `data/style.css` | Control panel styles |
| `data/app.js` | Control panel logic |
| `data/config.json` | Default settings template |

## Config & Build

| File | Purpose |
|------|---------|
| `platformio.ini` | Board, framework, libs, partition table |
| `.gitignore` | Excludes build artifacts, IDE files |
| `AGENTS.md` | Agent hub (binding decisions, stop rules) |
| `CLAUDE.md` | Agent hub (mirror of AGENTS.md) |
| `CONTRIBUTING.md` | Git workflow, coding standards |
| `README.md` | Project overview for humans |
