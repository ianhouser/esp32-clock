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

def read_line(fd, timeout=3.0, residual=None):
    start = time.time()
    buf = bytearray() if residual is None else residual
    while time.time() - start < timeout:
        if b'\n' in buf:
            idx = buf.index(b'\n')
            line = buf[:idx].decode("utf-8", errors="ignore").strip()
            del buf[:idx+1]
            return line
        try:
            chunk = os.read(fd, 4096)
            if chunk:
                buf.extend(chunk)
            else:
                time.sleep(0.005)
        except BlockingIOError:
            time.sleep(0.005)
    if buf:
        line = buf.decode("utf-8", errors="ignore").strip()
        buf.clear()
        return line
    return None

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
    residual = bytearray()
    while time.time() - start_time < 5.0:
        line = read_line(fd, timeout=1.0, residual=residual)
        if line and "===CAPTURE_PPM_B64_START===" in line:
            header_found = True
            break

    if not header_found:
        print("[capture] Error: Timeout waiting for ===CAPTURE_PPM_B64_START===")
        os.close(fd)
        return False

    magic = read_line(fd, timeout=2.0, residual=residual)

    if magic == "TILED":
        width, height = 320, 240
        print(f"[capture] Receiving TILED stream for {width}x{height} image...")
        img = Image.new("RGB", (width, height), (0, 0, 0))
        cur_tile = None
        cur_b64 = []

        while time.time() - start_time < 12.0:
            line = read_line(fd, timeout=1.0, residual=residual)
            if line is None:
                continue
            if "===CAPTURE_PPM_B64_END===" in line:
                break
            if line.startswith("TILE:"):
                cur_tile = [int(v) for v in line[5:].split(",")]
                cur_b64 = []
            elif line == "END_TILE":
                if cur_tile and cur_b64:
                    x, y, w, h = cur_tile
                    tile_bytes = base64.b64decode("".join(cur_b64))
                    if len(tile_bytes) == w * h * 3:
                        tile_img = Image.frombytes("RGB", (w, h), tile_bytes)
                        img.paste(tile_img, (x, y))
                cur_tile = None
                cur_b64 = []
            elif line and not line.startswith("[") and " " not in line:
                cur_b64.append(line)

        os.close(fd)
        out_file = Path(output_path)
        out_file.parent.mkdir(parents=True, exist_ok=True)
        img.save(str(out_file))
        print(f"[capture] Successfully saved screenshot to: {out_file.resolve()}")
        return str(out_file.resolve())

    dims = read_line(fd, timeout=2.0, residual=residual)
    max_val = read_line(fd, timeout=2.0, residual=residual)

    if not dims:
        print("[capture] Error: Failed to read image dimensions")
        os.close(fd)
        return False

    width, height = [int(x) for x in dims.split()]
    print(f"[capture] Receiving stream for {width}x{height} image...")

    b64_chunks = []
    while time.time() - start_time < 12.0:
        line = read_line(fd, timeout=1.0, residual=residual)
        if line is None:
            continue
        if "===CAPTURE_PPM_B64_END===" in line:
            idx = line.find("===CAPTURE_PPM_B64_END===")
            prefix = line[:idx].strip()
            if prefix and not prefix.startswith("[") and " " not in prefix:
                b64_chunks.append(prefix)
            break
        if line and not line.startswith("[") and " " not in line:
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

def set_device_mode(port, mode):
    fd = open_serial_port(port, 115200)
    time.sleep(0.2)
    termios.tcflush(fd, termios.TCIFLUSH)
    print(f"[capture] Setting device mode: '{mode}'...")
    os.write(fd, f"{mode}\n".encode("utf-8"))
    time.sleep(0.3)
    os.close(fd)

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="Capture ESP32 display framebuffer over USB CDC")
    parser.add_argument("--port", help="Serial port (auto-detect if omitted)")
    parser.add_argument("--mode", choices=["day", "night", "auto"], help="Set mode before capturing")
    parser.add_argument("--out", default="artifacts/screen_capture.png", help="Output PNG file path")
    parser.add_argument("--keep-mode", action="store_true", help="Do not restore auto mode after capture")
    args = parser.parse_args()

    port = args.port or find_serial_port()

    if args.mode:
        set_device_mode(port, args.mode)

    capture_screen(port=port, output_path=args.out)

    # If a temporary mode was forced and --keep-mode was not passed, return device to auto schedule
    if args.mode and args.mode != "auto" and not args.keep_mode:
        print("[capture] Restoring auto schedule on device...")
        set_device_mode(port, "auto")
