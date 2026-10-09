#!/usr/bin/env python3
"""
Screen capture utility for esp32-clock.
Connects over USB CDC serial, sends 'cap', reads the Base64 PPM stream,
and saves the captured frame as PNG and PPM.
"""

import sys
import time
import base64
import os
import glob
from pathlib import Path
import serial
from PIL import Image

def find_serial_port():
    ports = glob.glob("/dev/cu.usbmodem*")
    if ports:
        return ports[0]
    return "/dev/cu.usbmodem1101"

def capture_screen(port=None, output_path="artifacts/screen_capture.png"):
    if not port:
        port = find_serial_port()

    print(f"[capture] Opening serial port: {port}")
    ser = serial.Serial(port, 115200, timeout=3)
    time.sleep(0.5)

    # Flush input buffer
    ser.reset_input_buffer()

    print("[capture] Sending 'cap' command...")
    ser.write(b"cap\n")
    ser.flush()

    # Wait for header
    start_time = time.time()
    header_found = False
    while time.time() - start_time < 5.0:
        line = ser.readline().decode("utf-8", errors="ignore").strip()
        if "===CAPTURE_PPM_B64_START===" in line:
            header_found = True
            break

    if not header_found:
        print("[capture] Error: Timeout waiting for ===CAPTURE_PPM_B64_START===")
        ser.close()
        return False

    magic = ser.readline().decode("utf-8", errors="ignore").strip()
    dims = ser.readline().decode("utf-8", errors="ignore").strip()
    max_val = ser.readline().decode("utf-8", errors="ignore").strip()

    width, height = [int(x) for x in dims.split()]
    print(f"[capture] Receiving stream for {width}x{height} image...")

    b64_chunks = []
    while time.time() - start_time < 10.0:
        line = ser.readline().decode("utf-8", errors="ignore").strip()
        if "===CAPTURE_PPM_B64_END===" in line:
            break
        if line:
            b64_chunks.append(line)

    ser.close()

    full_b64 = "".join(b64_chunks)
    raw_rgb = base64.b64decode(full_b64)
    expected_bytes = width * height * 3

    if len(raw_rgb) != expected_bytes:
        print(f"[capture] Warning: received {len(raw_rgb)} bytes, expected {expected_bytes}")
        # Pad or slice if slightly off
        if len(raw_rgb) < expected_bytes:
            raw_rgb = raw_rgb.ljust(expected_bytes, b'\x00')
        else:
            raw_rgb = raw_rgb[:expected_bytes]

    out_file = Path(output_path)
    out_file.parent.mkdir(parents=True, exist_ok=True)

    img = Image.frombytes("RGB", (width, height), raw_rgb)
    img.save(str(out_file))
    print(f"[capture] Successfully saved screenshot to: {out_file.resolve()}")
    return str(out_file.resolve())

if __name__ == "__main__":
    out = sys.argv[1] if len(sys.argv) > 1 else "artifacts/screen_capture.png"
    capture_screen(output_path=out)
