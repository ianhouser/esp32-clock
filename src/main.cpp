#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "TimeManager.h"
#include "DisplayManager.h"
#include "WeatherManager.h"
#include "UIManager.h"

// Secure credentials fallback if secrets.h is not yet created
#if __has_include("secrets.h")
#include "secrets.h"
#else
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define NTP_SERVER_1  "pool.ntp.org"
#define NTP_SERVER_2  "time.nist.gov"
#define NTP_SERVER_3  "time.google.com"
#define TIMEZONE_TZ   "PST8PDT,M3.2.0,M11.1.0"
#endif

// Fallbacks for optional custom weather parameters in secrets.h
#ifndef WEATHER_LAT
#define WEATHER_LAT DEFAULT_WEATHER_LAT
#endif
#ifndef WEATHER_LON
#define WEATHER_LON DEFAULT_WEATHER_LON
#endif
#ifndef WEATHER_USE_FAHR
#define WEATHER_USE_FAHR DEFAULT_WEATHER_USE_FAHR
#endif

TFT_eSPI tft = TFT_eSPI();
TimeManager timeManager;
DisplayManager displayManager;
WeatherManager weatherManager;
UIManager uiManager;

void setup() {
    Serial.begin(115200);
    unsigned long startSerialWait = millis();
    while (!Serial && (millis() - startSerialWait < 1500)) {
        delay(10);
    }

    Serial.println("\n==============================================");
    Serial.println("  esp32-clock — LVGL Modern Dashboard");
    Serial.println("==============================================");

    // Hardware reset pulse to ensure ST7789 wakes up
    pinMode(PIN_TFT_RST, OUTPUT);
    digitalWrite(PIN_TFT_RST, HIGH);
    delay(50);
    digitalWrite(PIN_TFT_RST, LOW);
    delay(50);
    digitalWrite(PIN_TFT_RST, HIGH);
    delay(150);

    // Initialize ST7789 Display
    tft.init();
    tft.setRotation(SCREEN_ROTATION);
    tft.invertDisplay(SCREEN_INVERT_DISPLAY);
    tft.fillScreen(TFT_BLACK);

    // Initialize Display Manager (schedule & backlight)
    displayManager.begin();

    // Initialize LVGL UI Manager
    uiManager.begin(&tft);

    // Initialize Weather Manager with coordinates & units
    weatherManager.begin(WEATHER_LAT, WEATHER_LON, WEATHER_USE_FAHR);

    // Start WiFi & SNTP synchronization
    timeManager.begin(WIFI_SSID, WIFI_PASSWORD, TIMEZONE_TZ, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);
}

void loop() {
    // Process WiFi & SNTP state machine
    timeManager.update();

    // Poll Open-Meteo weather when online (non-blocking)
    bool newWeatherData = weatherManager.update(timeManager.isConnected());
    if (newWeatherData) {
        struct tm curTime;
        int currentWday = timeManager.getLocalTime(curTime) ? curTime.tm_wday : 0;
        uiManager.updateWeather(weatherManager.getData(),
                                weatherManager.isUsingFahrenheit(),
                                currentWday);
    }

    // Process Serial Commands (theme toggle, screen capture)
    static int s_forcedMode = 0; // 0 = Auto, 1 = Force Day, 2 = Force Night
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        cmd.toLowerCase();
        if (cmd == "day") {
            s_forcedMode = 1;
            uiManager.setTheme(DisplayMode::DAY);
            Serial.println("[CMD] Theme override: DAY mode forced");
        } else if (cmd == "night") {
            s_forcedMode = 2;
            uiManager.setTheme(DisplayMode::NIGHT);
            Serial.println("[CMD] Theme override: NIGHT mode forced");
        } else if (cmd == "auto") {
            s_forcedMode = 0;
            uiManager.setTheme(displayManager.getMode());
            Serial.println("[CMD] Theme override cleared: AUTO schedule resumed");
        } else if (cmd == "cap" || cmd == "screenshot") {
            Serial.println("[CMD] Initiating screen framebuffer capture...");
            uiManager.requestCapture();
        } else if (cmd.length() > 0) {
            Serial.printf("[CMD] Unknown command '%s'. Supported: day, night, auto, cap\n", cmd.c_str());
        }
    }

    // Check time-based day/night schedule transition (only if not forced)
    struct tm timeinfo;
    bool hasLocalTime = timeManager.getLocalTime(timeinfo);
    if (hasLocalTime && (s_forcedMode == 0)) {
        bool modeChanged = displayManager.update(timeinfo.tm_hour, timeinfo.tm_min);
        if (modeChanged) {
            uiManager.setTheme(displayManager.getMode());
        }
    }

    // Update UI clock time and date (12-hour format)
    static int lastSecond = -1;
    if (hasLocalTime && (timeinfo.tm_sec != lastSecond)) {
        lastSecond = timeinfo.tm_sec;

        int hour12 = timeinfo.tm_hour % 12;
        if (hour12 == 0) hour12 = 12;
        const char* ampm = (timeinfo.tm_hour >= 12) ? "PM" : "AM";

        char timeBuf[16];
        snprintf(timeBuf, sizeof(timeBuf), "%d:%02d:%02d",
                 hour12, timeinfo.tm_min, timeinfo.tm_sec);
        char dateBuf[48];
        strftime(dateBuf, sizeof(dateBuf), "%A, %B %d, %Y", &timeinfo);
        uiManager.updateTime(timeBuf, ampm, dateBuf);
    } else if (!hasLocalTime && (lastSecond != -99)) {
        lastSecond = -99;
        uiManager.updateTime("--:--:--", "", "Awaiting NTP Synchronization...");
    }

    // Update network and weather widgets every 1 second
    static unsigned long lastTelemetryUpdate = 0;
    if (millis() - lastTelemetryUpdate >= 1000) {
        lastTelemetryUpdate = millis();

        int currentWday = hasLocalTime ? timeinfo.tm_wday : 0;
        uiManager.updateWeather(weatherManager.getData(),
                                weatherManager.isUsingFahrenheit(),
                                currentWday);

        uiManager.updateNetwork(timeManager.getSSID(),
                                timeManager.getLocalIP().toString().c_str(),
                                timeManager.getRSSI(),
                                timeManager.isConnected(),
                                timeManager.isSynchronized() ? "Locked" : timeManager.getStateString());
    }

    // Advance LVGL timer and render queue
    uiManager.loop();

    delay(5);
}
