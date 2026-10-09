#pragma once

#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include "DisplayTheme.h"
#include "WeatherManager.h"

class UIManager {
public:
    UIManager();

    // Initialize LVGL, display buffers, driver callbacks, and construct Concept A UI
    void begin(TFT_eSPI* tft);

    // Call regularly in Arduino loop() to advance LVGL timer engine
    void loop();

    // Reactive data update APIs
    void updateTime(const char* timeStr, const char* dateStr);
    void updateWeather(const WeatherData& data, bool isFahrenheit, bool isWiFiConnected);
    void updateNetwork(const char* ssid, const char* ip, int8_t rssi, bool connected, const char* syncState);
    void setTheme(DisplayMode mode);

    // Framebuffer Capture over Serial
    void requestCapture();
    static bool isCaptureActive();

    // Status
    DisplayMode getActiveTheme() const { return _currentMode; }

private:
    TFT_eSPI* _tft;
    DisplayMode _currentMode;

    // LVGL partial display buffer structures
    static const uint32_t BUFFER_LINES = 20;
    static const uint32_t BUFFER_PIXELS = 320 * BUFFER_LINES;
    lv_color_t _buf1[BUFFER_PIXELS];
    lv_color_t _buf2[BUFFER_PIXELS];
    lv_disp_draw_buf_t _dispDrawBuf;
    lv_disp_drv_t _dispDrv;

    // UI Widgets
    lv_obj_t* _scr;
    lv_obj_t* _headerBar;
    lv_obj_t* _lblTitle;
    lv_obj_t* _badgeMode;
    lv_obj_t* _lblBadgeMode;
    lv_obj_t* _lblWifiStatus;

    lv_obj_t* _cardClock;
    lv_obj_t* _lblTime;
    lv_obj_t* _lblDate;

    lv_obj_t* _bottomContainer;
    lv_obj_t* _cardWeather;
    lv_obj_t* _lblWeatherTitle;
    lv_obj_t* _lblWeatherTemp;
    lv_obj_t* _lblWeatherFeels;
    lv_obj_t* _lblWeatherHum;

    lv_obj_t* _cardSystem;
    lv_obj_t* _lblSystemTitle;
    lv_obj_t* _lblSystemSsid;
    lv_obj_t* _lblSystemIp;
    lv_obj_t* _lblSystemSync;

    // Theme Styles
    lv_style_t _styleScr;
    lv_style_t _styleHeader;
    lv_style_t _styleCard;
    lv_style_t _styleBadge;
    lv_style_t _styleTime;
    lv_style_t _styleDate;
    lv_style_t _styleCardTitle;
    lv_style_t _styleMutedText;

    void buildDashboard();
    void applyThemeStyles(DisplayMode mode);

    // Static display flush callback hooked to TFT_eSPI
    static void dispFlushCallback(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p);
};
