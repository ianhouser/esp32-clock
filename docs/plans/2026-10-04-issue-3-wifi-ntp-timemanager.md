# Implementation Plan: Issue #3 — WiFi Connection & NTP Time Synchronization

**Issue:** [#3 (WiFi Connection & NTP Time Synchronization)](https://github.com/ianhouser/esp32-clock/issues/3)  
**Branch:** `feat/#3-time-manager-ntp`

## Objective
Implement modular WiFi connection management and SNTP time synchronization (`TimeManager`) on the ESP32-C3 Super Mini, and update the ST7789 display firmware to render an active digital clock interface with live status telemetry.

## Task Breakdown

### Task 1: Branch & Secret Management Setup
- [x] Create GitHub issue #3 and associate with milestone `v0.1-mvp`.
- [x] Check out feature branch `feat/#3-time-manager-ntp`.
- [x] Verify `.gitignore` rules protect credential files (`secrets.h`).
- [x] Create `include/secrets.h.example` with template WiFi credentials, NTP server definitions, and POSIX timezone documentation.

### Task 2: Modular `TimeManager` Subsystem
- **Files:** `include/TimeManager.h`, `src/TimeManager.cpp`
- [x] Non-blocking WiFi association with retry/reconnect backoff.
- [x] POSIX timezone and SNTP configuration (`configTzTime`, `sntp_set_time_sync_notification_cb`).
- [x] Local time retrieval (`getLocalTime`, `struct tm`, formatted time/date strings).
- [x] State tracking (`TimeSyncState` enum: IDLE, CONNECTING_WIFI, WAITING_NTP, SYNCHRONIZED, ERROR_NO_WIFI, ERROR_TIMEOUT).
- [x] Network diagnostics (SSID, IP address, RSSI dBm, sync status).

### Task 3: Digital Clock Display Firmware
- **File:** `src/main.cpp`
- [x] Safe fallback definitions if `include/secrets.h` is omitted.
- [x] Modern dark slate UI layout (320x240 landscape):
  - Top header with WiFi status pill/badge and signal strength.
  - Center clock card rendering large digital time (HH:MM:SS) and date.
  - Bottom split telemetry cards for Network (SSID, IP, RSSI) and System (Sync state, Heap, Uptime).
- [x] Flicker-free text updates using TFT_eSPI text padding and conditional redraws.

### Task 4: Compilation & Build Verification
- [x] Run `pio run` to verify successful compilation with 0 code errors and 0 warnings.
- [x] Verified memory footprint (RAM: ~12%, Flash: ~58%).

### Task 5: Documentation & PR Submission
- [x] Update `docs/agent-handoff.md`.
- [x] Update `docs/INDEX.md`.
- [ ] Commit changes with conventional commit message.
- [ ] Push branch and open Pull Request with comprehensive verification instructions.
