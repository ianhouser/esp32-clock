---
name: screen-capture
description: >-
  Capture, inspect, and visually verify the physical ESP32 display framebuffer directly
  over USB CDC serial. Use whenever the user or agent needs to view the current display screen,
  verify UI changes, take a screenshot, or inspect colors and layout.
---

# Screen Capture

Capture the live ST7789 display framebuffer from the connected ESP32-C3 device over USB CDC serial into an image file, and inspect it directly.

## When to use

- When the user asks to see what's on the screen, take a screenshot, or check the display.
- After modifying UI layouts, widgets, fonts, or theme palettes to visually verify the rendering.
- To compare Day Mode vs Night Mode appearance.

## When not to use

- When the ESP32 is not plugged into USB serial.
- During firmware upload / flashing (port will be busy).

## Instructions

1. **Execute capture command:**
   Run the capture script using npm or python:
   ```bash
   npm run capture
   ```
   Or capture a specific theme:
   ```bash
   npm run capture:day    # force Day mode and capture
   npm run capture:night  # force Night mode and capture
   ```

2. **Inspect the captured frame:**
   Call `view_file` on `artifacts/screen_capture.png` (or `artifacts/screen_capture_night.png`).
   Evaluate:
   - Layout geometry, card borders, margins, and alignments.
   - Color accuracy (vibrant neon cyan in Day mode, warm amber in Night mode).
   - Text rendering clarity and correct timestamp / sensor telemetry.

3. **Restore auto schedule if mode was overridden:**
   If a theme was temporarily forced, restore automatic day/night tracking:
   ```bash
   python3 scripts/capture_screen.py --mode auto
   ```
