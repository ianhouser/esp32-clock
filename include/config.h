#pragma once

#include <Arduino.h>

// ==========================================
// Hardware Pin Configuration
// GMT020-02-7P 2.0" IPS TFT (ST7789, 4-wire SPI)
// ==========================================

#define PIN_TFT_CS    10  // Chip Select
#define PIN_TFT_RST    3  // Reset
#define PIN_TFT_DC     2  // Data / Command
#define PIN_TFT_MOSI   7  // SPI Data (Display SDA)
#define PIN_TFT_SCLK   6  // SPI Clock (Display SCL)

// Display Specifications
#define SCREEN_WIDTH   320
#define SCREEN_HEIGHT  240
#define SCREEN_ROTATION  1 // Landscape (0=portrait, 1=landscape 90 deg, 2=inv portrait, 3=inv landscape)

// Reserved Pins for Future Expansions
// #define PIN_LED_DATA   0  // Future: WS2812B RGBIC LED
// #define PIN_LIGHT_ADC  1  // Future: Ambient Light Sensor (ADC)
// #define PIN_I2C_SDA    4  // Future: BME680 Air Quality Sensor
// #define PIN_I2C_SCL    5  // Future: BME680 Air Quality Sensor
