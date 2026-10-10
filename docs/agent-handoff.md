# Agent Handoff

**Last updated:** 2026-10-10 10:55 PDT — Antigravity (Implemented streaming iCal CalendarManager, dynamic WeatherCard coordinates, and tile-based screen capture verification)

## Current Focus

- **Milestone:** [v0.2-web-control](https://github.com/ianhouser/esp32-clock/milestone/2)
- **Task:** Issue #11: Modular Web Control Panel & Customization Dashboard (PR #12)
- **Branch:** `feat/11-web-control-panel`
- **Blockers:** None

## Last Three Decisions

1. Built streaming line-by-line `CalendarManager` (`include/CalendarManager.h`, `src/CalendarManager.cpp`) using `WiFiClientSecure`: directly parses ~300+ KB Google Calendar iCal feeds over HTTPS without buffering in memory, preserving free heap (~80-100 KB free) — 2026-10-10
2. Decoupled `WeatherManager` from static defines to dynamically bind to `WeatherCard` configuration: uses user's configured latitude (`34.2526`), longitude (`-118.4967`), and temperature units with a 10s initial recovery retry — 2026-10-10
3. Re-architected USB CDC serial screen capture with coordinate-aware `TILED` packets (`TILE:x,y,w,h`): eliminates multi-strip raster shearing when partial rectangles/cards are flushed by LVGL — 2026-10-10

## What Just Happened

- Created `CalendarManager` streaming parser that extracts live upcoming and recent events from private Google Calendar iCal URLs and feeds them to `UIManager::updateCalendar`
- Bound `WeatherManager` to `WeatherCard` settings so custom coordinates (North Hills) and units are applied both on boot and during dynamic web reload
- Fixed font references in `UIManager.cpp` to supported `&lv_font_montserrat_12` and wired `setContainer` in `CalendarCard.h`
- Upgraded `dispFlushCallback` and `scripts/capture_screen.py` to use `TILED` framing, eliminating capture tears
- Uploaded firmware to ESP32 hardware and visually verified over serial framebuffer captures:
  - Live weather rendered: 78°F, Overcast, 42% Humidity (North Hills)
  - Live calendar rendered: Real events from user's Google Calendar feed ("Chiropractor Appointment", "Fix Bernie's Computer")
- Pushed updates to `feat/11-web-control-panel`

## Next Up

1. Add unit/integration tests for `CalendarManager` line parser edge cases
2. Conduct final PR self-review on PR #12 and hand off to user for merge approval

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
