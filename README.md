# 🕐 esp32-clock

A WiFi-connected nightstand clock for kids, built on an ESP32-C3 Super Mini with a 2" IPS TFT display. Controllable from any phone or browser on your local network.

## Features (Phase 1 — MVP)

- **Digital clock** — 12h format, auto-synced via NTP, configurable timezone
- **Weather** — current temperature and conditions via Open-Meteo (free, no API key)
- **Web control panel** — configure everything from your phone's browser
- **Night mode** — auto-dims to minimal red display during bedtime hours
- **WiFi setup** — captive portal for first-time configuration (no hardcoded passwords)
- **OTA updates** — update firmware over WiFi without unplugging

## Hardware

| Component | Model |
|-----------|-------|
| Microcontroller | ESP32-C3 Super Mini |
| Display | GMT020-02-7P 2" IPS TFT (ST7789, 240×320) |
| Connection | Female-female Dupont jumpers (dev) → solder (production) |

See [`docs/hardware-guide.md`](docs/hardware-guide.md) for wiring instructions.

## Quick Start

### 1. Install PlatformIO

```bash
brew install platformio   # macOS
# or
pip install platformio    # any OS
```

### 2. Clone & Build

```bash
git clone https://github.com/<owner>/esp32-clock.git
cd esp32-clock
pio run                        # Build
pio run --target upload        # Flash firmware
pio run --target uploadfs      # Upload web UI
pio device monitor --baud 115200  # Watch serial output
```

### 3. First Boot

1. Power on → ESP32 creates `ESP32-Clock-Setup` WiFi network
2. Connect your phone → captive portal opens
3. Enter your home WiFi credentials → ESP32 restarts
4. Open `http://<device-ip>` on your phone → control panel

See [`docs/developer-guide.md`](docs/developer-guide.md) for detailed setup.

## Documentation

| Doc | What's In It |
|-----|-------------|
| [`docs/hardware-guide.md`](docs/hardware-guide.md) | Wiring, components, pin reference |
| [`docs/developer-guide.md`](docs/developer-guide.md) | Build, flash, PlatformIO setup |
| [`docs/architecture.md`](docs/architecture.md) | System design, modules, data flows |
| [`docs/product-brief.md`](docs/product-brief.md) | Vision, scope, success criteria |
| [`CONTRIBUTING.md`](CONTRIBUTING.md) | Git workflow, coding standards |

## Future Plans

- **Phase 2:** ESPHome / Home Assistant integration, air quality sensor
- **Phase 3:** RGBIC LED strip, per-kid themes, 3D-printed enclosures

## License

MIT
