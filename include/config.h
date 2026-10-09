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
#define SCREEN_INVERT_DISPLAY true // Required for ST7789 IPS panels (0x0000=Black, 0xFFFF=White)

// Time-Based Night Mode Schedule (24-hour format)
#define NIGHT_MODE_START_HOUR 22  // 10:00 PM
#define NIGHT_MODE_START_MIN  0
#define NIGHT_MODE_END_HOUR   7   // 7:00 AM
#define NIGHT_MODE_END_MIN    0

// Optional Hardware Backlight Dimming (LEDC PWM)
// Set to -1 if using 7-pin GMT020-02-7P (no dedicated BL pin; uses software palette dimming)
#define PIN_TFT_BL          -1
#define PWM_BL_CHANNEL       0
#define PWM_BL_FREQ       5000
#define PWM_BL_RESOLUTION    8
#define PWM_BL_DAY_DUTY    255
#define PWM_BL_NIGHT_DUTY   38   // ~15% duty for dark environments

// Weather Telemetry Configuration (Open-Meteo REST API)
#define DEFAULT_WEATHER_LAT           37.7749f   // Default: San Francisco, CA
#define DEFAULT_WEATHER_LON          -122.4194f
#define DEFAULT_WEATHER_USE_FAHR      true       // true = Fahrenheit (°F), false = Celsius (°C)
#define WEATHER_UPDATE_INTERVAL_MS    (15 * 60 * 1000UL) // 15 minutes between API queries
#define WEATHER_RETRY_INTERVAL_MS     (60 * 1000UL)      // 1 minute retry on failure

// Reserved Pins for Future Expansions
// #define PIN_LED_DATA   0  // Future: WS2812B RGBIC LED
// #define PIN_LIGHT_ADC  1  // Future: Ambient Light Sensor (ADC)
// #define PIN_I2C_SDA    4  // Future: BME680 Air Quality Sensor
// #define PIN_I2C_SCL    5  // Future: BME680 Air Quality Sensor
