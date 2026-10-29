# Implementation Plan: Modular Web Control Panel & Customization Dashboard

**Issue:** https://github.com/ianhouser/esp32-clock/issues/11  
**Milestone:** `v0.2-web-control`  
**Branch:** `feat/11-web-control-panel`  
**Date:** 2026-10-09

## Description
Implement an asynchronous, browser-based web control panel hosted directly on the ESP32 (accessible at `http://esp32-clock.local` and local IP) enabling users to customize visual themes, arrange display cards, and configure device settings.

### Motivation & Extensibility
The control panel and firmware architecture is designed to be **strictly modular** so new display cards (such as upcoming Calendar Events, Email & Messaging Notifications, and Smart Home telemetry) can register their data schemas, configuration fields, and render routines without rewriting core engine code.

### Architecture & Components
1. **Modular Card Architecture & Registry (`CardRegistry` / `Card` Interface)**:
   - Base `Card` interface with lifecycle hooks (`init`, `applyTheme`, `setVisible`, `serializeConfig`, `deserializeConfig`).
   - Cards registered by unique ID (`clock`, `weather`, `forecast`, `calendar`, `notifications`, `system`).
   - Cards can be toggled on/off, reordered via Up/Down controls, and configure card-specific settings.
2. **Configuration Engine (`ConfigManager` in LittleFS)**:
   - Stores persistent configuration in `/config.json` on LittleFS.
   - Dynamic schema validation, fallback defaults, and thread-safe in-memory caching.
   - Dynamic `ThemeColors` conversion (hex strings to RGB565 16-bit color integers).
3. **Async Web Server & REST API (`WebServerManager`)**:
   - `ESPAsyncWebServer` on port 80 with mDNS hostname discovery (`http://esp32-clock.local`).
   - Static asset streaming from LittleFS with HTTP caching headers.
   - REST Endpoints: `GET /api/config`, `POST /api/config`, `GET /api/status`, `POST /api/restart`.
4. **Single-Page Control Panel Dashboard (`data/`)**:
   - Modern glassmorphic dark UI with responsive mobile/desktop layout (`index.html`, `style.css`, `app.js`).
   - Modular Cards Manager: Enable/disable, reorder cards, and configure card parameters dynamically.
   - Theme Studio: Palette presets (Cyberpunk, Nord, Solar Amber, Emerald Matrix, OLED Minimal, Deep Violet), custom color pickers, and day/night schedules.
   - Live 320x240 Screen Preview: Real-time HTML5 canvas rendering simulating the ST7789 display.
5. **Flash Partitioning (`partitions.csv`)**:
   - Custom 4MB partition table allocating 2MB for app firmware (only ~60% utilized) and ~1.87MB for LittleFS web assets.

## Verification
- Firmware compiles cleanly (`pio run`) with 0 errors.
- LittleFS filesystem builds cleanly (`pio run -t buildfs`) with `index.html`, `style.css`, and `app.js`.
