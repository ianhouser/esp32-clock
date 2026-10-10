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
    void updateTime(const char* timeStr, const char* ampmStr, const char* dateStr);
    void updateWeather(const WeatherData& data, bool isFahrenheit, int todayWeekday);
    void updateCalendar(const char* title,
                        const char* ev1Title, const char* ev1Time, uint32_t ev1Color = 0,
                        const char* ev2Title = "", uint32_t ev2Color = 0);
    void updateNetwork(const char* ssid, const char* ip, int8_t rssi, bool connected, const char* syncState) {}
    void setTheme(DisplayMode mode);
    void reloadConfig();
    void refreshCards();

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

    // 1. Hero Clock Card (Top)
    lv_obj_t* _cardClock;
    lv_obj_t* _lblTime;
    lv_obj_t* _badgeAmPm;
    lv_obj_t* _lblAmPm;
    lv_obj_t* _lblDate;

    // 2. Current Weather Card (Bottom Left)
    lv_obj_t* _cardWeather;
    lv_obj_t* _iconWeather;
    lv_obj_t* _lblWeatherTemp;
    lv_obj_t* _lblWeatherDesc;
    lv_obj_t* _lblWeatherHighLow;
    lv_obj_t* _lblWeatherMetrics;

    // 3. 2-Day Forecast Card (Bottom Right)
    lv_obj_t* _cardForecast;
    lv_obj_t* _lblForecastTitle;

    lv_obj_t* _lblDay1Name;
    lv_obj_t* _iconDay1;
    lv_obj_t* _lblDay1HighLow;

    lv_obj_t* _lblDay2Name;
    lv_obj_t* _iconDay2;
    lv_obj_t* _lblDay2HighLow;

    // 4. Calendar Card
    lv_obj_t* _cardCalendar;
    lv_obj_t* _lblCalendarTitle;
    lv_obj_t* _lblEvent1Title;
    lv_obj_t* _lblEvent1Time;
    lv_obj_t* _lblEvent2Title;
    lv_obj_t* _dotEvent1;
    lv_obj_t* _dotEvent2;

    // 5. Notifications Card
    lv_obj_t* _cardNotifications;
    lv_obj_t* _lblNotifTitle;
    lv_obj_t* _lblEmailAlert;
    lv_obj_t* _lblMsgAlert;

    // 6. System Diagnostics Card
    lv_obj_t* _cardSystem;
    lv_obj_t* _lblSysTitle;
    lv_obj_t* _lblSysWifi;
    lv_obj_t* _lblSysHeap;
    lv_obj_t* _lblSysUptime;

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
