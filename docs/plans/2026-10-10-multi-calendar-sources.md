# Plan: Multi-Calendar Sources with Colored Category Indicators

**Date:** 2026-10-10  
**Spec:** `docs/specs/2026-10-10-multi-calendar-sources-design.md`  
**Milestone:** [v0.2-web-control](https://github.com/ianhouser/esp32-clock/milestone/2)  

---

### Task 1: Data Model & Configuration Serialization
**Files:**
- Modify: `include/cards/CalendarCard.h`
- Modify: `src/cards/CalendarCard.cpp`

- [ ] Define `CalendarSource` struct with `name`, `url`, `color`.
- [ ] Add `std::vector<CalendarSource> _sources` to `CalendarCard`.
- [ ] Update `serializeConfig()` to write `"sources": [ ... ]` array.
- [ ] Update `deserializeConfig()` to read `"sources"` array with fallback to legacy `"calendar_url"`.
- [ ] Add getter `const std::vector<CalendarSource>& getSources() const`.

---

### Task 2: Aggregated Sequential Streaming & Chronological Sorting
**Files:**
- Modify: `include/CalendarManager.h`
- Modify: `src/CalendarManager.cpp`

- [ ] Add `uint32_t colorHex` and `String sourceName` to `CalendarEvent`.
- [ ] Update `CalendarManager::begin()` to take `std::vector<CalendarSource> sources`.
- [ ] In `CalendarManager::fetchCalendar()`:
  - Loop sequentially over each source with non-empty URL.
  - Yield to `NetworkTaskCoordinator::yieldUI()` between streams and every 40 ms within streams.
  - Parse events, tagging each with source color and name.
  - Filter `dtstart >= currentDateYmd`.
- [ ] Merge events across sources and sort ascending by `rawStart`.
- [ ] Store top events in `_events`.

---

### Task 3: LVGL UI Display Colored Dot Indicators
**Files:**
- Modify: `include/UIManager.h`
- Modify: `src/UIManager.cpp`

- [ ] Add `lv_obj_t* _dotEvent1` and `lv_obj_t* _dotEvent2` members to `UIManager`.
- [ ] In `initCalendarCard()`:
  - Create 6×6 circular dot objects with `LV_RADIUS_CIRCLE`.
  - Align dots immediately to the left of `_lblEvent1Title` and `_lblEvent2Title`.
  - Default flags to hidden.
- [ ] In `updateCalendar()`:
  - Accept `uint32_t ev1Color` and `uint32_t ev2Color`.
  - When event is valid: apply `lv_obj_set_style_bg_color(..., lv_color_hex(color), 0)` and clear `LV_OBJ_FLAG_HIDDEN`.
  - When event is empty or "No Upcoming Events": add `LV_OBJ_FLAG_HIDDEN`.

---

### Task 4: Main Application Coordination
**Files:**
- Modify: `src/main.cpp`

- [ ] In `applyServicesConfig()`:
  - Pass `cc->getSources()` to `calendarManager.begin()`.
- [ ] In `NetworkTaskCoordinator` calendar job:
  - Pass `events[0].colorHex` and `events[1].colorHex` to `uiManager.updateCalendar()`.

---

### Task 5: Web Control Panel Multi-Source Editor
**Files:**
- Modify: `data/index.html`
- Modify: `data/app.js`
- Modify: `data/style.css`

- [ ] In `data/index.html`:
  - Add calendar feeds manager container (`#calendar-sources-list`) and "+ Add Calendar Source" button in calendar modal.
- [ ] In `data/style.css`:
  - Style calendar source cards, input groups, and preset color picker badges.
- [ ] In `data/app.js`:
  - Dynamically populate sources table/list when opening calendar config.
  - Support adding/removing sources and selecting color swatches.
  - Save sources array into `config.cards.find(c => c.id === 'calendar').config.sources`.

---

### Task 6: Hardware Build, Flash & Verification
**Commands:**
- `pio run -t uploadfs --upload-port /dev/cu.usbmodem211101`
- `pio run -t upload --upload-port /dev/cu.usbmodem211101`
- `python3 scripts/capture_screen.py --port /dev/cu.usbmodem211101 --out artifacts/screen_multi_calendar.png`

- [ ] Compile and upload LittleFS web assets.
- [ ] Compile and flash firmware binary.
- [ ] Capture physical display and verify colored dots alongside upcoming agenda events.
