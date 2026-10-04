# KidClock — Hardware Guide

> **Audience:** A teenager with basic electronics experience. If you can plug in a USB cable, you can build this.

## What You Need

| Item | Qty | Notes |
|------|-----|-------|
| ESP32-C3 Super Mini | 1 | The tiny board with WiFi and USB-C |
| GMT020-02-7P 2" TFT Display | 1 | Ver 1.3, 240×320, IPS, ST7789 driver |
| Female-female Dupont jumper wires | 7 | For development wiring (solder later) |
| USB-C cable | 1 | For power and programming |
| Computer with USB port | 1 | Mac, Windows, or Linux |

## Know Your Parts

### ESP32-C3 Super Mini

A tiny microcontroller board (about the size of a postage stamp) with:
- **WiFi** built in — connects to your home network
- **USB-C** — for programming and power
- **GPIO pins** — the numbered pins on the edges that connect to other hardware
- **3.3V logic** — important: everything is 3.3V, not 5V

### GMT020-02-7P Display

A 2-inch color screen (like a tiny phone screen) with 7 pins on one edge:

| Pin # | Label on Board | What It Does |
|-------|---------------|--------------|
| 1 | VCC | Power (3.3V) |
| 2 | GND | Ground |
| 3 | CS | Chip Select — tells the screen "I'm talking to you" |
| 4 | RST | Reset — restarts the screen |
| 5 | DC | Data/Command — tells the screen if you're sending a command or pixel data |
| 6 | SDA | Data line — the actual pixel data goes here (this is SPI MOSI) |
| 7 | SCL | Clock line — keeps data in sync (this is SPI SCK) |

> **Don't be confused!** The labels say "SDA" and "SCL" which usually mean I²C, but this display uses **SPI**. The labels are just misleading. SDA = MOSI, SCL = SCK.

## Wiring Diagram

Connect the display to the ESP32-C3 using Dupont jumper wires:

```
Display Pin          ESP32-C3 Pin
───────────          ────────────
VCC  ──────────────► 3V3
GND  ──────────────► GND
CS   ──────────────► GPIO 10
RST  ──────────────► GPIO 3
DC   ──────────────► GPIO 2
SDA  ──────────────► GPIO 7
SCL  ──────────────► GPIO 6
```

### Step-by-Step

1. **Lay out both boards** on a flat surface, pins facing up.
2. Pick up a jumper wire. Plug one end into the display's **VCC** pin.
3. Plug the other end into the ESP32-C3's **3V3** pin.
4. Repeat for each row in the table above.
5. **Double-check every wire** before plugging in USB. Wrong wiring won't break anything, but the screen won't work.

### Pin Finding Tips

- On the ESP32-C3 Super Mini, pins are labeled on the **bottom** of the board (flip it over).
- GPIO numbers are printed as small text next to each pin.
- **3V3** is the 3.3V power output pin.
- **GND** — there are multiple GND pins; any one works.

## ⚠️ Important Safety Notes

1. **Always unplug USB before changing wires.** It's safe, but good practice.
2. **Never connect VCC to 5V.** The display is 3.3V only. The ESP32-C3's 5V pin (if it has one) could damage the display.
3. **Check orientation.** Make sure female connectors are fully seated on the pins.

## Pin Map Reference

This is the complete GPIO allocation for the project (current + future):

| GPIO | Used For | Status |
|------|----------|--------|
| 0 | Future: RGBIC LED data line | 🔲 Reserved |
| 1 | Future: Ambient light sensor (ADC input) | 🔲 Reserved |
| 2 | Display DC (Data/Command) | ✅ In use |
| 3 | Display RST (Reset) | ✅ In use |
| 4 | Future: I²C SDA (air quality sensor) | 🔲 Reserved |
| 5 | Future: I²C SCL (air quality sensor) | 🔲 Reserved |
| 6 | Display SCK (SPI Clock) | ✅ In use |
| 7 | Display MOSI (SPI Data) | ✅ In use |
| 8 | Onboard LED (built into the board) | ⚡ System |
| 9 | BOOT button (built into the board) | ⚡ System |
| 10 | Display CS (Chip Select) | ✅ In use |

## Troubleshooting

| Symptom | Check |
|---------|-------|
| Screen stays black | Is VCC connected to 3V3 (not 5V)? Is USB plugged in? |
| Screen has backlight but no image | Check DC and RST wires. Try swapping them if reversed. |
| Garbled/scrambled image | Check SCK (GPIO 6) and SDA/MOSI (GPIO 7) aren't swapped. |
| Colors look wrong | Normal for first boot — the display init code sets color mode. |
| ESP32 won't boot / stuck in boot loop | Check that GPIO 8 and GPIO 9 aren't connected to anything. |
