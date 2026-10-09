#pragma once

#include "config.h"

// ==========================================
// TFT_eSPI Configuration for esp32-clock
// Display: GMT020-02-7P (ST7789, 240x320 IPS)
// Controller: ESP32-C3 Super Mini
// ==========================================

#define ST7789_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// ST7789 on GMT020-02-7P requires TFT_BGR so MADCTL bit 3 is set (0x08), correctly mapping RGB565 to the physical subpixels
#define TFT_RGB_ORDER TFT_BGR

// Pin mappings linked to single source of truth in config.h
#define TFT_CS   PIN_TFT_CS
#define TFT_RST  PIN_TFT_RST
#define TFT_DC   PIN_TFT_DC
#define TFT_MOSI PIN_TFT_MOSI
#define TFT_SCLK PIN_TFT_SCLK

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
