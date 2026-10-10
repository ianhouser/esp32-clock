#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "TimeManager.h"
#include "DisplayManager.h"
#include "WeatherManager.h"
#include "UIManager.h"
#include "ConfigManager.h"
#include "WebServerManager.h"
#include "cards/CardRegistry.h"
#include "cards/ClockCard.h"
#include "cards/WeatherCard.h"
#include "cards/ForecastCard.h"
#include "cards/CalendarCard.h"
#include "cards/NotificationCard.h"
#include "cards/SystemCard.h"
#include "CalendarManager.h"
#include "NetworkTaskCoordinator.h"

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
CalendarManager calendarManager;
UIManager uiManager;

static ClockCard s_clockCard;
static WeatherCard s_weatherCard;
static ForecastCard s_forecastCard;
static CalendarCard s_calendarCard;
static NotificationCard s_notificationCard;
static SystemCard s_systemCard;
static volatile bool s_configReloadPending = false;

void applyServicesConfig() {
    WeatherCard* wc = (WeatherCard*)CardRegistry::getInstance().getCard("weather");
    if (wc) {
        Serial.printf("[main] Configuring Weather: Lat: %.4f, Lon: %.4f, Fahr: %d, poll: %d min\n",
                      wc->getLat(), wc->getLon(), wc->isFahrenheit(), wc->getUpdateIntervalMin());
        weatherManager.begin(wc->getLat(), wc->getLon(), wc->isFahrenheit(), wc->getUpdateIntervalMin() * 60000UL);
        NetworkTaskCoordinator::getInstance().registerTask(
            "weather",
            wc->getUpdateIntervalMin() * 60000UL,
            2500, // 2.5s after boot
            1,    // Priority 1
            []() {
                bool ok = weatherManager.executeFetch();
                if (ok) {
                    struct tm curTime;
                    int currentWday = timeManager.getLocalTime(curTime) ? curTime.tm_wday : 0;
                    uiManager.updateWeather(weatherManager.getData(),
                                            weatherManager.isUsingFahrenheit(),
                                            currentWday);
                }
                return ok;
            }
        );
    }

    CalendarCard* cc = (CalendarCard*)CardRegistry::getInstance().getCard("calendar");
    if (cc && (!cc->getSources().empty() || cc->getCalendarUrl().length() > 0)) {
        const auto& sources = cc->getSources();
        Serial.printf("[main] Configuring Calendar: %d source(s), Max: %d\n",
                      (int)sources.size(), cc->getMaxEvents());
        calendarManager.begin(sources, cc->getMaxEvents());
        NetworkTaskCoordinator::getInstance().registerTask(
            "calendar",
            15 * 60 * 1000UL, // 15 min poll
            7000,             // 7s after boot (staggered after weather)
            2,                // Priority 2
            []() {
                char currentDateYmd[16] = {0};
                struct tm curTimeCal;
                if (timeManager.getLocalTime(curTimeCal)) {
                    snprintf(currentDateYmd, sizeof(currentDateYmd), "%04d%02d%02d",
                             curTimeCal.tm_year + 1900, curTimeCal.tm_mon + 1, curTimeCal.tm_mday);
                }
                bool ok = calendarManager.executeFetch(currentDateYmd[0] ? currentDateYmd : nullptr);
                if (ok) {
                    const auto& events = calendarManager.getEvents();
                    if (events.empty()) {
                        uiManager.updateCalendar("Upcoming Agenda", "No Upcoming Events", "All caught up", 0, "", 0);
                    } else if (events.size() == 1) {
                        uiManager.updateCalendar("Upcoming Agenda", events[0].summary.c_str(), events[0].timeStr.c_str(), events[0].colorHex, "", 0);
                    } else {
                        uiManager.updateCalendar("Upcoming Agenda", events[0].summary.c_str(), events[0].timeStr.c_str(), events[0].colorHex,
                                                 events[1].summary.c_str(), events[1].colorHex);
                    }
                }
                return ok;
            }
        );
    }
}

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
    // Enforce GMT020-02-7P subpixel mapping: 0x68 = MX (0x40) | MV (0x20) | BGR (0x08)
    tft.writecommand(0x36);
    tft.writedata(0x68);
    tft.fillScreen(TFT_BLACK);

    // Initialize Display Manager (schedule & backlight)
    displayManager.begin();

    // Register modular cards into registry
    CardRegistry::getInstance().registerCard(&s_clockCard);
    CardRegistry::getInstance().registerCard(&s_weatherCard);
    CardRegistry::getInstance().registerCard(&s_forecastCard);
    CardRegistry::getInstance().registerCard(&s_calendarCard);
    CardRegistry::getInstance().registerCard(&s_notificationCard);
    CardRegistry::getInstance().registerCard(&s_systemCard);

    // Mount LittleFS & load configuration
    ConfigManager::getInstance().begin();

    // Hook dynamic config change listener
    ConfigManager::getInstance().onConfigChanged([]() {
        Serial.println("[main] Configuration updated dynamically!");
        s_configReloadPending = true;
        const auto& t = ConfigManager::getInstance().getThemeConfig();
        displayManager.setSchedule(t.nightStartHour, t.nightStartMin, t.nightEndHour, t.nightEndMin);
    });

    // Initialize LVGL UI Manager
    uiManager.begin(&tft);

    // Register UI yield hook for non-blocking network operations
    NetworkTaskCoordinator::getInstance().setYieldCallback([]() {
        uiManager.loop();
    });

    // Apply configured services (Weather coordinates & Calendar iCal)
    applyServicesConfig();

    // Start WiFi & SNTP synchronization
    timeManager.begin(WIFI_SSID, WIFI_PASSWORD, TIMEZONE_TZ, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);
}

void loop() {
    // Process WiFi & SNTP state machine
    timeManager.update();

    // Safely apply pending configuration reloads on the UI thread
    if (s_configReloadPending) {
        s_configReloadPending = false;
        uiManager.reloadConfig();
        applyServicesConfig();
    }

    // Start Web Server once WiFi connects
    static bool s_webServerStarted = false;
    if (timeManager.isConnected() && !s_webServerStarted) {
        WebServerManager::getInstance().begin();
        s_webServerStarted = true;
    }

    // Process queued network operations (sequential, non-blocking, rate-limited)
    NetworkTaskCoordinator::getInstance().loop(timeManager.isConnected());

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
        } else if (cmd == "rgb") {
            tft.writecommand(0x36);
            tft.writedata(0x60);
            Serial.println("[CMD] Display MADCTL bit 3 cleared -> RGB mode forced");
        } else if (cmd == "bgr") {
            tft.writecommand(0x36);
            tft.writedata(0x68);
            Serial.println("[CMD] Display MADCTL bit 3 set -> BGR mode forced");
        } else if (cmd == "cap" || cmd == "screenshot") {
            Serial.println("[CMD] Initiating screen framebuffer capture...");
            uiManager.requestCapture();
        } else if (cmd.length() > 0) {
            Serial.printf("[CMD] Unknown command '%s'. Supported: day, night, auto, rgb, bgr, cap\n", cmd.c_str());
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
