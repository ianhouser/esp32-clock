#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "TimeManager.h"
#include "DisplayManager.h"
#include "WeatherManager.h"

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

void drawStaticLayout() {
    int16_t w = tft.width();
    int16_t h = tft.height();
    const ThemeColors& theme = displayManager.getTheme();

    // Background fill
    tft.fillScreen(theme.bg);

    // Header banner
    tft.fillRect(0, 0, w, 28, theme.headerBg);
    tft.drawFastHLine(0, 28, w, theme.cardBorder);

    tft.setTextColor(theme.dateColor, theme.headerBg);
    tft.setTextDatum(ML_DATUM);
    tft.setTextFont(2);
    tft.drawString("esp32-clock", 10, 14);

    // Mode Badge (DAY / NIGHT)
    tft.fillRoundRect(98, 4, 52, 20, 4, theme.modeBadgeBg);
    tft.setTextColor(theme.modeBadgeText, theme.modeBadgeBg);
    tft.setTextDatum(MC_DATUM);
    tft.setTextFont(2);
    tft.drawString(theme.modeLabel, 124, 14);

    // Main Clock Card
    const int16_t clockCardX = 8;
    const int16_t clockCardY = 34;
    const int16_t clockCardW = w - 16;
    const int16_t clockCardH = 104;

    tft.fillRoundRect(clockCardX, clockCardY, clockCardW, clockCardH, 6, theme.cardBg);
    tft.drawRoundRect(clockCardX, clockCardY, clockCardW, clockCardH, 6, theme.cardBorder);

    // Info Cards: Weather (Left) & System (Right)
    const int16_t infoCardY = 144;
    const int16_t infoCardW = (w - 22) / 2;
    const int16_t infoCardH = h - infoCardY - 8;

    // Weather Card
    const int16_t weatherCardX = 8;
    tft.fillRoundRect(weatherCardX, infoCardY, infoCardW, infoCardH, 6, theme.cardBg);
    tft.drawRoundRect(weatherCardX, infoCardY, infoCardW, infoCardH, 6, theme.cardBorder);

    tft.setTextColor(theme.accentColor, theme.cardBg);
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(2);
    tft.drawString("WEATHER", weatherCardX + 8, infoCardY + 6);
    tft.drawFastHLine(weatherCardX + 8, infoCardY + 23, infoCardW - 16, theme.cardBorder);

    // System/Network Card
    const int16_t sysCardX = weatherCardX + infoCardW + 6;
    tft.fillRoundRect(sysCardX, infoCardY, infoCardW, infoCardH, 6, theme.cardBg);
    tft.drawRoundRect(sysCardX, infoCardY, infoCardW, infoCardH, 6, theme.cardBorder);

    tft.setTextColor(theme.accentColor, theme.cardBg);
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(2);
    tft.drawString("SYSTEM & NET", sysCardX + 8, infoCardY + 6);
    tft.drawFastHLine(sysCardX + 8, infoCardY + 23, infoCardW - 16, theme.cardBorder);
}

void updateDisplay(bool forceAll = false) {
    static int lastSecond = -1;
    static int lastMinute = -1;
    static TimeSyncState lastState = TimeSyncState::IDLE;
    static int8_t lastRssi = 99;
    static unsigned long lastHeaderUpdate = 0;
    static unsigned long lastInfoUpdate = 0;

    const ThemeColors& theme = displayManager.getTheme();

    struct tm timeinfo;
    bool hasLocalTime = timeManager.getLocalTime(timeinfo);
    int currentSecond = hasLocalTime ? timeinfo.tm_sec : -1;
    int currentMinute = hasLocalTime ? timeinfo.tm_min : -1;
    TimeSyncState currentState = timeManager.getState();
    int8_t currentRssi = timeManager.getRSSI();

    int16_t w = tft.width();

    // 1. Header Status Update (WiFi + NTP badges)
    if (forceAll || currentState != lastState || currentRssi != lastRssi || (millis() - lastHeaderUpdate >= 2000)) {
        lastHeaderUpdate = millis();
        lastRssi = currentRssi;

        // Draw WiFi Status Indicator on top-right
        uint16_t wifiDotColor = TFT_RED;
        const char* wifiLabel = "Offline";

        if (timeManager.isConnected()) {
            wifiDotColor = TFT_GREEN;
            wifiLabel = "Online";
        } else if (currentState == TimeSyncState::CONNECTING_WIFI) {
            wifiDotColor = 0xFDA0; // Amber
            wifiLabel = "Connecting";
        }

        // Header status area
        tft.setTextDatum(MR_DATUM);
        tft.setTextFont(2);
        tft.setTextColor(theme.dateColor, theme.headerBg);
        tft.setTextPadding(130);

        char headerStatus[32];
        if (timeManager.isConnected()) {
            snprintf(headerStatus, sizeof(headerStatus), "WiFi %d dBm", currentRssi);
        } else {
            snprintf(headerStatus, sizeof(headerStatus), "%s", wifiLabel);
        }
        tft.drawString(headerStatus, w - 24, 14);

        // Status circle dot
        tft.fillCircle(w - 12, 14, 4, wifiDotColor);
    }

    // 2. Main Clock Time (HH:MM:SS)
    if (forceAll || currentSecond != lastSecond) {
        lastSecond = currentSecond;

        tft.setTextDatum(MC_DATUM);

        if (hasLocalTime) {
            // Hours and Minutes in large 7-segment font (Font 7)
            char timeBuf[16];
            snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d",
                     timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

            tft.setTextColor(theme.timeColor, theme.cardBg);
            tft.setTextPadding(280);
            tft.setTextFont(7); // 48px height 7-segment digital font
            tft.drawString(timeBuf, w / 2, 70);
        } else {
            tft.setTextColor(theme.accentColor, theme.cardBg);
            tft.setTextPadding(280);
            tft.setTextFont(7);
            tft.drawString("--:--:--", w / 2, 70);
        }

        // Date Display inside main card
        if (forceAll || currentMinute != lastMinute) {
            lastMinute = currentMinute;
            tft.setTextFont(2);
            tft.setTextColor(theme.dateColor, theme.cardBg);
            tft.setTextPadding(280);

            if (hasLocalTime) {
                char dateBuf[48];
                strftime(dateBuf, sizeof(dateBuf), "%A, %B %d, %Y", &timeinfo);
                tft.drawString(dateBuf, w / 2, 116);
            } else {
                tft.drawString("Awaiting NTP Synchronization...", w / 2, 116);
            }
        }
    }

    // 3. Lower Detail Cards (Updated every 1 second)
    if (forceAll || (millis() - lastInfoUpdate >= 1000)) {
        lastInfoUpdate = millis();
        lastState = currentState;

        const int16_t weatherCardX = 8;
        const int16_t infoCardY = 144;
        const int16_t infoCardW = (w - 22) / 2;
        const int16_t sysCardX = weatherCardX + infoCardW + 6;

        tft.setTextDatum(TL_DATUM);
        tft.setTextFont(2);

        // --- Left: Weather Details ---
        tft.setTextPadding(infoCardW - 20);

        if (weatherManager.hasValidData()) {
            const WeatherData& wd = weatherManager.getData();
            char unitChar = weatherManager.isUsingFahrenheit() ? 'F' : 'C';

            // Line 1: Temperature & Conditions
            char line1Buf[40];
            snprintf(line1Buf, sizeof(line1Buf), "%.0f°%c  %.9s",
                     wd.temperature, unitChar, wd.conditionText);
            tft.setTextColor(theme.accentColor, theme.cardBg);
            tft.drawString(line1Buf, weatherCardX + 10, infoCardY + 28);

            // Line 2: Apparent "Feels like" Temperature
            char line2Buf[40];
            snprintf(line2Buf, sizeof(line2Buf), "Feels: %.0f°%c", wd.apparentTemperature, unitChar);
            tft.setTextColor(theme.mutedText, theme.cardBg);
            tft.drawString(line2Buf, weatherCardX + 10, infoCardY + 46);

            // Line 3: Humidity
            char line3Buf[40];
            snprintf(line3Buf, sizeof(line3Buf), "Humidity: %d%%", wd.humidity);
            tft.drawString(line3Buf, weatherCardX + 10, infoCardY + 64);
        } else {
            tft.setTextColor(theme.mutedText, theme.cardBg);
            const char* status = timeManager.isConnected() ? weatherManager.getData().statusText : "Awaiting WiFi";
            tft.drawString(status, weatherCardX + 10, infoCardY + 28);
            tft.drawString("Open-Meteo REST", weatherCardX + 10, infoCardY + 46);
            tft.drawString("Telemetry Link", weatherCardX + 10, infoCardY + 64);
        }

        // --- Right: System & Network Details ---
        tft.setTextColor(theme.mutedText, theme.cardBg);
        tft.setTextPadding(infoCardW - 20);

        char sysBuf[40];
        // Line 1: SSID
        snprintf(sysBuf, sizeof(sysBuf), "WiFi: %.10s", timeManager.getSSID());
        tft.drawString(sysBuf, sysCardX + 10, infoCardY + 28);

        // Line 2: IP
        if (timeManager.isConnected()) {
            snprintf(sysBuf, sizeof(sysBuf), "IP: %s", timeManager.getLocalIP().toString().c_str());
        } else {
            snprintf(sysBuf, sizeof(sysBuf), "IP: Disconnected");
        }
        tft.drawString(sysBuf, sysCardX + 10, infoCardY + 46);

        // Line 3: NTP Sync Status
        if (timeManager.isSynchronized()) {
            snprintf(sysBuf, sizeof(sysBuf), "Sync: Locked (NTP)");
        } else {
            snprintf(sysBuf, sizeof(sysBuf), "Sync: %s", timeManager.getStateString());
        }
        tft.drawString(sysBuf, sysCardX + 10, infoCardY + 64);
    }
}

void setup() {
    Serial.begin(115200);
    unsigned long startSerialWait = millis();
    while (!Serial && (millis() - startSerialWait < 1500)) {
        delay(10);
    }

    Serial.println("\n==============================================");
    Serial.println("  esp32-clock — WiFi, NTP & Weather Digital Clock");
    Serial.println("==============================================");

    // Explicit hardware reset pulse to ensure ST7789 wakes up
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
    tft.invertDisplay(SCREEN_INVERT_DISPLAY); // Panel inversion configured in config.h

    // Initialize Display Manager (schedule & backlight)
    displayManager.begin();

    // Initialize Weather Manager with coordinates & units
    weatherManager.begin(WEATHER_LAT, WEATHER_LON, WEATHER_USE_FAHR);

    // Render base frame layout
    drawStaticLayout();

    // Start WiFi & SNTP synchronization
    timeManager.begin(WIFI_SSID, WIFI_PASSWORD, TIMEZONE_TZ, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);

    // Initial display paint
    updateDisplay(true);
}

void loop() {
    // Process WiFi & SNTP state machine
    timeManager.update();

    // Poll Open-Meteo weather when online (non-blocking)
    bool newWeatherData = weatherManager.update(timeManager.isConnected());
    if (newWeatherData) {
        updateDisplay(true); // Redraw info cards with fresh weather telemetry
    }

    // Check time-based day/night schedule transition
    struct tm timeinfo;
    if (timeManager.getLocalTime(timeinfo)) {
        bool modeChanged = displayManager.update(timeinfo.tm_hour, timeinfo.tm_min);
        if (modeChanged) {
            // Re-render whole frame when transitioning themes
            drawStaticLayout();
            updateDisplay(true);
        }
    }

    // Refresh UI
    updateDisplay(false);

    delay(50);
}
