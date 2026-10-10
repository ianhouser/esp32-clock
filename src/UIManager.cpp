#include "UIManager.h"
#include "ConfigManager.h"
#include "cards/CardRegistry.h"
#include "cards/ClockCard.h"
#include "cards/WeatherCard.h"
#include "cards/ForecastCard.h"
#include "cards/CalendarCard.h"
#include "cards/NotificationCard.h"
#include "cards/SystemCard.h"

static TFT_eSPI* s_tft = nullptr;

UIManager::UIManager()
    : _tft(nullptr),
      _currentMode(DisplayMode::DAY),
      _scr(nullptr),
      _cardClock(nullptr),
      _lblTime(nullptr),
      _badgeAmPm(nullptr),
      _lblAmPm(nullptr),
      _lblDate(nullptr),
      _cardWeather(nullptr),
      _iconWeather(nullptr),
      _lblWeatherTemp(nullptr),
      _lblWeatherDesc(nullptr),
      _lblWeatherHighLow(nullptr),
      _lblWeatherMetrics(nullptr),
      _cardForecast(nullptr),
      _lblForecastTitle(nullptr),
      _lblDay1Name(nullptr),
      _iconDay1(nullptr),
      _lblDay1HighLow(nullptr),
      _lblDay2Name(nullptr),
      _iconDay2(nullptr),
      _lblDay2HighLow(nullptr),
      _cardCalendar(nullptr),
      _lblCalendarTitle(nullptr),
      _lblEvent1Title(nullptr),
      _lblEvent1Time(nullptr),
      _lblEvent2Title(nullptr),
      _cardNotifications(nullptr),
      _lblNotifTitle(nullptr),
      _lblEmailAlert(nullptr),
      _lblMsgAlert(nullptr),
      _cardSystem(nullptr),
      _lblSysTitle(nullptr),
      _lblSysWifi(nullptr),
      _lblSysHeap(nullptr),
      _lblSysUptime(nullptr) {}

static bool s_capturePending = false;
static bool s_captureActive = false;
static uint32_t s_capturePixelCount = 0;
static uint8_t s_b64LineLen = 0;

void UIManager::dispFlushCallback(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
    if (s_tft) {
        uint32_t w = (area->x2 - area->x1 + 1);
        uint32_t h = (area->y2 - area->y1 + 1);

        s_tft->startWrite();
        s_tft->setAddrWindow(area->x1, area->y1, w, h);
        s_tft->pushColors((uint16_t*)&color_p->full, w * h, false);
        s_tft->endWrite();

        if (s_capturePending && area->x1 == 0 && area->y1 == 0) {
            s_capturePending = false;
            s_captureActive = true;
            s_capturePixelCount = 0;
            Serial.println("\n===CAPTURE_PPM_B64_START===");
            Serial.println("TILED");
        }

        if (s_captureActive) {
            Serial.printf("TILE:%d,%d,%d,%d\n", area->x1, area->y1, w, h);
            static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            uint32_t totalPixels = w * h;
            uint8_t lineLen = 0;
            for (uint32_t i = 0; i < totalPixels; i++) {
                uint8_t b0 = ((uint8_t*)&color_p[i])[0];
                uint8_t b1 = ((uint8_t*)&color_p[i])[1];
                uint16_t raw = (b0 << 8) | b1;
                uint8_t r = (((raw >> 11) & 0x1F) * 255) / 31;
                uint8_t g = (((raw >> 5) & 0x3F) * 255) / 63;
                uint8_t b = ((raw & 0x1F) * 255) / 31;

                char out[4];
                out[0] = b64[(r >> 2) & 0x3F];
                out[1] = b64[((r & 0x03) << 4) | ((g >> 4) & 0x0F)];
                out[2] = b64[((g & 0x0F) << 2) | ((b >> 6) & 0x03)];
                out[3] = b64[b & 0x3F];

                Serial.write((const uint8_t*)out, 4);
                lineLen += 4;
                if (lineLen >= 64) {
                    Serial.println();
                    lineLen = 0;
                }
            }
            if (lineLen > 0) Serial.println();
            Serial.println("END_TILE");

            s_capturePixelCount += totalPixels;
            if (s_capturePixelCount >= 320 * 240) {
                Serial.println("===CAPTURE_PPM_B64_END===");
                s_captureActive = false;
            }
        }
    }
    lv_disp_flush_ready(disp);
}

void UIManager::requestCapture() {
    s_capturePending = true;
    s_captureActive = false;
    lv_obj_invalidate(lv_scr_act());
}

bool UIManager::isCaptureActive() {
    return s_captureActive;
}

void UIManager::begin(TFT_eSPI* tft) {
    _tft = tft;
    s_tft = tft;

    lv_init();

    lv_disp_draw_buf_init(&_dispDrawBuf, _buf1, _buf2, BUFFER_PIXELS);

    lv_disp_drv_init(&_dispDrv);
    _dispDrv.hor_res = 320;
    _dispDrv.ver_res = 240;
    _dispDrv.flush_cb = dispFlushCallback;
    _dispDrv.draw_buf = &_dispDrawBuf;
    lv_disp_drv_register(&_dispDrv);

    buildDashboard();
    applyThemeStyles(DisplayMode::DAY);

    Serial.println("[UIManager] LVGL v8 initialized with partial double buffers");
}

void UIManager::loop() {
    static uint32_t lastTick = 0;
    uint32_t now = millis();
    if (lastTick > 0 && now > lastTick) {
        lv_tick_inc(now - lastTick);
    }
    lastTick = now;

    lv_timer_handler();
}

static void renderWeatherIcon(lv_obj_t* parent, int weatherCode, bool isNight) {
    if (!parent) return;
    lv_obj_clean(parent);
    lv_obj_set_style_bg_opa(parent, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(parent, 0, 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    if (weatherCode == 0 || weatherCode == 1) {
        if (isNight) {
            // Crescent Moon: silver disc
            lv_obj_t* moon = lv_obj_create(parent);
            lv_obj_set_size(moon, 22, 22);
            lv_obj_set_style_radius(moon, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(moon, lv_color_make(226, 232, 240), 0);
            lv_obj_set_style_bg_opa(moon, LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(moon, 0, 0);
            lv_obj_align(moon, LV_ALIGN_CENTER, 0, 0);

            lv_obj_t* shadow = lv_obj_create(parent);
            lv_obj_set_size(shadow, 18, 18);
            lv_obj_set_style_radius(shadow, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(shadow, lv_color_make(18, 12, 10), 0);
            lv_obj_set_style_bg_opa(shadow, LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(shadow, 0, 0);
            lv_obj_align(shadow, LV_ALIGN_CENTER, 4, -3);
        } else {
            // Radiant Sun: Golden yellow disc with subtle amber rim
            lv_obj_t* sun = lv_obj_create(parent);
            lv_obj_set_size(sun, 22, 22);
            lv_obj_set_style_radius(sun, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(sun, lv_color_make(251, 191, 36), 0);
            lv_obj_set_style_bg_opa(sun, LV_OPA_COVER, 0);
            lv_obj_set_style_border_color(sun, lv_color_make(245, 158, 11), 0);
            lv_obj_set_style_border_width(sun, 2, 0);
            lv_obj_align(sun, LV_ALIGN_CENTER, 0, 0);
        }
    } else if (weatherCode == 2 || weatherCode == 3) {
        // Partly Cloudy / Overcast: Dual rounded cloud layers
        lv_obj_t* cloud1 = lv_obj_create(parent);
        lv_obj_set_size(cloud1, 24, 14);
        lv_obj_set_style_radius(cloud1, 7, 0);
        lv_obj_set_style_bg_color(cloud1, lv_color_make(148, 163, 184), 0);
        lv_obj_set_style_bg_opa(cloud1, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(cloud1, 0, 0);
        lv_obj_align(cloud1, LV_ALIGN_BOTTOM_MID, 0, -2);

        lv_obj_t* cloud2 = lv_obj_create(parent);
        lv_obj_set_size(cloud2, 14, 14);
        lv_obj_set_style_radius(cloud2, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(cloud2, lv_color_make(203, 213, 225), 0);
        lv_obj_set_style_bg_opa(cloud2, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(cloud2, 0, 0);
        lv_obj_align(cloud2, LV_ALIGN_CENTER, -3, -4);
    } else if (weatherCode >= 51 && weatherCode <= 82) {
        // Rain: Cloud with cyan droplets
        lv_obj_t* cloud = lv_obj_create(parent);
        lv_obj_set_size(cloud, 22, 12);
        lv_obj_set_style_radius(cloud, 6, 0);
        lv_obj_set_style_bg_color(cloud, lv_color_make(100, 116, 139), 0);
        lv_obj_set_style_bg_opa(cloud, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(cloud, 0, 0);
        lv_obj_align(cloud, LV_ALIGN_TOP_MID, 0, 2);

        lv_obj_t* drop1 = lv_obj_create(parent);
        lv_obj_set_size(drop1, 3, 7);
        lv_obj_set_style_radius(drop1, 1, 0);
        lv_obj_set_style_bg_color(drop1, lv_color_make(56, 189, 248), 0);
        lv_obj_set_style_bg_opa(drop1, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(drop1, 0, 0);
        lv_obj_align(drop1, LV_ALIGN_BOTTOM_LEFT, 6, -2);

        lv_obj_t* drop2 = lv_obj_create(parent);
        lv_obj_set_size(drop2, 3, 7);
        lv_obj_set_style_radius(drop2, 1, 0);
        lv_obj_set_style_bg_color(drop2, lv_color_make(56, 189, 248), 0);
        lv_obj_set_style_bg_opa(drop2, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(drop2, 0, 0);
        lv_obj_align(drop2, LV_ALIGN_BOTTOM_RIGHT, -6, -2);
    } else {
        // General / Mist / Fog / Default
        lv_obj_t* glyph = lv_obj_create(parent);
        lv_obj_set_size(glyph, 20, 20);
        lv_obj_set_style_radius(glyph, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(glyph, lv_color_make(56, 189, 248), 0);
        lv_obj_set_style_bg_opa(glyph, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(glyph, 0, 0);
        lv_obj_align(glyph, LV_ALIGN_CENTER, 0, 0);
    }
}

void UIManager::buildDashboard() {
    _scr = lv_scr_act();
    lv_obj_clear_flag(_scr, LV_OBJ_FLAG_SCROLLABLE);

    // 1. Hero Clock Card (Y: 8..112, H: 104, W: 304, Radius: 16)
    _cardClock = lv_obj_create(_scr);
    lv_obj_set_pos(_cardClock, 8, 8);
    lv_obj_set_size(_cardClock, 304, 104);
    lv_obj_clear_flag(_cardClock, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(_cardClock, 16, 0);
    lv_obj_set_style_border_width(_cardClock, 1, 0);
    lv_obj_set_style_pad_all(_cardClock, 6, 0);

    // Time Digits (Montserrat 48)
    _lblTime = lv_label_create(_cardClock);
    lv_label_set_text(_lblTime, "--:--:--");
    lv_obj_set_style_text_font(_lblTime, &lv_font_montserrat_48, 0);
    lv_obj_align(_lblTime, LV_ALIGN_TOP_MID, -22, 6);

    // AM/PM Pill Chip
    _badgeAmPm = lv_obj_create(_cardClock);
    lv_obj_set_size(_badgeAmPm, 38, 22);
    lv_obj_set_style_radius(_badgeAmPm, 6, 0);
    lv_obj_set_style_border_width(_badgeAmPm, 1, 0);
    lv_obj_clear_flag(_badgeAmPm, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align_to(_badgeAmPm, _lblTime, LV_ALIGN_OUT_RIGHT_MID, 6, -6);

    _lblAmPm = lv_label_create(_badgeAmPm);
    lv_label_set_text(_lblAmPm, "PM");
    lv_obj_set_style_text_font(_lblAmPm, &lv_font_montserrat_12, 0);
    lv_obj_center(_lblAmPm);

    // Full Date String
    _lblDate = lv_label_create(_cardClock);
    lv_label_set_text(_lblDate, "Awaiting NTP Synchronization...");
    lv_obj_set_style_text_font(_lblDate, &lv_font_montserrat_14, 0);
    lv_obj_align(_lblDate, LV_ALIGN_BOTTOM_MID, 0, -6);

    // 2. Current Weather Card (Y: 120..232, H: 112, W: 148, Radius: 16)
    _cardWeather = lv_obj_create(_scr);
    lv_obj_set_pos(_cardWeather, 8, 120);
    lv_obj_set_size(_cardWeather, 148, 112);
    lv_obj_clear_flag(_cardWeather, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(_cardWeather, 16, 0);
    lv_obj_set_style_border_width(_cardWeather, 1, 0);
    lv_obj_set_style_pad_all(_cardWeather, 6, 0);

    // Weather Icon Container
    _iconWeather = lv_obj_create(_cardWeather);
    lv_obj_set_pos(_iconWeather, 4, 4);
    lv_obj_set_size(_iconWeather, 28, 28);
    lv_obj_clear_flag(_iconWeather, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(_iconWeather, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(_iconWeather, 0, 0);

    // Current Temp (Montserrat 24)
    _lblWeatherTemp = lv_label_create(_cardWeather);
    lv_label_set_text(_lblWeatherTemp, "--°F");
    lv_obj_set_style_text_font(_lblWeatherTemp, &lv_font_montserrat_24, 0);
    lv_obj_set_pos(_lblWeatherTemp, 38, 4);

    // Condition Text
    _lblWeatherDesc = lv_label_create(_cardWeather);
    lv_label_set_text(_lblWeatherDesc, "Loading...");
    lv_obj_set_style_text_font(_lblWeatherDesc, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblWeatherDesc, 4, 38);

    // Daily High / Low Temperatures (ASCII format)
    _lblWeatherHighLow = lv_label_create(_cardWeather);
    lv_label_set_text(_lblWeatherHighLow, "H: --° / L: --°");
    lv_obj_set_style_text_font(_lblWeatherHighLow, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblWeatherHighLow, 4, 60);

    // Feels Like & Humidity (ASCII format)
    _lblWeatherMetrics = lv_label_create(_cardWeather);
    lv_label_set_text(_lblWeatherMetrics, "Feels --° | --% Hum");
    lv_obj_set_style_text_font(_lblWeatherMetrics, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblWeatherMetrics, 4, 82);

    // 3. 2-Day Forecast Card (Y: 120..232, H: 112, W: 148, Radius: 16)
    _cardForecast = lv_obj_create(_scr);
    lv_obj_set_pos(_cardForecast, 164, 120);
    lv_obj_set_size(_cardForecast, 148, 112);
    lv_obj_clear_flag(_cardForecast, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(_cardForecast, 16, 0);
    lv_obj_set_style_border_width(_cardForecast, 1, 0);
    lv_obj_set_style_pad_all(_cardForecast, 6, 0);

    _lblForecastTitle = lv_label_create(_cardForecast);
    lv_label_set_text(_lblForecastTitle, "2-DAY FORECAST");
    lv_obj_set_style_text_font(_lblForecastTitle, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblForecastTitle, 4, 4);

    // Forecast Day 1 (Tomorrow)
    _lblDay1Name = lv_label_create(_cardForecast);
    lv_label_set_text(_lblDay1Name, "Tomorrow");
    lv_obj_set_style_text_font(_lblDay1Name, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblDay1Name, 4, 34);

    _iconDay1 = lv_obj_create(_cardForecast);
    lv_obj_set_pos(_iconDay1, 38, 28);
    lv_obj_set_size(_iconDay1, 24, 24);
    lv_obj_clear_flag(_iconDay1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(_iconDay1, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(_iconDay1, 0, 0);

    _lblDay1HighLow = lv_label_create(_cardForecast);
    lv_label_set_text(_lblDay1HighLow, "--° / --°");
    lv_obj_set_style_text_font(_lblDay1HighLow, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblDay1HighLow, 68, 34);

    // Forecast Day 2 (Day after)
    _lblDay2Name = lv_label_create(_cardForecast);
    lv_label_set_text(_lblDay2Name, "Day After");
    lv_obj_set_style_text_font(_lblDay2Name, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblDay2Name, 4, 74);

    _iconDay2 = lv_obj_create(_cardForecast);
    lv_obj_set_pos(_iconDay2, 38, 68);
    lv_obj_set_size(_iconDay2, 24, 24);
    lv_obj_clear_flag(_iconDay2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(_iconDay2, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(_iconDay2, 0, 0);

    _lblDay2HighLow = lv_label_create(_cardForecast);
    lv_label_set_text(_lblDay2HighLow, "--° / --°");
    lv_obj_set_style_text_font(_lblDay2HighLow, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblDay2HighLow, 68, 74);

    // 4. Calendar & Agenda Card
    _cardCalendar = lv_obj_create(_scr);
    lv_obj_set_pos(_cardCalendar, 8, 120);
    lv_obj_set_size(_cardCalendar, 148, 112);
    lv_obj_clear_flag(_cardCalendar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(_cardCalendar, 16, 0);
    lv_obj_set_style_border_width(_cardCalendar, 1, 0);
    lv_obj_set_style_pad_all(_cardCalendar, 6, 0);

    _lblCalendarTitle = lv_label_create(_cardCalendar);
    lv_label_set_text(_lblCalendarTitle, "Upcoming Agenda");
    lv_obj_set_style_text_font(_lblCalendarTitle, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblCalendarTitle, 4, 4);

    _lblEvent1Title = lv_label_create(_cardCalendar);
    lv_label_set_text(_lblEvent1Title, "Team Standup");
    lv_obj_set_style_text_font(_lblEvent1Title, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblEvent1Title, 4, 26);

    _lblEvent1Time = lv_label_create(_cardCalendar);
    lv_label_set_text(_lblEvent1Time, "in 25 mins - Zoom");
    lv_obj_set_style_text_font(_lblEvent1Time, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblEvent1Time, 4, 46);

    _lblEvent2Title = lv_label_create(_cardCalendar);
    lv_label_set_text(_lblEvent2Title, "Design Sync");
    lv_obj_set_style_text_font(_lblEvent2Title, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblEvent2Title, 4, 74);

    // 5. Notifications Card
    _cardNotifications = lv_obj_create(_scr);
    lv_obj_set_pos(_cardNotifications, 164, 120);
    lv_obj_set_size(_cardNotifications, 148, 112);
    lv_obj_clear_flag(_cardNotifications, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(_cardNotifications, 16, 0);
    lv_obj_set_style_border_width(_cardNotifications, 1, 0);
    lv_obj_set_style_pad_all(_cardNotifications, 6, 0);

    _lblNotifTitle = lv_label_create(_cardNotifications);
    lv_label_set_text(_lblNotifTitle, "Alerts");
    lv_obj_set_style_text_font(_lblNotifTitle, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblNotifTitle, 4, 4);

    _lblEmailAlert = lv_label_create(_cardNotifications);
    lv_label_set_text(_lblEmailAlert, "3 Unread Emails");
    lv_obj_set_style_text_font(_lblEmailAlert, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblEmailAlert, 4, 32);

    _lblMsgAlert = lv_label_create(_cardNotifications);
    lv_label_set_text(_lblMsgAlert, "Slack: 2 Mentions");
    lv_obj_set_style_text_font(_lblMsgAlert, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblMsgAlert, 4, 64);

    // 6. System Diagnostics Card
    _cardSystem = lv_obj_create(_scr);
    lv_obj_set_pos(_cardSystem, 164, 120);
    lv_obj_set_size(_cardSystem, 148, 112);
    lv_obj_clear_flag(_cardSystem, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(_cardSystem, 16, 0);
    lv_obj_set_style_border_width(_cardSystem, 1, 0);
    lv_obj_set_style_pad_all(_cardSystem, 6, 0);

    _lblSysTitle = lv_label_create(_cardSystem);
    lv_label_set_text(_lblSysTitle, "Telemetry");
    lv_obj_set_style_text_font(_lblSysTitle, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblSysTitle, 4, 4);

    _lblSysWifi = lv_label_create(_cardSystem);
    lv_label_set_text(_lblSysWifi, "WiFi: Connected");
    lv_obj_set_style_text_font(_lblSysWifi, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblSysWifi, 4, 28);

    _lblSysHeap = lv_label_create(_cardSystem);
    lv_label_set_text(_lblSysHeap, "RAM: 220 KB free");
    lv_obj_set_style_text_font(_lblSysHeap, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblSysHeap, 4, 52);

    _lblSysUptime = lv_label_create(_cardSystem);
    lv_label_set_text(_lblSysUptime, "Up: Active");
    lv_obj_set_style_text_font(_lblSysUptime, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblSysUptime, 4, 76);

    // Link card widgets to modular CardRegistry
    ClockCard* clockCard = (ClockCard*)CardRegistry::getInstance().getCard("clock");
    if (clockCard) clockCard->setContainer(_cardClock);

    WeatherCard* weatherCard = (WeatherCard*)CardRegistry::getInstance().getCard("weather");
    if (weatherCard) weatherCard->setContainer(_cardWeather);

    ForecastCard* forecastCard = (ForecastCard*)CardRegistry::getInstance().getCard("forecast");
    if (forecastCard) forecastCard->setContainer(_cardForecast);

    CalendarCard* calendarCard = (CalendarCard*)CardRegistry::getInstance().getCard("calendar");
    if (calendarCard) calendarCard->setContainer(_cardCalendar);

    NotificationCard* notifCard = (NotificationCard*)CardRegistry::getInstance().getCard("notifications");
    if (notifCard) notifCard->setContainer(_cardNotifications);

    SystemCard* sysCard = (SystemCard*)CardRegistry::getInstance().getCard("system");
    if (sysCard) sysCard->setContainer(_cardSystem);

    refreshCards();
}

static lv_color_t hexToLvColor(const char* hex) {
    if (!hex || hex[0] == '\0') return lv_color_white();
    if (hex[0] == '#') hex++;
    long rgb = strtol(hex, nullptr, 16);
    return lv_color_hex((uint32_t)rgb);
}

void UIManager::applyThemeStyles(DisplayMode mode) {
    _currentMode = mode;

    lv_color_t colorBg, colorCard, colorBorder, colorAccent, colorText, colorMuted, colorBadgeBg;

    if (mode == DisplayMode::NIGHT) {
        // Night Theme: Pure black, warm amber bedside glow
        colorBg        = lv_color_make(0, 0, 0);
        colorCard      = lv_color_make(18, 12, 10);
        colorBorder    = lv_color_make(45, 20, 16);
        colorAccent    = lv_color_make(255, 152, 0);
        colorText      = lv_color_make(226, 160, 100);
        colorMuted     = lv_color_make(140, 86, 56);
        colorBadgeBg   = lv_color_make(42, 20, 8);
    } else {
        const auto& themeCfg = ConfigManager::getInstance().getThemeConfig();
        colorBg        = hexToLvColor(themeCfg.bgColor.c_str());
        colorCard      = hexToLvColor(themeCfg.cardBgColor.c_str());
        colorBorder    = hexToLvColor(themeCfg.cardBorderColor.c_str());
        colorAccent    = hexToLvColor(themeCfg.accentColor.c_str());
        colorText      = hexToLvColor(themeCfg.dateColor.c_str());
        colorMuted     = hexToLvColor(themeCfg.mutedTextColor.c_str());
        colorBadgeBg   = hexToLvColor(themeCfg.cardBgColor.c_str());
    }

    // Apply Canvas Background
    lv_obj_set_style_bg_color(_scr, colorBg, 0);
    lv_obj_set_style_bg_opa(_scr, LV_OPA_COVER, 0);

    // Apply Cards
    lv_obj_t* cards[] = {_cardClock, _cardWeather, _cardForecast, _cardCalendar, _cardNotifications, _cardSystem};
    for (lv_obj_t* card : cards) {
        if (card) {
            lv_obj_set_style_bg_color(card, colorCard, 0);
            lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
            lv_obj_set_style_border_color(card, colorBorder, 0);
        }
    }

    // Apply Hero Clock text & AM/PM chip
    lv_obj_set_style_text_color(_lblTime, colorAccent, 0);
    lv_obj_set_style_text_color(_lblDate, colorText, 0);

    lv_obj_set_style_bg_color(_badgeAmPm, colorBadgeBg, 0);
    lv_obj_set_style_bg_opa(_badgeAmPm, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(_badgeAmPm, colorBorder, 0);
    lv_obj_set_style_text_color(_lblAmPm, colorAccent, 0);

    // Apply Current Weather Card elements
    lv_obj_set_style_text_color(_lblWeatherTemp, colorText, 0);
    lv_obj_set_style_text_color(_lblWeatherDesc, colorAccent, 0);
    lv_obj_set_style_text_color(_lblWeatherHighLow, colorText, 0);
    lv_obj_set_style_text_color(_lblWeatherMetrics, colorMuted, 0);

    // Apply 2-Day Forecast Card elements
    lv_obj_set_style_text_color(_lblForecastTitle, colorAccent, 0);
    lv_obj_set_style_text_color(_lblDay1Name, colorText, 0);
    lv_obj_set_style_text_color(_lblDay1HighLow, colorMuted, 0);
    lv_obj_set_style_text_color(_lblDay2Name, colorText, 0);
    lv_obj_set_style_text_color(_lblDay2HighLow, colorMuted, 0);

    // Apply Calendar Card elements
    if (_lblCalendarTitle) lv_obj_set_style_text_color(_lblCalendarTitle, colorAccent, 0);
    if (_lblEvent1Title)   lv_obj_set_style_text_color(_lblEvent1Title, colorText, 0);
    if (_lblEvent1Time)    lv_obj_set_style_text_color(_lblEvent1Time, colorMuted, 0);
    if (_lblEvent2Title)   lv_obj_set_style_text_color(_lblEvent2Title, colorText, 0);

    // Apply Notifications Card elements
    if (_lblNotifTitle) lv_obj_set_style_text_color(_lblNotifTitle, colorAccent, 0);
    if (_lblEmailAlert) lv_obj_set_style_text_color(_lblEmailAlert, colorText, 0);
    if (_lblMsgAlert)   lv_obj_set_style_text_color(_lblMsgAlert, colorMuted, 0);

    // Apply System Card elements
    if (_lblSysTitle)  lv_obj_set_style_text_color(_lblSysTitle, colorAccent, 0);
    if (_lblSysWifi)   lv_obj_set_style_text_color(_lblSysWifi, colorText, 0);
    if (_lblSysHeap)   lv_obj_set_style_text_color(_lblSysHeap, colorMuted, 0);
    if (_lblSysUptime) lv_obj_set_style_text_color(_lblSysUptime, colorMuted, 0);
}

void UIManager::setTheme(DisplayMode mode) {
    applyThemeStyles(mode);
}

void UIManager::reloadConfig() {
    refreshCards();
    applyThemeStyles(_currentMode);
}

void UIManager::refreshCards() {
    // 1. Clock card is always top
    ClockCard* clockCard = (ClockCard*)CardRegistry::getInstance().getCard("clock");
    if (clockCard && clockCard->getContainer()) {
        if (clockCard->isEnabled()) {
            lv_obj_clear_flag(clockCard->getContainer(), LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(clockCard->getContainer(), LV_OBJ_FLAG_HIDDEN);
        }
    }

    // 2. Hide all secondary cards initially
    const auto& allCards = CardRegistry::getInstance().getCards();
    for (Card* c : allCards) {
        if (strcmp(c->getId(), "clock") != 0 && c->getContainer()) {
            lv_obj_add_flag(c->getContainer(), LV_OBJ_FLAG_HIDDEN);
        }
    }

    // 3. Collect enabled secondary cards in order
    std::vector<Card*> enabledSecondary;
    for (Card* c : allCards) {
        if (strcmp(c->getId(), "clock") != 0 && c->isEnabled() && c->getContainer()) {
            enabledSecondary.push_back(c);
        }
    }

    // 4. Dynamically assign to bottom slots
    if (enabledSecondary.size() == 1) {
        // Single card takes full bottom width: (8, 120, 304, 112)
        lv_obj_t* cont = enabledSecondary[0]->getContainer();
        lv_obj_set_pos(cont, 8, 120);
        lv_obj_set_size(cont, 304, 112);
        lv_obj_clear_flag(cont, LV_OBJ_FLAG_HIDDEN);
    } else if (enabledSecondary.size() >= 2) {
        // Left Slot: (8, 120, 148, 112)
        lv_obj_t* leftCont = enabledSecondary[0]->getContainer();
        lv_obj_set_pos(leftCont, 8, 120);
        lv_obj_set_size(leftCont, 148, 112);
        lv_obj_clear_flag(leftCont, LV_OBJ_FLAG_HIDDEN);

        // Right Slot: (164, 120, 148, 112)
        lv_obj_t* rightCont = enabledSecondary[1]->getContainer();
        lv_obj_set_pos(rightCont, 164, 120);
        lv_obj_set_size(rightCont, 148, 112);
        lv_obj_clear_flag(rightCont, LV_OBJ_FLAG_HIDDEN);
    }

    // Redraw entire screen cleanly
    if (lv_scr_act()) {
        lv_obj_invalidate(lv_scr_act());
    }
}



void UIManager::updateTime(const char* timeStr, const char* ampmStr, const char* dateStr) {
    if (_lblTime && timeStr) {
        lv_label_set_text(_lblTime, timeStr);
    }
    if (_lblAmPm && ampmStr) {
        lv_label_set_text(_lblAmPm, ampmStr);
        lv_obj_align_to(_badgeAmPm, _lblTime, LV_ALIGN_OUT_RIGHT_MID, 6, -6);
    }
    if (_lblDate && dateStr) {
        lv_label_set_text(_lblDate, dateStr);
    }
}

void UIManager::updateWeather(const WeatherData& data, bool isFahrenheit, int todayWeekday) {
    if (!_cardWeather || !_cardForecast) return;

    static const char* const DAY_NAMES[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    if (data.isValid) {
        char tempBuf[16];
        char highLowBuf[32];
        char metricsBuf[32];
        char unit = isFahrenheit ? 'F' : 'C';

        snprintf(tempBuf, sizeof(tempBuf), "%.0f°%c", data.temperature, unit);
        snprintf(highLowBuf, sizeof(highLowBuf), "H: %.0f° / L: %.0f°", data.tempMax, data.tempMin);
        snprintf(metricsBuf, sizeof(metricsBuf), "Feels %.0f° | %d%% Hum", data.apparentTemperature, data.humidity);

        lv_label_set_text(_lblWeatherTemp, tempBuf);
        lv_label_set_text(_lblWeatherDesc, data.conditionText);
        lv_label_set_text(_lblWeatherHighLow, highLowBuf);
        lv_label_set_text(_lblWeatherMetrics, metricsBuf);

        renderWeatherIcon(_iconWeather, data.weatherCode, _currentMode == DisplayMode::NIGHT);

        // Update 2-Day Forecast
        int day1Idx = (todayWeekday + 1) % 7;
        int day2Idx = (todayWeekday + 2) % 7;
        lv_label_set_text(_lblDay1Name, DAY_NAMES[day1Idx]);
        lv_label_set_text(_lblDay2Name, DAY_NAMES[day2Idx]);

        if (data.forecast[0].isValid) {
            char f1Buf[24];
            snprintf(f1Buf, sizeof(f1Buf), "%.0f° / %.0f°", data.forecast[0].tempMax, data.forecast[0].tempMin);
            lv_label_set_text(_lblDay1HighLow, f1Buf);
            renderWeatherIcon(_iconDay1, data.forecast[0].weatherCode, false);
        }

        if (data.forecast[1].isValid) {
            char f2Buf[24];
            snprintf(f2Buf, sizeof(f2Buf), "%.0f° / %.0f°", data.forecast[1].tempMax, data.forecast[1].tempMin);
            lv_label_set_text(_lblDay2HighLow, f2Buf);
            renderWeatherIcon(_iconDay2, data.forecast[1].weatherCode, false);
        }
    } else {
        lv_label_set_text(_lblWeatherTemp, "--°");
        lv_label_set_text(_lblWeatherDesc, data.statusText);
        lv_label_set_text(_lblWeatherHighLow, "H: --° / L: --°");
        lv_label_set_text(_lblWeatherMetrics, "Open-Meteo REST");

        renderWeatherIcon(_iconWeather, 0, _currentMode == DisplayMode::NIGHT);
    }
}

void UIManager::updateCalendar(const char* title, const char* ev1Title, const char* ev1Time, const char* ev2Title) {
    if (_lblCalendarTitle && title) lv_label_set_text(_lblCalendarTitle, title);
    if (_lblEvent1Title && ev1Title) lv_label_set_text(_lblEvent1Title, ev1Title);
    if (_lblEvent1Time && ev1Time) lv_label_set_text(_lblEvent1Time, ev1Time);
    if (_lblEvent2Title && ev2Title) lv_label_set_text(_lblEvent2Title, ev2Title);
}

