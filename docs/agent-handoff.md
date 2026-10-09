# Agent Handoff

**Last updated:** 2026-10-09 16:45 PDT — Antigravity (Implemented modular web control panel, CardRegistry, dynamic theming, and 320x240 screen simulation for Issue #11)

## Current Focus

- **Milestone:** [v0.2-web-control](https://github.com/ianhouser/esp32-clock/milestone/2)
- **Task:** Issue #11: Modular Web Control Panel & Customization Dashboard
- **Branch:** `feat/11-web-control-panel`
- **Blockers:** None

## Last Three Decisions

1. Architected modular `Card` interface and `CardRegistry` to decouple cards from core firmware: allows existing cards (`clock`, `weather`, `forecast`, `system`) and upcoming cards (`calendar`, `notifications`) to be toggled, reordered, and configured without modifying core engines — 2026-10-09
2. Migrated partition table to custom 4MB layout (`partitions.csv`): provides 2MB app partition (reducing flash consumption from 95.4% down to 59.6%) and ~1.87MB LittleFS partition for web assets and dynamic `/config.json` — 2026-10-09
3. Built Single-Page Control Panel in `data/` (`index.html`, `style.css`, `app.js`): features a responsive dark glassmorphic dashboard, real-time 320×240 HTML5 canvas preview simulating the ST7789 display, palette preset studio, and dynamic card reordering — 2026-10-09

## What Just Happened

- Created GitHub Issue [#11](https://github.com/ianhouser/esp32-clock/issues/11) under milestone `v0.2-web-control` and branched to `feat/11-web-control-panel`
- Implemented `Card` base class (`include/cards/Card.h`) and `CardRegistry` (`include/cards/CardRegistry.h`, `src/cards/CardRegistry.cpp`)
- Created concrete card implementations: `ClockCard`, `WeatherCard`, `ForecastCard`, `SystemCard`, plus extensible templates for `CalendarCard` and `NotificationCard`
- Created `ConfigManager` (`include/ConfigManager.h`, `src/ConfigManager.cpp`) with LittleFS mounting and dynamic hex RGB565 theme conversion
- Integrated `ESPAsyncWebServer` & `AsyncTCP` in `WebServerManager` with REST endpoints (`GET /api/config`, `POST /api/config`, `GET /api/status`, `POST /api/restart`) and mDNS discovery at `http://esp32-clock.local`
- Built modern glassmorphic web control panel SPA in `data/` with dynamic card manager, theme studio, and live 320x240 canvas screen simulator
- Configured 4MB custom partition table (`partitions.csv`), reducing flash usage to 59.6%
- Verified with `pio run` (firmware) and `pio run -t buildfs` (LittleFS filesystem packaging) with 0 errors

## Next Up

1. Test flashing firmware and LittleFS filesystem image to physical hardware (`pio run -t upload && pio run -t uploadfs`)
2. Access `http://esp32-clock.local` in local browser to test live control and configuration
3. Push branch and open Pull Request for Issue #11

## Open Risks / Watch List

- Ensure `LittleFS` is flashed via `pio run -t uploadfs` whenever web assets in `data/` are modified
- Monitor heap during simultaneous web browsing and LVGL rendering (currently ~225 KB free SRAM)

## Pointers

- **INDEX:** `docs/INDEX.md`
- **Issue:** https://github.com/ianhouser/esp32-clock/issues/11
- **PR:** https://github.com/ianhouser/esp32-clock/pull/12
- **Plan:** `docs/plans/2026-10-09-issue-11-modular-web-control-panel.md`
- **Control Panel Assets:** `data/index.html`, `data/style.css`, `data/app.js`
- **Hardware wiring:** `docs/hardware-guide.md`
