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
import termios
from pathlib import Path
from PIL import Image

def find_serial_port():
    ports = glob.glob("/dev/cu.usbmodem*")
    if ports:
        return ports[0]
    return "/dev/cu.usbmodem1101"

def open_serial_port(port, baudrate=115200):
    fd = os.open(port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    # Configure terminal attributes (raw mode, 115200 8N1)
    attrs = termios.tcgetattr(fd)
    attrs[0] = 0 # iflag
    attrs[1] = 0 # oflag
    attrs[2] = termios.CS8 | termios.CREAD | termios.CLOCAL # cflag
    attrs[3] = 0 # lflag
    attrs[4] = termios.B115200 # ispeed
    attrs[5] = termios.B115200 # ospeed
    termios.tcsetattr(fd, termios.TCSANOW, attrs)
    return fd

def read_line(fd, timeout=3.0):
    start = time.time()
    buf = bytearray()
    while time.time() - start < timeout:
        try:
            chunk = os.read(fd, 1)
            if chunk:
                if chunk == b'\n':
                    return buf.decode("utf-8", errors="ignore").strip()
                elif chunk != b'\r':
                    buf.extend(chunk)
            else:
                time.sleep(0.005)
        except BlockingIOError:
            time.sleep(0.005)
    return buf.decode("utf-8", errors="ignore").strip() if buf else None

def capture_screen(port=None, output_path="artifacts/screen_capture.png"):
    if not port:
        port = find_serial_port()

    print(f"[capture] Opening serial port: {port}")
    fd = open_serial_port(port, 115200)
    time.sleep(0.5)

    # Flush input buffer
    termios.tcflush(fd, termios.TCIFLUSH)

    print("[capture] Sending 'cap' command...")
    os.write(fd, b"cap\n")

    # Wait for header
    start_time = time.time()
    header_found = False
    while time.time() - start_time < 5.0:
        line = read_line(fd, timeout=1.0)
        if line and "===CAPTURE_PPM_B64_START===" in line:
            header_found = True
            break

    if not header_found:
        print("[capture] Error: Timeout waiting for ===CAPTURE_PPM_B64_START===")
        os.close(fd)
        return False

    magic = read_line(fd, timeout=2.0)
    dims = read_line(fd, timeout=2.0)
    max_val = read_line(fd, timeout=2.0)

    if not dims:
        print("[capture] Error: Failed to read image dimensions")
        os.close(fd)
        return False

    width, height = [int(x) for x in dims.split()]
    print(f"[capture] Receiving stream for {width}x{height} image...")

    b64_chunks = []
    while time.time() - start_time < 12.0:
        line = read_line(fd, timeout=1.0)
        if line is None:
            continue
        if "===CAPTURE_PPM_B64_END===" in line:
            break
        if line:
            b64_chunks.append(line)

    os.close(fd)

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
