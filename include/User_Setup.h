#pragma once

// ==========================================
// TFT_eSPI Configuration for esp32-clock
// Display: GMT020-02-7P (ST7789, 240x320 IPS)
// Controller: ESP32-C3 Super Mini
// ==========================================

#define ST7789_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// Pin mappings for ESP32-C3
#define TFT_CS   10
#define TFT_RST   3
#define TFT_DC    2
#define TFT_MOSI  7
#define TFT_SCLK  6

// Fonts to include
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// Prototyping SPI Clock Frequency: 20MHz for clean signal integrity over Dupont jumpers
#define SPI_FREQUENCY       20000000
#define SPI_READ_FREQUENCY  10000000
