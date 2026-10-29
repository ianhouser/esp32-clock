# Agent Handoff

**Last updated:** 2026-10-29 08:36 PDT — Antigravity (Fixed iCal UTC timezone parsing and flashed live firmware)

## Current Focus

- **Milestone:** [v0.2-web-control](https://github.com/ianhouser/esp32-clock/milestone/2)
- **Task:** Notifications Engine & Dynamic Bottom-Right Card Carousel
- **Branch:** `feat/11-web-control-panel`
- **Blockers:** None

## Last Three Decisions

1. Converted RFC 5545 iCalendar UTC timestamps (`Z` suffix) to local time using `localtime_r` with configured POSIX timezone, and filtered expired events by comparing end epoch against `time(nullptr)` — 2026-10-29
2. Eliminated per-loop LVGL invalidation in `updateCarousel`: cached active slot indices and gated object positioning so LVGL only touches SPI when the card actually changes — 2026-10-16
3. Added 4-second watchdog timer on `s_captureActive` in `dispFlushCallback` to prevent USB CDC serial buffer stalls from blocking the rendering engine — 2026-10-16

## What Just Happened

- Fixed iCalendar event timezone offset bug in `CalendarManager` where UTC times were formatted directly without local timezone conversion (e.g. Oct 28 5:15 PM PDT appeared as Oct 29 12:15 AM).
- Added `parseICalDateTime`, `portable_timegm`, and event expiration checking.
- Built and flashed firmware to physical ESP32-C3 device on `/dev/cu.usbmodem211101`.
- Captured live screen frame buffer (`screen_capture.png`) confirming the past Disney event was dropped and the upcoming agenda now accurately shows valid future events ("Change brita filter", "Dentist").

## Next Up

1. Test Discord webhook or mobile app notification push via `ntfy.sh`
2. Test rotation through configured cards (Agenda, Forecast, Telemetry) on live hardware

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
