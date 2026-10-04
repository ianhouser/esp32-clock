# KidClock — Product Brief

## Vision

A nightstand clock for kids, built on an ESP32-C3 Super Mini with a 2" IPS TFT display. Controllable via any phone/browser on the local network. Designed to be simple enough for a teenager to build from a guide, cute enough to personalize per kid, and extensible enough to grow into a smart home device.

## Users

| User | Needs |
|------|-------|
| **Kids (ages ~5–12)** | Glanceable time, gentle night mode, fun personalized display |
| **Parents** | Remote control from phone, set night mode hours, configure weather location, OTA updates |
| **Builder (teenager / hobbyist)** | Clear wiring guide, step-by-step build instructions, explainable code |

## Success Criteria — Phase 1 (MVP)

1. **Clock works reliably** — accurate time via NTP, survives WiFi drops, auto-recovers.
2. **Weather displays** — current temp + condition from Open-Meteo, refreshed every 15 min.
3. **Web control panel** — any device on WiFi can access settings (timezone, weather location, brightness, night mode hours).
4. **Night mode** — auto-activates during configured hours, shows dim red numerals on black.
5. **First-time setup** — captive portal WiFi configuration (no hardcoded credentials).
6. **OTA updates** — firmware can be updated over WiFi without USB.
7. **Documentation** — a nephew with basic soldering skills can build it from the repo alone.

## Phase 2 — Smart Home Integration

- ESPHome / Home Assistant via MQTT
- Air quality sensor (BME680 or similar) on I²C
- AQI display screen

## Phase 3 — Personalization & Fun

- RGBIC LED strip (WS2812B) with web-controlled effects
- Per-kid themes (name, avatar, color scheme)
- Alarms / timers with optional buzzer
- 3D-printed personalized enclosure

## Hardware

| Component | Model |
|-----------|-------|
| MCU | ESP32-C3 Super Mini |
| Display | GMT020-02-7P 2" IPS TFT (ST7789, 240×320, 4-wire SPI) |
| Connection (dev) | Female-female Dupont jumpers |
| Enclosure (future) | 3D-printed, personalized per kid |

## Non-Goals (Phase 1)

- No BLE control
- No native mobile app
- No alarm/timer functionality
- No physical buttons or touch input
- No air quality or LED features
- No multi-device sync
