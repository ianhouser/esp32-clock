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
      _lblWifiStatus(nullptr),
      _cardClock(nullptr),
      _lblTime(nullptr),
      _lblDate(nullptr),
      _bottomContainer(nullptr),
      _cardWeather(nullptr),
      _lblWeatherTitle(nullptr),
      _lblWeatherTemp(nullptr),
      _lblWeatherFeels(nullptr),
      _lblWeatherHum(nullptr),
      _cardSystem(nullptr),
      _lblSystemTitle(nullptr),
      _lblSystemSsid(nullptr),
      _lblSystemIp(nullptr),
      _lblSystemSync(nullptr) {}

void UIManager::dispFlushCallback(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
    if (s_tft) {
        uint32_t w = (area->x2 - area->x1 + 1);
        uint32_t h = (area->y2 - area->y1 + 1);

        s_tft->startWrite();
        s_tft->setAddrWindow(area->x1, area->y1, w, h);
        s_tft->pushColors((uint16_t*)&color_p->full, w * h, true);
        s_tft->endWrite();
    }
    lv_disp_flush_ready(disp);
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
    lv_timer_handler();
}

void UIManager::buildDashboard() {
    _scr = lv_scr_act();
    lv_obj_clear_flag(_scr, LV_OBJ_FLAG_SCROLLABLE);

    // Initialize styles
    lv_style_init(&_styleScr);
    lv_style_init(&_styleHeader);
    lv_style_init(&_styleCard);
    lv_style_init(&_styleBadge);
    lv_style_init(&_styleTime);
    lv_style_init(&_styleDate);
    lv_style_init(&_styleCardTitle);
    lv_style_init(&_styleMutedText);

    // Common card properties
    lv_style_set_radius(&_styleCard, 6);
    lv_style_set_border_width(&_styleCard, 1);
    lv_style_set_pad_all(&_styleCard, 6);

    // Common badge properties
    lv_style_set_radius(&_styleBadge, 4);
    lv_style_set_pad_all(&_styleBadge, 2);

    // 1. Header Bar (Y: 0..28)
    _headerBar = lv_obj_create(_scr);
    lv_obj_set_pos(_headerBar, 0, 0);
    lv_obj_set_size(_headerBar, 320, 28);
    lv_obj_clear_flag(_headerBar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(_headerBar, 0, 0);
    lv_obj_set_style_border_side(_headerBar, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(_headerBar, 1, 0);
    lv_obj_set_style_pad_hor(_headerBar, 10, 0);
    lv_obj_set_style_pad_ver(_headerBar, 4, 0);

    _lblTitle = lv_label_create(_headerBar);
    lv_label_set_text(_lblTitle, "esp32-clock");
    lv_obj_set_style_text_font(_lblTitle, &lv_font_montserrat_14, 0);
    lv_obj_align(_lblTitle, LV_ALIGN_LEFT_MID, 0, 0);

    _badgeMode = lv_obj_create(_headerBar);
    lv_obj_set_size(_badgeMode, 54, 18);
    lv_obj_align(_badgeMode, LV_ALIGN_LEFT_MID, 95, 0);
    lv_obj_clear_flag(_badgeMode, LV_OBJ_FLAG_SCROLLABLE);

    _lblBadgeMode = lv_label_create(_badgeMode);
    lv_label_set_text(_lblBadgeMode, "DAY");
    lv_obj_set_style_text_font(_lblBadgeMode, &lv_font_montserrat_12, 0);
    lv_obj_center(_lblBadgeMode);

    _lblWifiStatus = lv_label_create(_headerBar);
    lv_label_set_text(_lblWifiStatus, "Offline");
    lv_obj_set_style_text_font(_lblWifiStatus, &lv_font_montserrat_12, 0);
    lv_obj_align(_lblWifiStatus, LV_ALIGN_RIGHT_MID, 0, 0);

    // 2. Main Clock Card (Y: 34..138)
    _cardClock = lv_obj_create(_scr);
    lv_obj_set_pos(_cardClock, 8, 34);
    lv_obj_set_size(_cardClock, 304, 104);
    lv_obj_clear_flag(_cardClock, LV_OBJ_FLAG_SCROLLABLE);

    _lblTime = lv_label_create(_cardClock);
    lv_label_set_text(_lblTime, "--:--:--");
    lv_obj_set_style_text_font(_lblTime, &lv_font_montserrat_40, 0);
    lv_obj_align(_lblTime, LV_ALIGN_TOP_MID, 0, 6);

    _lblDate = lv_label_create(_cardClock);
    lv_label_set_text(_lblDate, "Awaiting NTP Synchronization...");
    lv_obj_set_style_text_font(_lblDate, &lv_font_montserrat_14, 0);
    lv_obj_align(_lblDate, LV_ALIGN_BOTTOM_MID, 0, -6);

    // 3. Bottom Row Cards (Y: 144..232)
    // Left: Weather Card
    _cardWeather = lv_obj_create(_scr);
    lv_obj_set_pos(_cardWeather, 8, 144);
    lv_obj_set_size(_cardWeather, 149, 88);
    lv_obj_clear_flag(_cardWeather, LV_OBJ_FLAG_SCROLLABLE);

    _lblWeatherTitle = lv_label_create(_cardWeather);
    lv_label_set_text(_lblWeatherTitle, "WEATHER");
    lv_obj_set_style_text_font(_lblWeatherTitle, &lv_font_montserrat_12, 0);
    lv_obj_align(_lblWeatherTitle, LV_ALIGN_TOP_LEFT, 2, 0);

    _lblWeatherTemp = lv_label_create(_cardWeather);
    lv_label_set_text(_lblWeatherTemp, "Awaiting data");
    lv_obj_set_style_text_font(_lblWeatherTemp, &lv_font_montserrat_14, 0);
    lv_obj_align(_lblWeatherTemp, LV_ALIGN_TOP_LEFT, 2, 20);

    _lblWeatherFeels = lv_label_create(_cardWeather);
    lv_label_set_text(_lblWeatherFeels, "Feels: --");
    lv_obj_set_style_text_font(_lblWeatherFeels, &lv_font_montserrat_12, 0);
    lv_obj_align(_lblWeatherFeels, LV_ALIGN_TOP_LEFT, 2, 38);

    _lblWeatherHum = lv_label_create(_cardWeather);
    lv_label_set_text(_lblWeatherHum, "Humidity: --");
    lv_obj_set_style_text_font(_lblWeatherHum, &lv_font_montserrat_12, 0);
    lv_obj_align(_lblWeatherHum, LV_ALIGN_TOP_LEFT, 2, 54);

    // Right: System & Network Card
    _cardSystem = lv_obj_create(_scr);
    lv_obj_set_pos(_cardSystem, 163, 144);
    lv_obj_set_size(_cardSystem, 149, 88);
    lv_obj_clear_flag(_cardSystem, LV_OBJ_FLAG_SCROLLABLE);

    _lblSystemTitle = lv_label_create(_cardSystem);
    lv_label_set_text(_lblSystemTitle, "SYSTEM & NET");
    lv_obj_set_style_text_font(_lblSystemTitle, &lv_font_montserrat_12, 0);
    lv_obj_align(_lblSystemTitle, LV_ALIGN_TOP_LEFT, 2, 0);

    _lblSystemSsid = lv_label_create(_cardSystem);
    lv_label_set_text(_lblSystemSsid, "WiFi: --");
    lv_obj_set_style_text_font(_lblSystemSsid, &lv_font_montserrat_12, 0);
    lv_obj_align(_lblSystemSsid, LV_ALIGN_TOP_LEFT, 2, 20);

    _lblSystemIp = lv_label_create(_cardSystem);
    lv_label_set_text(_lblSystemIp, "IP: Disconnected");
    lv_obj_set_style_text_font(_lblSystemIp, &lv_font_montserrat_12, 0);
    lv_obj_align(_lblSystemIp, LV_ALIGN_TOP_LEFT, 2, 38);

    _lblSystemSync = lv_label_create(_cardSystem);
    lv_label_set_text(_lblSystemSync, "Sync: Idle");
    lv_obj_set_style_text_font(_lblSystemSync, &lv_font_montserrat_12, 0);
    lv_obj_align(_lblSystemSync, LV_ALIGN_TOP_LEFT, 2, 54);
}

void UIManager::applyThemeStyles(DisplayMode mode) {
    _currentMode = mode;

    lv_color_t colorBg, colorHeader, colorCard, colorBorder, colorAccent, colorText, colorMuted, colorBadgeBg, colorBadgeText;

    if (mode == DisplayMode::NIGHT) {
        // Night Theme: Pure black, deep warm amber/red glow
        colorBg        = lv_color_make(0, 0, 0);
        colorHeader    = lv_color_make(0, 0, 0);
        colorCard      = lv_color_make(12, 6, 6);
        colorBorder    = lv_color_make(45, 16, 16);
        colorAccent    = lv_color_make(255, 144, 0);
        colorText      = lv_color_make(220, 120, 40);
        colorMuted     = lv_color_make(130, 80, 50);
        colorBadgeBg   = lv_color_make(50, 20, 0);
        colorBadgeText = lv_color_make(255, 144, 0);
        lv_label_set_text(_lblBadgeMode, "NIGHT");
    } else {
        // Day Theme: Deep slate navy, crisp neon cyan
        colorBg        = lv_color_make(8, 16, 32);
        colorHeader    = lv_color_make(10, 24, 44);
        colorCard      = lv_color_make(16, 32, 52);
        colorBorder    = lv_color_make(36, 64, 96);
        colorAccent    = lv_color_make(0, 240, 255);
        colorText      = lv_color_make(255, 255, 255);
        colorMuted     = lv_color_make(140, 168, 196);
        colorBadgeBg   = lv_color_make(4, 56, 72);
        colorBadgeText = lv_color_make(0, 240, 255);
        lv_label_set_text(_lblBadgeMode, "DAY");
    }

    // Apply Background
    lv_obj_set_style_bg_color(_scr, colorBg, 0);

    // Apply Header
    lv_obj_set_style_bg_color(_headerBar, colorHeader, 0);
    lv_obj_set_style_border_color(_headerBar, colorBorder, 0);
    lv_obj_set_style_text_color(_lblTitle, colorText, 0);
    lv_obj_set_style_text_color(_lblWifiStatus, colorMuted, 0);

    // Apply Badge
    lv_obj_set_style_bg_color(_badgeMode, colorBadgeBg, 0);
    lv_obj_set_style_border_color(_badgeMode, colorBorder, 0);
    lv_obj_set_style_text_color(_lblBadgeMode, colorBadgeText, 0);

    // Apply Cards
    lv_obj_t* cards[] = {_cardClock, _cardWeather, _cardSystem};
    for (lv_obj_t* card : cards) {
        lv_obj_set_style_bg_color(card, colorCard, 0);
        lv_obj_set_style_border_color(card, colorBorder, 0);
    }

    // Apply Clock text
    lv_obj_set_style_text_color(_lblTime, colorAccent, 0);
    lv_obj_set_style_text_color(_lblDate, colorText, 0);

    // Apply Card titles
    lv_obj_set_style_text_color(_lblWeatherTitle, colorAccent, 0);
    lv_obj_set_style_text_color(_lblSystemTitle, colorAccent, 0);

    // Apply Card values
    lv_obj_set_style_text_color(_lblWeatherTemp, colorText, 0);
    lv_obj_set_style_text_color(_lblWeatherFeels, colorMuted, 0);
    lv_obj_set_style_text_color(_lblWeatherHum, colorMuted, 0);

    lv_obj_set_style_text_color(_lblSystemSsid, colorMuted, 0);
    lv_obj_set_style_text_color(_lblSystemIp, colorMuted, 0);
    lv_obj_set_style_text_color(_lblSystemSync, colorMuted, 0);
}

void UIManager::setTheme(DisplayMode mode) {
    if (mode != _currentMode) {
        applyThemeStyles(mode);
    }
}

void UIManager::updateTime(const char* timeStr, const char* dateStr) {
    if (_lblTime && timeStr) {
        lv_label_set_text(_lblTime, timeStr);
    }
    if (_lblDate && dateStr) {
        lv_label_set_text(_lblDate, dateStr);
    }
}

void UIManager::updateWeather(const WeatherData& data, bool isFahrenheit, bool isWiFiConnected) {
    if (!_cardWeather) return;

    if (data.isValid) {
        char tempBuf[32];
        char feelsBuf[32];
        char humBuf[32];
        char unit = isFahrenheit ? 'F' : 'C';

        snprintf(tempBuf, sizeof(tempBuf), "%.0f°%c  %s", data.temperature, unit, data.conditionText);
        snprintf(feelsBuf, sizeof(feelsBuf), "Feels: %.0f°%c", data.apparentTemperature, unit);
        snprintf(humBuf, sizeof(humBuf), "Humidity: %d%%", data.humidity);

        lv_label_set_text(_lblWeatherTemp, tempBuf);
        lv_label_set_text(_lblWeatherFeels, feelsBuf);
        lv_label_set_text(_lblWeatherHum, humBuf);
    } else {
        const char* status = isWiFiConnected ? data.statusText : "Awaiting WiFi";
        lv_label_set_text(_lblWeatherTemp, status);
        lv_label_set_text(_lblWeatherFeels, "Open-Meteo REST");
        lv_label_set_text(_lblWeatherHum, "Telemetry Active");
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
    snprintf(ssidBuf, sizeof(ssidBuf), "SSID: %.10s", ssid ? ssid : "--");
    lv_label_set_text(_lblSystemSsid, ssidBuf);

    char ipBuf[32];
    snprintf(ipBuf, sizeof(ipBuf), "IP: %s", (connected && ip) ? ip : "Disconnected");
    lv_label_set_text(_lblSystemIp, ipBuf);

    char syncBuf[32];
    snprintf(syncBuf, sizeof(syncBuf), "Sync: %s", syncState ? syncState : "Idle");
    lv_label_set_text(_lblSystemSync, syncBuf);
}
