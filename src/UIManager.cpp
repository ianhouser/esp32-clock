#include "UIManager.h"

static TFT_eSPI* s_tft = nullptr;

UIManager::UIManager()
    : _tft(nullptr),
      _currentMode(DisplayMode::DAY),
      _scr(nullptr),
      _headerBar(nullptr),
      _lblTitle(nullptr),
      _badgeMode(nullptr),
      _lblBadgeMode(nullptr),
      _badgeWifi(nullptr),
      _lblWifiStatus(nullptr),
      _cardClock(nullptr),
      _lblTime(nullptr),
      _badgeAmPm(nullptr),
      _lblAmPm(nullptr),
      _lblDate(nullptr),
      _bottomContainer(nullptr),
      _cardWeather(nullptr),
      _iconWeather(nullptr),
      _lblWeatherTemp(nullptr),
      _lblWeatherDesc(nullptr),
      _lblWeatherHighLow(nullptr),
      _lblWeatherMetrics(nullptr),
      _cardSystem(nullptr),
      _lblSystemTitle(nullptr),
      _lblSystemSsid(nullptr),
      _lblSystemIp(nullptr),
      _lblSystemSync(nullptr) {}

static bool s_captureActive = false;
static uint32_t s_capturePixelCount = 0;
static uint8_t s_b64LineLen = 0;

void UIManager::dispFlushCallback(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
    if (s_tft) {
        uint32_t w = (area->x2 - area->x1 + 1);
        uint32_t h = (area->y2 - area->y1 + 1);

        s_tft->pushImage(area->x1, area->y1, w, h, (uint16_t*)&color_p->full);

        if (s_captureActive) {
            static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            uint32_t totalPixels = w * h;
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
                s_b64LineLen += 4;
                if (s_b64LineLen >= 64) {
                    Serial.println();
                    s_b64LineLen = 0;
                }

                s_capturePixelCount++;
            }

            if (s_capturePixelCount >= 320 * 240) {
                if (s_b64LineLen > 0) Serial.println();
                Serial.println("===CAPTURE_PPM_B64_END===");
                s_captureActive = false;
            }
        }
    }
    lv_disp_flush_ready(disp);
}

void UIManager::requestCapture() {
    s_captureActive = true;
    s_capturePixelCount = 0;
    s_b64LineLen = 0;
    Serial.println("\n===CAPTURE_PPM_B64_START===");
    Serial.println("P6");
    Serial.printf("%d %d\n", 320, 240);
    Serial.println("255");
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

    // 1. Top Status Bar (Y: 0..26)
    _headerBar = lv_obj_create(_scr);
    lv_obj_set_pos(_headerBar, 0, 0);
    lv_obj_set_size(_headerBar, 320, 26);
    lv_obj_clear_flag(_headerBar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(_headerBar, 0, 0);
    lv_obj_set_style_border_side(_headerBar, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_pad_hor(_headerBar, 8, 0);
    lv_obj_set_style_pad_ver(_headerBar, 2, 0);

    // Day/Night Mode Chip
    _badgeMode = lv_obj_create(_headerBar);
    lv_obj_set_size(_badgeMode, 58, 20);
    lv_obj_set_style_radius(_badgeMode, 10, 0);
    lv_obj_align(_badgeMode, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_clear_flag(_badgeMode, LV_OBJ_FLAG_SCROLLABLE);

    _lblBadgeMode = lv_label_create(_badgeMode);
    lv_label_set_text(_lblBadgeMode, "DAY");
    lv_obj_set_style_text_font(_lblBadgeMode, &lv_font_montserrat_12, 0);
    lv_obj_center(_lblBadgeMode);

    // App Title
    _lblTitle = lv_label_create(_headerBar);
    lv_label_set_text(_lblTitle, "esp32-clock");
    lv_obj_set_style_text_font(_lblTitle, &lv_font_montserrat_14, 0);
    lv_obj_align(_lblTitle, LV_ALIGN_CENTER, 0, 0);

    // WiFi Status Chip
    _badgeWifi = lv_obj_create(_headerBar);
    lv_obj_set_size(_badgeWifi, 90, 20);
    lv_obj_set_style_radius(_badgeWifi, 10, 0);
    lv_obj_align(_badgeWifi, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_clear_flag(_badgeWifi, LV_OBJ_FLAG_SCROLLABLE);

    _lblWifiStatus = lv_label_create(_badgeWifi);
    lv_label_set_text(_lblWifiStatus, "Offline");
    lv_obj_set_style_text_font(_lblWifiStatus, &lv_font_montserrat_12, 0);
    lv_obj_center(_lblWifiStatus);

    // 2. Hero Clock Card (Y: 28..126, H: 98, W: 304, Radius: 14)
    _cardClock = lv_obj_create(_scr);
    lv_obj_set_pos(_cardClock, 8, 28);
    lv_obj_set_size(_cardClock, 304, 98);
    lv_obj_clear_flag(_cardClock, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(_cardClock, 14, 0);
    lv_obj_set_style_border_width(_cardClock, 1, 0);
    lv_obj_set_style_pad_all(_cardClock, 6, 0);

    // Time Digits (Montserrat 48)
    _lblTime = lv_label_create(_cardClock);
    lv_label_set_text(_lblTime, "--:--:--");
    lv_obj_set_style_text_font(_lblTime, &lv_font_montserrat_48, 0);
    lv_obj_align(_lblTime, LV_ALIGN_TOP_MID, -20, 2);

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
    lv_obj_align(_lblDate, LV_ALIGN_BOTTOM_MID, 0, -4);

    // 3. Bottom Row Cards (Y: 132..232, H: 100)
    // Left: Weather Card (W: 149, Radius: 14)
    _cardWeather = lv_obj_create(_scr);
    lv_obj_set_pos(_cardWeather, 8, 132);
    lv_obj_set_size(_cardWeather, 149, 100);
    lv_obj_clear_flag(_cardWeather, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(_cardWeather, 14, 0);
    lv_obj_set_style_border_width(_cardWeather, 1, 0);
    lv_obj_set_style_pad_all(_cardWeather, 6, 0);

    // Weather Icon Container
    _iconWeather = lv_obj_create(_cardWeather);
    lv_obj_set_pos(_iconWeather, 2, 2);
    lv_obj_set_size(_iconWeather, 28, 28);
    lv_obj_clear_flag(_iconWeather, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(_iconWeather, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(_iconWeather, 0, 0);

    // Current Temp (Montserrat 24)
    _lblWeatherTemp = lv_label_create(_cardWeather);
    lv_label_set_text(_lblWeatherTemp, "--°F");
    lv_obj_set_style_text_font(_lblWeatherTemp, &lv_font_montserrat_24, 0);
    lv_obj_set_pos(_lblWeatherTemp, 36, 4);

    // Condition Text
    _lblWeatherDesc = lv_label_create(_cardWeather);
    lv_label_set_text(_lblWeatherDesc, "Loading...");
    lv_obj_set_style_text_font(_lblWeatherDesc, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblWeatherDesc, 4, 34);

    // Daily High / Low Temperatures
    _lblWeatherHighLow = lv_label_create(_cardWeather);
    lv_label_set_text(_lblWeatherHighLow, "H: --°  L: --°");
    lv_obj_set_style_text_font(_lblWeatherHighLow, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblWeatherHighLow, 4, 52);

    // Feels Like & Humidity
    _lblWeatherMetrics = lv_label_create(_cardWeather);
    lv_label_set_text(_lblWeatherMetrics, "Feels --° · --% Hum");
    lv_obj_set_style_text_font(_lblWeatherMetrics, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblWeatherMetrics, 4, 70);

    // Right: System & Network Card (W: 149, Radius: 14)
    _cardSystem = lv_obj_create(_scr);
    lv_obj_set_pos(_cardSystem, 163, 132);
    lv_obj_set_size(_cardSystem, 149, 100);
    lv_obj_clear_flag(_cardSystem, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(_cardSystem, 14, 0);
    lv_obj_set_style_border_width(_cardSystem, 1, 0);
    lv_obj_set_style_pad_all(_cardSystem, 6, 0);

    _lblSystemTitle = lv_label_create(_cardSystem);
    lv_label_set_text(_lblSystemTitle, "SYSTEM & NET");
    lv_obj_set_style_text_font(_lblSystemTitle, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblSystemTitle, 4, 4);

    _lblSystemSsid = lv_label_create(_cardSystem);
    lv_label_set_text(_lblSystemSsid, "WiFi: --");
    lv_obj_set_style_text_font(_lblSystemSsid, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblSystemSsid, 4, 28);

    _lblSystemIp = lv_label_create(_cardSystem);
    lv_label_set_text(_lblSystemIp, "IP: Disconnected");
    lv_obj_set_style_text_font(_lblSystemIp, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblSystemIp, 4, 48);

    _lblSystemSync = lv_label_create(_cardSystem);
    lv_label_set_text(_lblSystemSync, "Sync: Idle");
    lv_obj_set_style_text_font(_lblSystemSync, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblSystemSync, 4, 68);
}

void UIManager::applyThemeStyles(DisplayMode mode) {
    _currentMode = mode;

    lv_color_t colorBg, colorHeader, colorCard, colorBorder, colorAccent, colorText, colorMuted, colorBadgeBg, colorBadgeText;

    if (mode == DisplayMode::NIGHT) {
        // Night Theme: Pure black, warm amber bedside glow
        colorBg        = lv_color_make(0, 0, 0);
        colorHeader    = lv_color_make(0, 0, 0);
        colorCard      = lv_color_make(18, 12, 10);
        colorBorder    = lv_color_make(45, 20, 16);
        colorAccent    = lv_color_make(255, 152, 0);
        colorText      = lv_color_make(226, 160, 100);
        colorMuted     = lv_color_make(140, 86, 56);
        colorBadgeBg   = lv_color_make(42, 20, 8);
        colorBadgeText = lv_color_make(255, 152, 0);
        lv_label_set_text(_lblBadgeMode, "NIGHT");
    } else {
        // Day Theme (Material 3 Dark Modern Obsidian/Cyan)
        colorBg        = lv_color_make(15, 19, 24);
        colorHeader    = lv_color_make(15, 19, 24);
        colorCard      = lv_color_make(24, 28, 36);
        colorBorder    = lv_color_make(40, 48, 61);
        colorAccent    = lv_color_make(56, 189, 248);
        colorText      = lv_color_make(241, 245, 249);
        colorMuted     = lv_color_make(148, 163, 184);
        colorBadgeBg   = lv_color_make(30, 41, 59);
        colorBadgeText = lv_color_make(56, 189, 248);
        lv_label_set_text(_lblBadgeMode, "DAY");
    }

    // Apply Background
    lv_obj_set_style_bg_color(_scr, colorBg, 0);
    lv_obj_set_style_bg_opa(_scr, LV_OPA_COVER, 0);

    // Apply Header
    lv_obj_set_style_bg_color(_headerBar, colorHeader, 0);
    lv_obj_set_style_bg_opa(_headerBar, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(_lblTitle, colorMuted, 0);

    // Apply Badges
    lv_obj_set_style_bg_color(_badgeMode, colorBadgeBg, 0);
    lv_obj_set_style_bg_opa(_badgeMode, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(_badgeMode, colorBorder, 0);
    lv_obj_set_style_text_color(_lblBadgeMode, colorBadgeText, 0);

    lv_obj_set_style_bg_color(_badgeWifi, colorBadgeBg, 0);
    lv_obj_set_style_bg_opa(_badgeWifi, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(_badgeWifi, colorBorder, 0);
    lv_obj_set_style_text_color(_lblWifiStatus, colorBadgeText, 0);

    // Apply Cards
    lv_obj_t* cards[] = {_cardClock, _cardWeather, _cardSystem};
    for (lv_obj_t* card : cards) {
        lv_obj_set_style_bg_color(card, colorCard, 0);
        lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(card, colorBorder, 0);
    }

    // Apply Clock text & AM/PM chip
    lv_obj_set_style_text_color(_lblTime, colorAccent, 0);
    lv_obj_set_style_text_color(_lblDate, colorText, 0);

    lv_obj_set_style_bg_color(_badgeAmPm, colorBadgeBg, 0);
    lv_obj_set_style_bg_opa(_badgeAmPm, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(_badgeAmPm, colorBorder, 0);
    lv_obj_set_style_text_color(_lblAmPm, colorAccent, 0);

    // Apply Weather Card elements
    lv_obj_set_style_text_color(_lblWeatherTemp, colorText, 0);
    lv_obj_set_style_text_color(_lblWeatherDesc, colorAccent, 0);
    lv_obj_set_style_text_color(_lblWeatherHighLow, colorText, 0);
    lv_obj_set_style_text_color(_lblWeatherMetrics, colorMuted, 0);

    // Apply System Card elements
    lv_obj_set_style_text_color(_lblSystemTitle, colorAccent, 0);
    lv_obj_set_style_text_color(_lblSystemSsid, colorMuted, 0);
    lv_obj_set_style_text_color(_lblSystemIp, colorMuted, 0);
    lv_obj_set_style_text_color(_lblSystemSync, colorMuted, 0);
}

void UIManager::setTheme(DisplayMode mode) {
    applyThemeStyles(mode);
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

void UIManager::updateWeather(const WeatherData& data, bool isFahrenheit, bool isWiFiConnected) {
    if (!_cardWeather) return;

    if (data.isValid) {
        char tempBuf[16];
        char highLowBuf[32];
        char metricsBuf[32];
        char unit = isFahrenheit ? 'F' : 'C';

        snprintf(tempBuf, sizeof(tempBuf), "%.0f°%c", data.temperature, unit);
        snprintf(highLowBuf, sizeof(highLowBuf), "H: %.0f° · L: %.0f°", data.tempMax, data.tempMin);
        snprintf(metricsBuf, sizeof(metricsBuf), "Feels %.0f° · %d%% Hum", data.apparentTemperature, data.humidity);

        lv_label_set_text(_lblWeatherTemp, tempBuf);
        lv_label_set_text(_lblWeatherDesc, data.conditionText);
        lv_label_set_text(_lblWeatherHighLow, highLowBuf);
        lv_label_set_text(_lblWeatherMetrics, metricsBuf);

        renderWeatherIcon(_iconWeather, data.weatherCode, _currentMode == DisplayMode::NIGHT);
    } else {
        const char* status = isWiFiConnected ? data.statusText : "Awaiting WiFi";
        lv_label_set_text(_lblWeatherTemp, "--°");
        lv_label_set_text(_lblWeatherDesc, status);
        lv_label_set_text(_lblWeatherHighLow, "H: --° · L: --°");
        lv_label_set_text(_lblWeatherMetrics, "Open-Meteo REST");

        renderWeatherIcon(_iconWeather, 0, _currentMode == DisplayMode::NIGHT);
    }
}

void UIManager::updateNetwork(const char* ssid, const char* ip, int8_t rssi, bool connected, const char* syncState) {
    if (!_headerBar) return;

    char wifiStatusBuf[32];
    if (connected) {
        snprintf(wifiStatusBuf, sizeof(wifiStatusBuf), "WiFi %d dBm", rssi);
    } else {
        snprintf(wifiStatusBuf, sizeof(wifiStatusBuf), "%s", "Offline");
    }
    lv_label_set_text(_lblWifiStatus, wifiStatusBuf);

    char ssidBuf[32];
    snprintf(ssidBuf, sizeof(ssidBuf), "WiFi: %.10s", ssid ? ssid : "--");
    lv_label_set_text(_lblSystemSsid, ssidBuf);

    char ipBuf[32];
    snprintf(ipBuf, sizeof(ipBuf), "IP: %s", (connected && ip) ? ip : "Disconnected");
    lv_label_set_text(_lblSystemIp, ipBuf);

    char syncBuf[32];
    snprintf(syncBuf, sizeof(syncBuf), "Sync: %s", syncState ? syncState : "Idle");
    lv_label_set_text(_lblSystemSync, syncBuf);
}
