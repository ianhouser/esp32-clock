# Troubleshooting Index

Known issues and solutions. Search here before investigating fresh.

| Symptom | Root Cause | Solution | Added |
|---------|------------|----------|-------|
| Screen backlight is on, but screen stays blank / no pixels render | Miswired SPI line (e.g. VCC and SCL swapped) or floating reset | Double check pin connections against `docs/hardware-guide.md`. Verify VCC is on 3V3, SCL is on GPIO 6, SDA on GPIO 7, DC on GPIO 2, RST on GPIO 3, and CS on GPIO 10. | 2026-10-04 |
| Upload fails with port busy or not detected | Device in deep sleep or wrong cable | Ensure USB-C cable supports data (not charge-only). Hold BOOT (GPIO 9), press and release RST, then release BOOT to force download mode. | 2026-10-04 |

## How to Add Entries

When you encounter and solve a problem:
1. Add a row to the table above with the symptom and solution.
2. If the solution is complex, create a `docs/troubleshooting/<topic>.md` file and link to it.
3. Update `docs/INDEX.md` if you add a new file.
