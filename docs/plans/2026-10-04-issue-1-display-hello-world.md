# Implementation Plan: Issue #1 — Project Scaffolding & Display Hello-World

**Issue:** [#1 (feat: project scaffolding & display hello-world)](https://github.com/ianhouser/esp32-clock/issues/1)  
**Branch:** `feat/#1-display-hello-world`

## Objective
Establish the PlatformIO build environment for the ESP32-C3 Super Mini and GMT020-02-7P (ST7789) TFT display, and verify display functionality with a test pattern/message rendered in 320x240 landscape orientation.

## Task Breakdown

### Task 1: Branch Setup
- [ ] Ensure local `main` is clean.
- [ ] Create and check out feature branch: `feat/#1-display-hello-world`.

### Task 2: PlatformIO Configuration (`platformio.ini`)
- **File:** `platformio.ini`
- [ ] Configure environment `[env:esp32-c3-devkitm-1]`.
- [ ] Set `platform = espressif32`.
- [ ] Set `board = esp32-c3-devkitm-1`.
- [ ] Set `framework = arduino`.
- [ ] Set monitor baud to `115200`.
- [ ] Add library dependency: `bodmer/TFT_eSPI@^2.5.43`.
- [ ] Add build flags directing `TFT_eSPI` to use our custom setup: `-D USER_SETUP_LOADED=1` and `-I include`.

### Task 3: Pin Definitions & Display Driver Setup
- **Files:**
  - Create: `include/config.h`
  - Create: `include/User_Setup.h`
- [ ] Define hardware SPI pins in `include/config.h`:
  - `TFT_CS = 10`
  - `TFT_RST = 3`
  - `TFT_DC = 2`
  - `TFT_MOSI = 7` (SDA)
  - `TFT_SCLK = 6` (SCL)
- [ ] Configure `include/User_Setup.h` for `TFT_eSPI`:
  - ST7789 driver definition (`#define ST7789_DRIVER`)
  - Resolution 240x320 (`#define TFT_WIDTH 240`, `#define TFT_HEIGHT 320`)
  - Map pins to `TFT_eSPI` macros (`TFT_CS`, `TFT_DC`, `TFT_RST`, `TFT_MOSI`, `TFT_SCLK`)
  - SPI frequency set to 40MHz or 27MHz for reliable signal integrity over Dupont jumpers.

### Task 4: Hello-World Firmware Implementation
- **File:** `src/main.cpp`
- [ ] Initialize `Serial` at 115200 baud.
- [ ] Initialize `TFT_eSPI`:
  - Set rotation to landscape (`tft.setRotation(1)` or `3`).
  - Fill screen with dark slate background.
  - Draw colorful test banner, borders, and hello-world text ("esp32-clock", "Display OK").
  - Print system status (free heap, chip revision) to Serial and screen.

### Task 5: Compilation & Verification
- [ ] Run `pio run` to verify clean compilation with 0 errors/warnings.
- [ ] Document exact flash and serial monitor commands for physical testing.

### Task 6: Documentation & Handoff
- [ ] Update `docs/agent-handoff.md`.
- [ ] Re-index docs via `docs/INDEX.md`.
