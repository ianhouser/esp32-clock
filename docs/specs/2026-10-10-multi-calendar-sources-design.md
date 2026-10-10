# Specification: Multi-Source Calendar Aggregation with Colored Category Indicators

**Date:** 2026-10-10  
**Status:** Proposed  
**Author:** Antigravity  
**Target Milestone:** [v0.2-web-control](https://github.com/ianhouser/esp32-clock/milestone/2)  

---

## 1. Objective

Allow the ESP32 Clock to subscribe to multiple iCal calendar feeds (e.g., Primary / Personal, Family, Work), aggregate their upcoming events chronologically into a single unified agenda timeline, and render a dedicated colored dot indicator (●) next to each event title on the physical LVGL display corresponding to its calendar source color.

---

## 2. Requirements & Constraints

1. **Multi-Source Configuration:**
   - Support up to 4 calendar sources in `CalendarCard` configuration.
   - Each source defines a `name` (e.g. "Personal", "Family"), a private iCal `url`, and a badge `color` (hex string, e.g. `#3B82F6`, `#10B981`).
   - Maintain 100% backward compatibility: existing single `calendar_url` configurations automatically become Source 1 ("Primary", blue).

2. **Sequential Streaming & Memory Safety:**
   - ESP32-C3 has single-core CPU and limited free SRAM (~120–150 KB free heap).
   - Feeds MUST be streamed sequentially (never concurrently) using `NetworkTaskCoordinator`.
   - Each stream yields to `NetworkTaskCoordinator::yieldUI()` every 40 ms so the clock seconds continue ticking and LVGL never drops frames.
   - Bounded memory footprint: events are filtered line-by-line (`dtstart >= currentDateYmd`) so only upcoming events are retained in heap.

3. **Chronological Sorting & Event Selection:**
   - Events across all sources are tagged with `sourceColor` and `sourceName`.
   - All collected upcoming events are merged and sorted ascending by `rawStart` (`YYYYMMDD` or `YYYYMMDDTHHMMSS`).
   - Top events are passed to `UIManager::updateCalendar`.

4. **Display Design (User Selected):**
   - A dedicated 6×6 circular dot indicator (`_dotEvent1`, `_dotEvent2`) positioned immediately to the left of the event title.
   - Styled using `lv_obj_set_style_radius(..., LV_RADIUS_CIRCLE, 0)` and dynamic `lv_obj_set_style_bg_color(..., lv_color_hex(color), 0)`.
   - Dots are hidden when no events are scheduled or when showing "No Upcoming Events".

5. **Web Control Panel Integration:**
   - Web UI (`data/index.html`, `data/app.js`) provides an interactive calendar source manager:
     - Add, edit, and delete calendar sources.
     - Color picker / preset palette for each source.
     - Saves cleanly into `/api/config`.

---

## 3. Data Architecture

### 3.1 C++ Data Structures

```cpp
// include/CalendarManager.h
struct CalendarSourceConfig {
    String name;
    String url;
    uint32_t colorHex; // 0x3B82F6
};

struct CalendarEvent {
    String summary;
    String timeStr;
    String rawStart;
    uint32_t colorHex;
    String sourceName;
};
```

### 3.2 JSON Configuration Schema (`config.json`)

```json
{
  "id": "calendar",
  "enabled": true,
  "config": {
    "max_events": 3,
    "show_countdown": true,
    "calendar_url": "https://calendar.google.com/...",
    "sources": [
      {
        "name": "Personal",
        "url": "https://calendar.google.com/calendar/ical/.../basic.ics",
        "color": "#3B82F6"
      },
      {
        "name": "Family",
        "url": "https://calendar.google.com/calendar/ical/.../basic.ics",
        "color": "#10B981"
      }
    ]
  }
}
```

---

## 4. UI Layout & Visual Specifications

```
+-------------------------------------------------------+
|  Upcoming Agenda                                      |
|                                                       |
|  ●  Soccer Practice                   (Family - Green)|
|     Today at 4:30 PM                                  |
|                                                       |
|  ●  Dentist Appointment               (Personal - Blue)|
|     Tomorrow at 9:00 AM                               |
+-------------------------------------------------------+
```

- **Dot Specs:**
  - Size: 6px × 6px
  - Radius: Circle (`LV_RADIUS_CIRCLE`)
  - Alignment: Vertically aligned with event title text baseline (`lv_obj_align_to(_dotEvent1, _lblEvent1Title, LV_ALIGN_OUT_LEFT_MID, -4, 0)` or flex container).
  - Background color: dynamically updated per event.
  - Visibility: hidden when event title is empty or "No Upcoming Events".

---

## 5. Web Control Panel Experience

- Inside the Calendar Card settings modal:
  - Table / Card list of configured calendar feeds.
  - Each entry shows:
    - Name input (e.g. "Primary", "Family")
    - URL input (with masked key for privacy or standard text input)
    - Color dot picker with fast presets:
      - 🔵 Blue (`#3B82F6`)
      - 🟢 Emerald (`#10B981`)
      - 🟣 Purple (`#8B5CF6`)
      - 🟠 Amber (`#F59E0B`)
      - 🔴 Rose (`#F43F5E`)
    - Remove button (trash icon)
  - "+ Add Calendar Feed" button.

---

## 6. Implementation Stages

1. **Stage 1 (Backend Core):**
   - Extend `CalendarCard.h` / `CalendarCard.cpp` with `std::vector<CalendarSourceConfig>`.
   - Update `CalendarManager.h` / `CalendarManager.cpp` to accept multiple sources, sequentially stream each, sort merged events chronologically, and tag with colors.
2. **Stage 2 (LVGL Display):**
   - Add `_dotEvent1` and `_dotEvent2` to `UIManager.cpp`.
   - Update `uiManager.updateCalendar()` to accept event colors and show/hide dots.
3. **Stage 3 (Web Control Panel):**
   - Update `data/index.html` and `data/app.js` with multi-source UI editor.
   - Update `data/style.css` for source row cards and color swatches.
4. **Stage 4 (Hardware Verification):**
   - Flash firmware + LittleFS filesystem to physical ESP32.
   - Add family calendar URL via web UI.
   - Screen capture verification of aggregated events and colored dots.
