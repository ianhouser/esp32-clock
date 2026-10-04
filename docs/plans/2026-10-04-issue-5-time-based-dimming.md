# Implementation Plan: Time-Based Display Dimming & Night Mode

- **Issue:** [#5 — feat: time-based display night mode and dimming schedule](https://github.com/ianhouser/esp32-clock/issues/5)
- **Branch:** `feat/#5-time-based-dimming`
- **Milestone:** `v0.1-mvp`
- **Status:** In Progress
- **Created:** 2026-10-04

---

## 1. Overview & Motivation

The GMT020-02-7P 2.0" IPS TFT display operates with high clarity and contrast, but at nighttime in a bedroom or dark room, high-luminance blue/cyan light causes glare, sleep disruption, and light bleed.

Since ambient light sensor hardware (ADC on GPIO 1) is deferred until physical components arrive, this feature implements **scheduled time-based dimming and night mode**. The clock leverages the accurate local time synchronized by `TimeManager` (SNTP) to automatically transition between vibrant **Day Mode** and dark, eye-friendly **Night Mode**.

Furthermore, it implements a dual architecture:
1. **Software Palette Dimming**: Immediately functional on the standard 7-pin GMT020-02-7P module (which powers backlight LEDs directly from VCC without an exposed BL pin), dynamically shifting the UI to a minimal-emission deep amber/crimson and true black palette.
2. **Hardware LEDC PWM Ready**: Configurable backlight pin (`PIN_TFT_BL`) support using ESP32-C3 LEDC PWM timer channels for hardware dimming if a backlight control pin or transistor modification is enabled.

---

## 2. Technical Design

### 2.1 Theme Data Structure

A unified `ThemeColors` struct in `include/DisplayTheme.h`:

```cpp
enum class DisplayMode {
    DAY,
    NIGHT
};

struct ThemeColors {
    uint16_t bg;           // Main background color (16-bit 565)
    uint16_t headerBg;     // Header banner background
    uint16_t cardBg;       // Info card background
    uint16_t cardBorder;   // Card border / separators
    uint16_t timeColor;    // Digital clock digits (e.g. Cyan for day, Dim Amber for night)
    uint16_t dateColor;    // Date string color
    uint16_t labelColor;   // Section headers & titles
    uint16_t mutedText;    // Body / secondary info text
    const char* modeLabel; // "DAY" / "NIGHT"
};
```

### 2.2 Color Palettes

- **Day Mode**:
  - `bg`: `0x0842` (Deep slate navy)
  - `headerBg`: `0x0926` (Deep dark cyan-navy)
  - `cardBg`: `0x10A4` (Dark slate card)
  - `cardBorder`: `0x2969` (Subtle cyan-slate border)
  - `timeColor`: `0x07FD` (Vibrant cyan)
  - `dateColor`: `0xFFFF` (Pure white)
  - `labelColor`: `0x07FD` (Cyan accent)
  - `mutedText`: `0x9CD3` (Light slate)

- **Night Mode**:
  - `bg`: `0x0000` (Pitch black — shuts off IPS light transmission as much as possible)
  - `headerBg`: `0x0000` (Pitch black)
  - `cardBg`: `0x0800` (Ultra-dim dark maroon/black)
  - `cardBorder`: `0x2000` (Faint dim red/brown border)
  - `timeColor`: `0xA200` (Muted warm amber/crimson — preserves melatonin / night vision)
  - `dateColor`: `0x6180` (Muted warm dusk gray)
  - `labelColor`: `0x8180` (Dim warm amber)
  - `mutedText`: `0x5140` (Faint night text)

### 2.3 Schedule Calculation

In `DisplayManager.h` / `DisplayManager.cpp`:
- Configurable schedule in `config.h`:
  - `NIGHT_MODE_START_HOUR` (default `22` = 10:00 PM)
  - `NIGHT_MODE_START_MIN` (default `0`)
  - `NIGHT_MODE_END_HOUR` (default `7` = 7:00 AM)
  - `NIGHT_MODE_END_MIN` (default `0`)
- Overnight schedule logic:
  - If `startMinutes > endMinutes` (standard overnight schedule):
    `isNight = (currentMinutes >= startMinutes || currentMinutes < endMinutes)`
  - If `startMinutes < endMinutes` (same-day schedule):
    `isNight = (currentMinutes >= startMinutes && currentMinutes < endMinutes)`

### 2.4 Hardware LEDC PWM Backlight Control

- If `PIN_TFT_BL >= 0` is defined in `config.h`:
  - Configures LEDC timer (5 kHz, 8-bit resolution, channel 0).
  - Smoothly updates duty cycle:
    - Day duty: `PWM_BL_DAY_DUTY` (e.g. 255 / 100%)
    - Night duty: `PWM_BL_NIGHT_DUTY` (e.g. 40 / ~15%)
- If `PIN_TFT_BL < 0`:
  - LEDC initialization is bypassed cleanly; software palette dimming operates autonomously.

---

## 3. Implementation Steps

1. **Step 1: Configuration Definitions (`include/config.h`)**
   - Add schedule hours/minutes for Day/Night mode.
   - Add backlight pin definition (`PIN_TFT_BL -1`) and PWM parameters.

2. **Step 2: Display Manager & Theme Module (`include/DisplayTheme.h`, `include/DisplayManager.h`, `src/DisplayManager.cpp`)**
   - Encapsulate `DisplayMode`, `ThemeColors`, Day & Night theme presets.
   - Implement `DisplayManager` class with:
     - `begin(int blPin = -1)`
     - `update(int hour, int minute)`: returns `true` if mode transitioned.
     - `getMode()`: returns current `DisplayMode`.
     - `getTheme()`: returns const reference to active `ThemeColors`.
     - `setMode(DisplayMode mode)`: manual override support.

3. **Step 3: Integrate with `src/main.cpp`**
   - Instantiate `DisplayManager displayManager`.
   - Update `drawStaticLayout()` and `updateDisplay()` to consume dynamic `displayManager.getTheme()` colors instead of hardcoded macros.
   - Detect mode change in `loop()` / `updateDisplay()` and trigger full redraw (`drawStaticLayout()` + `updateDisplay(true)`).
   - Display a mode badge in the header: `☀️ DAY` or `🌙 NIGHT`.

4. **Step 4: Compilation & Verification**
   - Run `pio run` to verify zero compiler warnings or errors on `esp32-c3-devkitm-1`.
   - Verify RAM and flash overhead are negligible.

5. **Step 5: Documentation & Git Commit**
   - Update `docs/agent-handoff.md` and `docs/INDEX.md`.
   - Commit and push to `feat/#5-time-based-dimming`.
   - Create PR linking Issue #5.
