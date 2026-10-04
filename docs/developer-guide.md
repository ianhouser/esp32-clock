# esp32-clock — Developer Guide

> **Audience:** Someone setting up the project for the first time. Covers everything from installing tools to flashing the ESP32-C3.

## Prerequisites

| Tool | Version | Install |
|------|---------|---------|
| **PlatformIO Core (CLI)** | Latest | `brew install platformio` (macOS) or `pip install platformio` |
| **PlatformIO IDE** (optional) | Latest | VS Code extension: "PlatformIO IDE" |
| **Git** | Any | `brew install git` or [git-scm.com](https://git-scm.com) |
| **USB-C cable** | — | Data-capable (not charge-only) |
| **Python 3** | 3.8+ | Required by PlatformIO (usually pre-installed) |

### Installing PlatformIO

**Option A — VS Code (Recommended for beginners):**
1. Install [VS Code](https://code.visualstudio.com/)
2. Open VS Code → Extensions → Search "PlatformIO IDE" → Install
3. Restart VS Code. PlatformIO icon appears in the sidebar.

**Option B — CLI only:**
```bash
# macOS
brew install platformio

# Or via pip
pip install platformio
```

## Clone & Build

```bash
# Clone the repo
git clone https://github.com/<owner>/esp32-clock.git
cd esp32-clock

# Build firmware
pio run

# Flash firmware to ESP32-C3 (connect via USB first)
pio run --target upload

# Upload web UI files to LittleFS
pio run --target uploadfs

# Monitor serial output (115200 baud)
pio device monitor --baud 115200
```

## Project Structure

```
esp32-clock/
├── platformio.ini              # Build config: board, libs, partitions
├── include/
│   ├── config.h                # Pin definitions, compile-time constants
│   └── User_Setup.h            # TFT_eSPI display configuration
├── src/
│   ├── main.cpp                # Setup + loop, screen manager
│   ├── display.cpp/h           # TFT init, sprite helpers, brightness
│   ├── screen_manager.cpp/h    # Active screen, transitions, night mode
│   ├── wifi_manager.cpp/h      # WiFi STA + AP captive portal
│   ├── time_manager.cpp/h      # NTP sync, timezone, formatting
│   ├── weather.cpp/h           # Open-Meteo client, JSON parse, cache
│   ├── settings.cpp/h          # LittleFS JSON persistence
│   ├── web_server.cpp/h        # REST API + static file server
│   └── screens/
│       ├── screen.h            # Base screen interface
│       ├── clock_screen.cpp/h  # Main clock face
│       └── weather_screen.cpp/h
├── data/                       # LittleFS filesystem root
│   ├── index.html              # Web control panel
│   ├── style.css
│   ├── app.js
│   └── config.json             # Default settings (overwritten on-device)
├── docs/                       # Project documentation
├── AGENTS.md                   # Agent hub (binding decisions)
├── CONTRIBUTING.md             # Git workflow rules
└── README.md
```

## Common Tasks

### Build Only (No Flash)
```bash
pio run
```

### Flash Firmware
```bash
pio run --target upload
```

### Upload Web UI to LittleFS
```bash
pio run --target uploadfs
```
> **Note:** This uploads everything in the `data/` directory to the ESP32's flash filesystem.

### Serial Monitor
```bash
pio device monitor --baud 115200
```
Press `Ctrl+C` to exit.

### OTA Update (once WiFi is configured)
```bash
pio run --target upload --upload-port <device-ip>
```

### Clean Build
```bash
pio run --target clean
```

## First Boot Flow

1. **Flash firmware** + **upload LittleFS** via USB.
2. ESP32 boots → no WiFi saved → starts **AP mode**.
3. AP name: `ESP32-Clock-Setup` (open network).
4. Connect your phone to `ESP32-Clock-Setup`.
5. Captive portal opens → enter your WiFi SSID + password.
6. ESP32 restarts → connects to your WiFi → shows clock.
7. Find the device IP in your router or serial monitor.
8. Open `http://<device-ip>` on your phone → web control panel.

## Troubleshooting Build Issues

| Problem | Solution |
|---------|----------|
| `pio: command not found` | Add PlatformIO to PATH or use full path: `~/.platformio/penv/bin/pio` |
| USB device not detected | Try a different USB-C cable (must be data-capable). Try `ls /dev/cu.*` to see ports. |
| Upload fails with "no serial port" | Hold BOOT button on ESP32 → press RST → release BOOT. Then retry upload. |
| LittleFS upload fails | Ensure `data/` directory exists with at least one file. |
| Wrong board selected | Confirm `platformio.ini` has `board = esp32-c3-devkitm-1` |
