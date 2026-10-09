# Agent Handoff

**Last updated:** 2026-10-09 12:40 PDT — Antigravity (Color order fixed to BGR, serial commands & capture tool added, hardware captures verified)

## Current Focus

- **Milestone:** [v0.1-mvp](https://github.com/ianhouser/esp32-clock/milestone/1)
- **Task:** Task 5: LVGL Modern Digital Dashboard (Issue #9)
- **Blockers:** Awaiting human user PR review and merge

## Last Three Decisions

1. Strictly confirmed workflow rule: AI agents must NEVER merge or close PRs. PR review, verification, and merging are exclusively performed by the human user — 2026-10-09
2. Configured ST7789 `TFT_RGB_ORDER TFT_BGR` in `include/User_Setup.h` to match GMT020-02-7P physical subpixels, eliminating Red/Blue swap and restoring vibrant Cyan and Deep Slate Navy palettes — 2026-10-09
3. Built zero-overhead framebuffer streaming via LVGL `dispFlushCallback` + native POSIX serial capture tool (`scripts/capture_screen.py`) to inspect live screen pixels directly over USB CDC — 2026-10-09

## What Just Happened

- Resolved color inversion: configured `TFT_RGB_ORDER TFT_BGR` in `include/User_Setup.h` and `LV_COLOR_16_SWAP 1` in `include/lv_conf.h`
- Upgraded hero clock font to `lv_font_montserrat_48` for high-definition digital clock display
- Added interactive serial commands in `src/main.cpp`: `day`, `night`, `auto`, and `cap`
- Built `scripts/capture_screen.py` using native macOS POSIX serial (`termios`) and verified live screen captures for both Day and Night modes
- Confirmed pixel-perfect rendering of Concept A dashboard
- Updated [PR #10](https://github.com/ianhouser/esp32-clock/pull/10) with all fixes and tools

## Next Up (Human User Action)

1. Verify the display looks correct in person on the physical device
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


