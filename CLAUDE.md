# KidClock

> Read this file first. It wins over all other docs if there's a conflict — except for **current execution state**, where `docs/agent-handoff.md` wins (see Read Order below).

## Read Order

1. **`docs/agent-handoff.md`** — Current focus, recent decisions, next up. Read first when picking up work.
2. **`docs/INDEX.md`** — Card catalog of all documentation; use to find the right spoke.
3. **This hub** — Binding decisions, stop rules, critical gotchas.

For immutable decisions, **`docs/adr/`** is authoritative over narrative docs.

## Documentation Map

| Spoke | When to read |
|-------|--------------|
| docs/agent-handoff.md | Current milestone, task, blockers, what just happened, next up |
| docs/INDEX.md | Find any doc by path; one-line summaries (card catalog) |
| docs/product-brief.md | Vision, users, success criteria |
| docs/architecture.md | System design, data flows, boundaries |
| docs/codebase-map.md | Where code and key files live |
| docs/developer-guide.md | Build, flash, test, local setup (PlatformIO) |
| docs/hardware-guide.md | Wiring, components, pin reference, troubleshooting hardware |
| docs/adr/ | Recorded decisions (immutable history) |
| docs/runbooks/*.md | Target-specific hello-world and operations |
| docs/troubleshooting/ | Known failures and fixes (see troubleshooting/INDEX.md) |
| docs/issues/README.md | How issues are mirrored; tracker is source of truth |
| docs/plans/ | Implementation plans for features in progress |
| docs/_reference/ | Hub-and-spoke doctrine, blueprints catalog, runbook protocol |

## First Post-Scaffold Session

After hello-world succeeds:
1. Revisit `docs/product-brief.md` scope with the user.
2. Use **`issue-architect`** to decompose scope into epics, milestones, and iterations using GitHub's full feature set.
3. Structure the roadmap IN the tracker — not as a separate doc. The tracker is the authoritative planning artifact.
4. Update `docs/agent-handoff.md` to reflect the active milestone and tracker roadmap URL.
5. Begin development loop: pick issue from tracker → implement → verify → handoff-keeper → next.

## Binding Decisions

1. Before proposing fixes for failures, search `docs/troubleshooting/INDEX.md` for matching symptoms — reuse known solutions before investigating fresh.
2. **Build system is PlatformIO** — Arduino framework targeting `esp32-c3-devkitm-1`. No Arduino IDE, no ESP-IDF raw builds.
3. **Display library is TFT_eSPI** — ST7789 driver, 320×240 landscape. Do not switch to Adafruit_ST7789 or LovyanGFX without an ADR.
4. **Control UI is web over WiFi** — served from LittleFS on the ESP32 itself. No native app, no BLE control panel.
5. **Weather via Open-Meteo** — free, no API key. Do not introduce paid weather APIs without explicit approval.
6. **No secrets in repo** — WiFi credentials, API tokens, and location data live in LittleFS JSON on-device, never committed.
7. **Conventional Commits** — use `feat`, `fix`, `chore`, `docs`, `refactor`, `test` prefixes.
8. **Git Workflow is Mandatory** — **CRITICAL FIRST STEP: Before making ANY code edits, run `git branch --show-current` to ensure you are NOT on `main`.** If you are on `main`, immediately use `git checkout -b <prefix>/<#-short-description>` to create a feature branch for the active issue. Follow this strict flow: Create/Validate Issue → Branch from Issue → Implement & Commit → Push → Create PR → Review & Remediate → Human User Merges PR. No AI agent should ever merge a PR. See `CONTRIBUTING.md` for details.
9. **Screen orientation is landscape** — 320w × 240h. Display code must initialize in landscape rotation.
10. **Night mode uses time range** — configurable start/end hours, with architecture hook for future ambient light sensor.

## Stop Rules

Stop and ask a human if:
- Changing hardware wiring, pin assignments, or adding new hardware components.
- Changing the display driver or SPI configuration.
- Adding authentication or exposing the device beyond local WiFi.
- Introducing a paid API or service dependency.
- Changing the web control protocol (HTTP REST endpoints).

## Critical Gotchas

- **ESP32-C3 has ONE usable SPI bus** — SPI0 and SPI1 are reserved for flash. All external SPI devices share SPI2.
- **Strapping pins** — GPIO 2 (safe for output), GPIO 8 (onboard LED), GPIO 9 (BOOT button). Avoid using GPIO 8/9 for peripherals.
- **ESP32-C3 Super Mini has limited GPIO** — only ~11 usable pins. Plan pin allocation carefully for future sensors/LEDs.
- **TFT_eSPI config is compile-time** — pin definitions go in a `User_Setup.h` file, not runtime config. Changing pins requires recompile.
- **LittleFS partition size** — default partition may be too small for web UI assets. Use a custom partition table if needed.
- **Open-Meteo rate limits** — no API key, but respect rate limits. Cache weather data; fetch no more than every 15 minutes.
- **NTP requires WiFi** — clock cannot sync time without network. Must gracefully handle WiFi disconnection.
- **3.3V only** — the display and ESP32-C3 are both 3.3V logic. No 5V tolerant pins. If adding 5V LEDs, power them separately.
