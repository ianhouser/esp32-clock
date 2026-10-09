# Implementation Plan: LVGL Modern Digital Dashboard (Concept A)

- **Issue:** [#9 — feat: integrate LVGL graphics library and build modern digital dashboard UI](https://github.com/ianhouser/esp32-clock/issues/9)
- **Branch:** `feat/#9-lvgl-modern-dashboard`
- **Milestone:** `v0.1-mvp`
- **Status:** In Progress
- **Created:** 2026-10-09

---

## 1. Overview & Objectives

This plan transitions the **esp32-clock** user interface from direct `TFT_eSPI` rendering to **[LVGL](https://lvgl.io)** (Light and Versatile Graphics Library) v8.3.11.

By layering LVGL on top of our existing validated `TFT_eSPI` ST7789 SPI driver:
1. We gain anti-aliased proportional typography, sub-pixel rendering, rounded cards, flexbox layouts, and built-in theme support.
2. We implement **Concept A ("Sleek Modern Digital Dashboard")**, featuring a top status bar, hero clock card, and side-by-side weather and network cards.
3. We preserve Concept B ("Multi-Screen Carousel") in documentation for future expansion.

---

## 2. Technical Architecture & Memory Constraints

### 2.1 Memory Footprint on ESP32-C3 Super Mini
- **Total SRAM**: 320 KB (~160 KB usable heap at runtime, no external PSRAM).
- **LVGL Memory Pool**: Allocate 32 KB internal heap (`LV_MEM_SIZE (32 * 1024U)`).
- **Partial Display Buffers**:
  - Buffer size: 320 × 20 pixels = 6,400 pixels = 12.8 KB (RGB565).
  - Using two 12.8 KB partial line buffers for asynchronous rendering and SPI DMA transfers: ~25.6 KB RAM.
  - Leaves **over 90–100 KB free heap** for WiFi, SNTP, HTTPClient, and ArduinoJson.

### 2.2 Display Driver Hook (`disp_flush`)
`TFT_eSPI` remains the low-level SPI driver:

```cpp
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t*)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}
```

### 2.3 Tick and Event Loop Integration
- In `loop()`:
  - Call `lv_timer_handler()` regularly (every 5–10ms).
  - Update clock, weather, and system labels only when underlying data changes.

---

## 3. UI Component Architecture (Concept A)

### 3.1 Layout Tree
```
lv_scr_act()
├── header_bar (H=28, Flex row, space-between)
│   ├── lbl_title ("esp32-clock")
│   ├── badge_mode ("☀️ DAY" / "🌙 NIGHT")
│   └── lbl_wifi_status ("WiFi -45dBm")
├── main_clock_card (Y=34, H=104, W=304, Rounded=8)
│   ├── lbl_time ("11:42:30", Montserrat 32/36 bold)
│   └── lbl_date ("Friday, October 09, 2026", Montserrat 16)
└── bottom_container (Y=144, H=88, Flex row, gap=6)
    ├── weather_card (W=149, Rounded=8)
    │   ├── lbl_weather_title ("WEATHER")
    │   ├── lbl_weather_temp ("86°F  Clear Sky")
    │   ├── lbl_weather_feels ("Feels: 86°F")
    │   └── lbl_weather_humidity ("Humidity: 29%")
    └── system_card (W=149, Rounded=8)
        ├── lbl_sys_title ("SYSTEM & NET")
        ├── lbl_sys_ssid ("WiFi: MySSID")
        ├── lbl_sys_ip ("IP: 192.168.1.145")
        └── lbl_sys_ntp ("NTP: Locked")
```

### 3.2 Dynamic Day & Night Styles
- **Day Style**:
  - Screen BG: `lv_color_make(8, 16, 32)`
  - Card BG: `lv_color_make(16, 32, 48)`
  - Accents: `lv_color_make(0, 240, 255)` (Cyan)
- **Night Style**:
  - Screen BG: `lv_color_black()`
  - Card BG: `lv_color_make(12, 6, 6)`
  - Accents / Digits: `lv_color_make(255, 140, 0)` (Warm Amber)

---

## 4. Implementation Steps

1. **Step 1: Dependency & Configuration (`platformio.ini`, `include/lv_conf.h`)**
   - Add `lvgl/lvgl@^8.3.11` to `platformio.ini`.
   - Create `include/lv_conf.h` with optimized settings for ESP32-C3.
2. **Step 2: LVGL Subsystem Wrapper (`include/UIManager.h`, `src/UIManager.cpp`)**
   - Encapsulate LVGL initialization, partial buffer allocation, and UI widget creation.
   - Provide clean update methods: `setTime(...)`, `setWeather(...)`, `setNetwork(...)`, `setTheme(...)`.
3. **Step 3: Integration with `src/main.cpp`**
   - Initialize `uiManager.begin()`.
   - Replace manual drawing loops with reactive `uiManager` updates.
   - Call `lv_timer_handler()` in `loop()`.
4. **Step 4: Verification & Intellisense**
   - Verify build with `pio run`.
   - Update `compile_commands.json` and documentation (`docs/INDEX.md`, `docs/agent-handoff.md`).
5. **Step 5: PR Creation & Hand Off to Human User**
   - Push branch and open PR linking Issue #9.
   - **Per AGENTS.md: Stop and let the human user review and merge the PR.**
