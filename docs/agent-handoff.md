# Agent Handoff

**Last updated:** 2026-10-09 15:50 PDT — Antigravity (Hardware subpixel mapping locked to MADCTL 0x68, POSIX TZ applied, day palette verified on device)

## Current Focus

- **Milestone:** [v0.1-mvp](https://github.com/ianhouser/esp32-clock/milestone/1)
- **Task:** Task 5: LVGL Modern Digital Dashboard (Issue #9)
- **Blockers:** Awaiting human user PR review and merge

## Last Three Decisions

1. Resolved hardware color inversion: Enforced ST7789 MADCTL register `0x68` (MX | MV | BGR subpixel ordering) on boot in `setup()`, aligning physical GMT020-02-7P display subpixels with LVGL RGB565 buffer — 2026-10-09
2. Fixed automatic Night mode false-trigger: Added explicit `setenv("TZ", _tz, 1); tzset();` in `TimeManager` so local hour evaluates in PDT (`15:xx`) instead of UTC (`22:xx`), preventing daytime transition to night mode — 2026-10-09
3. Overhauled dashboard layout: Removed top header bar, added 12-hour hero time card with AM/PM pill, and replaced system/net pane with a 2-day Open-Meteo weather forecast card — 2026-10-09

## What Just Happened

- Removed top header bar to maximize canvas space for time and weather
- Redesigned hero clock card: 12-hour format (`3:47:22`), AM/PM pill badge, centered full date
- Replaced system/net card with Open-Meteo 2-day forecast card (tomorrow & day after with day names, vector weather icons, High/Low temps)
- Updated `TimeManager.cpp` with explicit `setenv("TZ")` and `tzset()` calls
- Configured `.clangd` to strip RISC-V specific GCC flags and eliminate IDE diagnostics on macOS
- Enforced ST7789 MADCTL register `0x68` in `main.cpp` and verified correct Day colors (Dark Slate + Cyan + Golden Yellow Sun) on physical hardware
- Committed and pushed updates to `feat/#9-lvgl-modern-dashboard` on [PR #10](https://github.com/ianhouser/esp32-clock/pull/10)

## Next Up (Human User Action)

1. Verify physical display on desk
2. Human user reviews and merges [PR #10](https://github.com/ianhouser/esp32-clock/pull/10) into `main`

## Open Risks / Watch List

- LVGL partial double-buffering (320×20) leaves ample headroom (>227 KB RAM free), but watch heap if adding complex full-screen images
- 7-pin GMT020-02-7P TFT module connects backlight directly to VCC; Day/Night theme switching uses LVGL styles

## Pointers

- **INDEX:** `docs/INDEX.md`
- **UI Concepts:** `docs/ui-design-concepts.md`
- **Issue:** https://github.com/ianhouser/esp32-clock/issues/9
- **PR:** https://github.com/ianhouser/esp32-clock/pull/10
- **Plan:** `docs/plans/2026-10-09-issue-9-lvgl-modern-dashboard.md`
- **Hardware wiring:** `docs/hardware-guide.md`


