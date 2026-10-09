# UI Design Concepts & Architecture

This document tracks the UI design directions, wireframes, and architectural options for the **esp32-clock** display interface.

---

## 1. Design Direction: Concept A (Active Implementation)

### "Sleek Modern Digital Dashboard" (Card & Glassmorphism)

Concept A organizes the 320×240 IPS landscape display into a balanced, glanceable smart dashboard:

```
┌────────────────────────────────────────────────────────────────┐
│  esp32-clock         [ ☀️ DAY ]              WiFi -45dBm  [●]  │ (Header: H=28)
├────────────────────────────────────────────────────────────────┤
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                                                          │  │
│  │                       11 : 42 : 30                       │  │ (Hero Clock Card)
│  │                 Friday, October 09, 2026                 │  │
│  │                                                          │  │
│  └──────────────────────────────────────────────────────────┘  │
├────────────────────────────────────────────────────────────────┤
│  ┌─────────────────────────────┐ ┌───────────────────────────┐  │
│  │ WEATHER                     │ │ SYSTEM & NET              │  │
│  │ 86°F  Clear Sky             │ │ WiFi: Home-Network        │  │ (Split Detail Cards)
│  │ Feels: 86°F                 │ │ IP: 192.168.1.145         │  │
│  │ Humidity: 29%               │ │ NTP: Locked               │  │
│  └─────────────────────────────┘ └───────────────────────────┘  │
└────────────────────────────────────────────────────────────────┘
```

### Key Elements & Characteristics
- **Header**: Compact status bar with device identity, Day/Night pill badge, and live WiFi dBm signal indicator.
- **Main Hero Card**: Large anti-aliased digital time with smooth second ticks, accompanied by full weekday and date formatting.
- **Split Detail Cards**:
  - **Left (Weather)**: Real-time outdoor conditions from Open-Meteo REST API (temperature, feels-like, humidity, condition string).
  - **Right (System & Net)**: Essential connection telemetry (SSID, IP address, NTP lock status).
- **Day / Night Themes**:
  - *Day Mode*: Deep slate navy canvas (`#081020`), slate cards (`#102030`), crisp neon cyan accents (`#00F0FF`).
  - *Night Mode*: True pitch black canvas (`#000000`) for minimal IPS light bleed, deep warm amber/crimson typography (`#FF9000`) to protect night-adapted vision.

---

## 2. Design Direction: Concept B (Tracked for Future Evolution)

### "Multi-Screen Carousel" (Timed Auto-Cycle & Interactive Navigation)

Concept B expands the interface into multiple dedicated full-screen views that cycle automatically (e.g. every 10–15 seconds) or navigate via physical button/touch input:

### Screen 1: Minimalist Ambient Clock Face
- Large 64pt Montserrat/Inter bold digital or modern minimal analog face.
- Subtle current weather icon and temperature pill badge in the corner.
- Zero clutter, optimal for bedroom bedside glance.

### Screen 2: Extended Weather & Hourly Forecast
- 12-hour hourly forecast temperature strip or bar graph using `lv_chart`.
- Daily High / Low temperature arc gauge (`lv_arc`).
- Precipitation chance, UV index, wind speed, and humidity.

### Screen 3: Environment & Sensor Telemetry (When BME680 arrives)
- Indoor temperature vs outdoor temperature comparison.
- Air Quality Index (AQI) / TVOC gauge with good/moderate/poor color indicators.
- Atmospheric pressure trend (barometer).

### Screen 4: Dedicated Night Mode (Sleep View)
- Ultra-dim minimalist bedside clock with subtle amber glow.
- Shuts off secondary telemetry to avoid light emission.
- Optional sleep timer / chime status.

### Transition Mechanism
- Powered by LVGL screen manager: `lv_scr_load_anim(next_scr, LV_SCR_LOAD_ANIM_MOVE_LEFT, 400, 0, false)`.
- Smooth slide or fade animations between views.

---

## 3. Technology Stack: LVGL on ESP32-C3

- **Driver**: `TFT_eSPI` remains the low-level hardware SPI driver (using pre-validated GPIO 6, 7, 10, 2, 3 pinouts).
- **Buffer Strategy**: Partial display buffer in internal SRAM (e.g. two 8 KB line buffers, ~16 KB total) to preserve over 120 KB heap on the ESP32-C3 Super Mini.
- **Engine**: LVGL v8.3.11 with custom styles, animations, and flexbox containers.
