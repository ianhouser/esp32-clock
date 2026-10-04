#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "config.h"

TFT_eSPI tft = TFT_eSPI();

void printTFTDiagnostics() {
    setup_t setup;
    tft.getSetup(setup);

    Serial.println("\n--- TFT_eSPI Diagnostic Info ---");
    Serial.printf("TFT Driver ID : 0x%04X\n", setup.tft_driver);
    Serial.printf("TFT Width     : %d\n", setup.tft_width);
    Serial.printf("TFT Height    : %d\n", setup.tft_height);
    Serial.printf("MOSI Pin      : %d\n", setup.pin_tft_mosi);
    Serial.printf("SCLK Pin      : %d\n", setup.pin_tft_clk);
    Serial.printf("CS Pin        : %d\n", setup.pin_tft_cs);
    Serial.printf("DC Pin        : %d\n", setup.pin_tft_dc);
    Serial.printf("RST Pin       : %d\n", setup.pin_tft_rst);
    Serial.printf("SPI Freq (MHz): %d\n", setup.tft_spi_freq / 10);
    Serial.println("--------------------------------\n");
}

void drawTestScreen() {
    tft.fillScreen(TFT_NAVY);

    // Header banner
    tft.fillRect(0, 0, 320, 36, TFT_DARKCYAN);
    tft.setTextColor(TFT_WHITE, TFT_DARKCYAN);
    tft.setTextDatum(MC_DATUM);
    tft.setTextFont(4);
    tft.drawString("esp32-clock", 160, 18);

    // Subtitle
    tft.setTextColor(TFT_YELLOW, TFT_NAVY);
    tft.setTextDatum(MC_DATUM);
    tft.setTextFont(2);
    tft.drawString("ST7789 2.0 IPS TFT - 320x240", 160, 50);

    // Status box
    tft.drawRect(20, 75, 280, 80, TFT_WHITE);
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(2);

    char buf[64];
    snprintf(buf, sizeof(buf), "MCU: ESP32-C3 @ %u MHz", getCpuFrequencyMhz());
    tft.drawString(buf, 30, 85);

    snprintf(buf, sizeof(buf), "Free Heap: %u KB", (unsigned int)(ESP.getFreeHeap() / 1024));
    tft.drawString(buf, 30, 105);

    tft.drawString("Hardware SPI2 Active", 30, 125);

    // Color bars
    const uint16_t testColors[] = { TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA, TFT_WHITE };
    const int count = sizeof(testColors) / sizeof(testColors[0]);
    const int width = 280 / count;
    for (int i = 0; i < count; i++) {
        tft.fillRect(20 + (i * width), 168, width, 24, testColors[i]);
    }
    tft.drawRect(19, 167, (count * width) + 2, 26, TFT_WHITE);

    tft.setTextColor(TFT_GREEN, TFT_NAVY);
    tft.setTextDatum(MC_DATUM);
    tft.setTextFont(2);
    tft.drawString("Display Test Active", 160, 215);
}

void setup() {
    Serial.begin(115200);
    delay(1500); // Wait for USB CDC

    Serial.println("\n=================================");
    Serial.println("  esp32-clock — Display Test v2");
    Serial.println("=================================");

    // Explicit hardware reset pulse to ensure ST7789 wakes up
    pinMode(PIN_TFT_RST, OUTPUT);
    digitalWrite(PIN_TFT_RST, HIGH);
    delay(50);
    digitalWrite(PIN_TFT_RST, LOW);
    delay(50);
    digitalWrite(PIN_TFT_RST, HIGH);
    delay(150);

    // Initialize display
    tft.init();
    tft.setRotation(1); // Landscape
    tft.invertDisplay(true); // Required for IPS ST7789 panels

    printTFTDiagnostics();

    drawTestScreen();
    Serial.println("[TFT] Initial test screen rendered.");
}

void loop() {
    static unsigned long lastUpdate = 0;
    static int cycle = 0;

    if (millis() - lastUpdate >= 2000) {
        lastUpdate = millis();
        cycle++;

        // Flash small heartbeat dot
        uint16_t dotColor = (cycle % 2 == 0) ? TFT_GREEN : TFT_BLACK;
        tft.fillCircle(300, 18, 5, dotColor);

        Serial.printf("[Loop] Heartbeat #%d | Uptime: %lu s | Free Heap: %u bytes\n",
                      cycle, millis() / 1000, ESP.getFreeHeap());
    }

    delay(20);
}
