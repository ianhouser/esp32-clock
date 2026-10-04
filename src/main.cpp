#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "TimeManager.h"

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

// Color definitions (16-bit 565 format)
#define COLOR_BG          0x0842  // Deep slate navy (RGB 8, 8, 16)
#define COLOR_CARD_BG     0x10A4  // Dark slate card background
#define COLOR_CARD_BORDER 0x2969  // Subtle card border
#define COLOR_CYAN_ACCENT 0x07FD  // Crisp bright cyan
#define COLOR_AMBER       0xFDA0  // Warm amber for warnings
#define COLOR_MUTED_TEXT  0x9CD3  // Muted light slate gray
#define COLOR_HEADER_BG   0x0926  // Deep dark cyan-navy for header

TFT_eSPI tft = TFT_eSPI();
TimeManager timeManager;

void drawStaticLayout() {
    int16_t w = tft.width();
    int16_t h = tft.height();

    // Background fill
    tft.fillScreen(COLOR_BG);

    // Header banner
    tft.fillRect(0, 0, w, 28, COLOR_HEADER_BG);
    tft.drawFastHLine(0, 28, w, COLOR_CARD_BORDER);

    tft.setTextColor(TFT_WHITE, COLOR_HEADER_BG);
    tft.setTextDatum(ML_DATUM);
    tft.setTextFont(2);
    tft.drawString("esp32-clock", 10, 14);

    // Main Clock Card
    const int16_t clockCardX = 8;
    const int16_t clockCardY = 34;
    const int16_t clockCardW = w - 16;
    const int16_t clockCardH = 104;

    tft.fillRoundRect(clockCardX, clockCardY, clockCardW, clockCardH, 6, COLOR_CARD_BG);
    tft.drawRoundRect(clockCardX, clockCardY, clockCardW, clockCardH, 6, COLOR_CARD_BORDER);

    // Info Cards: Network (Left) & Diagnostics (Right)
    const int16_t infoCardY = 144;
    const int16_t infoCardW = (w - 22) / 2;
    const int16_t infoCardH = h - infoCardY - 8;

    // Network Card
    const int16_t netCardX = 8;
    tft.fillRoundRect(netCardX, infoCardY, infoCardW, infoCardH, 6, COLOR_CARD_BG);
    tft.drawRoundRect(netCardX, infoCardY, infoCardW, infoCardH, 6, COLOR_CARD_BORDER);

    tft.setTextColor(COLOR_CYAN_ACCENT, COLOR_CARD_BG);
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(2);
    tft.drawString("NETWORK", netCardX + 8, infoCardY + 6);
    tft.drawFastHLine(netCardX + 8, infoCardY + 23, infoCardW - 16, COLOR_CARD_BORDER);

    // System/Sync Card
    const int16_t sysCardX = netCardX + infoCardW + 6;
    tft.fillRoundRect(sysCardX, infoCardY, infoCardW, infoCardH, 6, COLOR_CARD_BG);
    tft.drawRoundRect(sysCardX, infoCardY, infoCardW, infoCardH, 6, COLOR_CARD_BORDER);

    tft.setTextColor(COLOR_CYAN_ACCENT, COLOR_CARD_BG);
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(2);
    tft.drawString("TIME & SYSTEM", sysCardX + 8, infoCardY + 6);
    tft.drawFastHLine(sysCardX + 8, infoCardY + 23, infoCardW - 16, COLOR_CARD_BORDER);
}

void updateDisplay(bool forceAll = false) {
    static int lastSecond = -1;
    static int lastMinute = -1;
    static TimeSyncState lastState = TimeSyncState::IDLE;
    static int8_t lastRssi = 99;
    static unsigned long lastInfoUpdate = 0;

    struct tm timeinfo;
    bool hasLocalTime = timeManager.getLocalTime(timeinfo);
    int currentSecond = hasLocalTime ? timeinfo.tm_sec : -1;
    int currentMinute = hasLocalTime ? timeinfo.tm_min : -1;
    TimeSyncState currentState = timeManager.getState();

    int16_t w = tft.width();

    // 1. Header Status Update (WiFi + NTP badges)
    if (forceAll || currentState != lastState || (millis() - lastInfoUpdate >= 2000)) {
        // Draw WiFi Status Indicator on top-right
        uint16_t wifiDotColor = TFT_RED;
        const char* wifiLabel = "Offline";

        if (timeManager.isConnected()) {
            wifiDotColor = TFT_GREEN;
            wifiLabel = "Online";
        } else if (currentState == TimeSyncState::CONNECTING_WIFI) {
            wifiDotColor = COLOR_AMBER;
            wifiLabel = "Connecting";
        }

        // Header status area
        tft.setTextDatum(MR_DATUM);
        tft.setTextFont(2);
        tft.setTextColor(TFT_WHITE, COLOR_HEADER_BG);
        tft.setTextPadding(140);

        char headerStatus[32];
        if (timeManager.isConnected()) {
            snprintf(headerStatus, sizeof(headerStatus), "WiFi %d dBm", timeManager.getRSSI());
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

            tft.setTextColor(COLOR_CYAN_ACCENT, COLOR_CARD_BG);
            tft.setTextPadding(280);
            tft.setTextFont(7); // 48px height 7-segment digital font
            tft.drawString(timeBuf, w / 2, 70);
        } else {
            tft.setTextColor(COLOR_AMBER, COLOR_CARD_BG);
            tft.setTextPadding(280);
            tft.setTextFont(7);
            tft.drawString("--:--:--", w / 2, 70);
        }

        // Date Display inside main card
        if (forceAll || currentMinute != lastMinute) {
            lastMinute = currentMinute;
            tft.setTextFont(2);
            tft.setTextColor(TFT_WHITE, COLOR_CARD_BG);
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

        const int16_t netCardX = 8;
        const int16_t infoCardY = 144;
        const int16_t infoCardW = (w - 22) / 2;
        const int16_t sysCardX = netCardX + infoCardW + 6;

        tft.setTextDatum(TL_DATUM);
        tft.setTextFont(2);

        // --- Left: Network Details ---
        tft.setTextColor(COLOR_MUTED_TEXT, COLOR_CARD_BG);
        tft.setTextPadding(infoCardW - 20);

        char netBuf[40];
        // Line 1: SSID
        snprintf(netBuf, sizeof(netBuf), "SSID: %.10s", timeManager.getSSID());
        tft.drawString(netBuf, netCardX + 10, infoCardY + 28);

        // Line 2: IP
        if (timeManager.isConnected()) {
            snprintf(netBuf, sizeof(netBuf), "IP: %s", timeManager.getLocalIP().toString().c_str());
        } else {
            snprintf(netBuf, sizeof(netBuf), "IP: Disconnected");
        }
        tft.drawString(netBuf, netCardX + 10, infoCardY + 46);

        // Line 3: RSSI & State
        if (timeManager.isConnected()) {
            snprintf(netBuf, sizeof(netBuf), "Sig: %d dBm", timeManager.getRSSI());
        } else {
            snprintf(netBuf, sizeof(netBuf), "St: %s", timeManager.getStateString());
        }
        tft.drawString(netBuf, netCardX + 10, infoCardY + 64);

        // --- Right: Time & System Details ---
        tft.setTextColor(COLOR_MUTED_TEXT, COLOR_CARD_BG);
        tft.setTextPadding(infoCardW - 20);

        char sysBuf[40];
        // Line 1: NTP Sync Status
        if (timeManager.isSynchronized()) {
            snprintf(sysBuf, sizeof(sysBuf), "Sync: Locked (NTP)");
        } else {
            snprintf(sysBuf, sizeof(sysBuf), "Sync: %s", timeManager.getStateString());
        }
        tft.drawString(sysBuf, sysCardX + 10, infoCardY + 28);

        // Line 2: Free Heap
        snprintf(sysBuf, sizeof(sysBuf), "Heap: %u KB", (unsigned int)(ESP.getFreeHeap() / 1024));
        tft.drawString(sysBuf, sysCardX + 10, infoCardY + 46);

        // Line 3: Uptime
        uint32_t sec = millis() / 1000;
        uint32_t min = sec / 60;
        uint32_t hrs = min / 60;
        snprintf(sysBuf, sizeof(sysBuf), "Up: %02u:%02u:%02u", hrs, min % 60, sec % 60);
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
    Serial.println("  esp32-clock — WiFi & NTP Digital Clock");
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
    tft.invertDisplay(true); // Required for IPS display panels

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

    // Refresh UI
    updateDisplay(false);

    delay(50);
}
