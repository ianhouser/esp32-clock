#include "ConfigManager.h"
#include "cards/CardRegistry.h"

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager() {
    setDefaults();
}

void ConfigManager::setDefaults() {
    _theme.preset = "cyberpunk";
    _theme.accentColor = "#00FFFF";
    _theme.bgColor = "#081018";
    _theme.cardBgColor = "#101C28";
    _theme.cardBorderColor = "#203850";
    _theme.timeColor = "#00FFFF";
    _theme.dateColor = "#FFFFFF";
    _theme.mutedTextColor = "#8CA0B8";

    _theme.dayDuty = 255;
    _theme.nightDuty = 38;
    _theme.nightStartHour = 22;
    _theme.nightStartMin = 0;
    _theme.nightEndHour = 7;
    _theme.nightEndMin = 0;

    _system.mdnsHostname = "esp32-clock";
    _system.timezone = "PST8PDT,M3.2.0,M11.1.0";
    _system.ntpServer = "pool.ntp.org";
}

bool ConfigManager::begin() {
    if (!LittleFS.begin(true)) {
        Serial.println("[ConfigManager] Failed to mount LittleFS!");
        return false;
    }
    Serial.println("[ConfigManager] LittleFS mounted successfully.");
    return load();
}

bool ConfigManager::load() {
    if (!LittleFS.exists(CONFIG_PATH)) {
        Serial.println("[ConfigManager] config.json does not exist. Creating defaults.");
        return save();
    }

    File file = LittleFS.open(CONFIG_PATH, "r");
    if (!file) {
        Serial.println("[ConfigManager] Failed to open config.json for reading.");
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.printf("[ConfigManager] Failed to parse config.json: %s. Using defaults.\n", error.c_str());
        return false;
    }

    return deserializeConfig(doc);
}

bool ConfigManager::save() {
    File file = LittleFS.open(CONFIG_PATH, "w");
    if (!file) {
        Serial.println("[ConfigManager] Failed to open config.json for writing.");
        return false;
    }

    JsonDocument doc;
    serializeConfig(doc);

    if (serializeJson(doc, file) == 0) {
        Serial.println("[ConfigManager] Failed to write config.json");
        file.close();
        return false;
    }

    file.close();
    Serial.println("[ConfigManager] Saved config.json successfully.");
    return true;
}

void ConfigManager::serializeConfig(JsonDocument& doc) const {
    JsonObject themeObj = doc["theme"].to<JsonObject>();
    themeObj["preset"] = _theme.preset;
    themeObj["accent_color"] = _theme.accentColor;
    themeObj["bg_color"] = _theme.bgColor;
    themeObj["card_bg_color"] = _theme.cardBgColor;
    themeObj["card_border_color"] = _theme.cardBorderColor;
    themeObj["time_color"] = _theme.timeColor;
    themeObj["date_color"] = _theme.dateColor;
    themeObj["muted_text_color"] = _theme.mutedTextColor;

    themeObj["day_duty"] = _theme.dayDuty;
    themeObj["night_duty"] = _theme.nightDuty;
    themeObj["night_start_hour"] = _theme.nightStartHour;
    themeObj["night_start_min"] = _theme.nightStartMin;
    themeObj["night_end_hour"] = _theme.nightEndHour;
    themeObj["night_end_min"] = _theme.nightEndMin;

    JsonArray cardsArray = doc["cards"].to<JsonArray>();
    CardRegistry::getInstance().serializeAll(cardsArray);

    JsonObject sysObj = doc["system"].to<JsonObject>();
    sysObj["mdns_hostname"] = _system.mdnsHostname;
    sysObj["timezone"] = _system.timezone;
    sysObj["ntp_server"] = _system.ntpServer;
}

bool ConfigManager::deserializeConfig(const JsonDocument& doc) {
    if (doc["theme"].is<JsonObjectConst>()) {
        JsonObjectConst t = doc["theme"].as<JsonObjectConst>();
        if (t["preset"].is<const char*>()) _theme.preset = t["preset"].as<String>();
        if (t["accent_color"].is<const char*>()) _theme.accentColor = t["accent_color"].as<String>();
        if (t["bg_color"].is<const char*>()) _theme.bgColor = t["bg_color"].as<String>();
        if (t["card_bg_color"].is<const char*>()) _theme.cardBgColor = t["card_bg_color"].as<String>();
        if (t["card_border_color"].is<const char*>()) _theme.cardBorderColor = t["card_border_color"].as<String>();
        if (t["time_color"].is<const char*>()) _theme.timeColor = t["time_color"].as<String>();
        if (t["date_color"].is<const char*>()) _theme.dateColor = t["date_color"].as<String>();
        if (t["muted_text_color"].is<const char*>()) _theme.mutedTextColor = t["muted_text_color"].as<String>();

        if (t["day_duty"].is<uint8_t>()) _theme.dayDuty = t["day_duty"].as<uint8_t>();
        if (t["night_duty"].is<uint8_t>()) _theme.nightDuty = t["night_duty"].as<uint8_t>();
        if (t["night_start_hour"].is<uint8_t>()) _theme.nightStartHour = t["night_start_hour"].as<uint8_t>();
        if (t["night_start_min"].is<uint8_t>()) _theme.nightStartMin = t["night_start_min"].as<uint8_t>();
        if (t["night_end_hour"].is<uint8_t>()) _theme.nightEndHour = t["night_end_hour"].as<uint8_t>();
        if (t["night_end_min"].is<uint8_t>()) _theme.nightEndMin = t["night_end_min"].as<uint8_t>();
    }

    if (doc["cards"].is<JsonArrayConst>()) {
        CardRegistry::getInstance().deserializeAll(doc["cards"].as<JsonArrayConst>());
    }

    if (doc["system"].is<JsonObjectConst>()) {
        JsonObjectConst s = doc["system"].as<JsonObjectConst>();
        if (s["mdns_hostname"].is<const char*>()) _system.mdnsHostname = s["mdns_hostname"].as<String>();
        if (s["timezone"].is<const char*>()) _system.timezone = s["timezone"].as<String>();
        if (s["ntp_server"].is<const char*>()) _system.ntpServer = s["ntp_server"].as<String>();
    }

    if (_onChanged) {
        _onChanged();
    }
    return true;
}

ThemeColors ConfigManager::getActiveThemeColors(DisplayMode mode) const {
    if (mode == DisplayMode::NIGHT) {
        return THEME_NIGHT;
    }

    ThemeColors colors;
    colors.bg = hexToRGB565(_theme.bgColor.c_str());
    colors.headerBg = hexToRGB565(_theme.bgColor.c_str());
    colors.cardBg = hexToRGB565(_theme.cardBgColor.c_str());
    colors.cardBorder = hexToRGB565(_theme.cardBorderColor.c_str());
    colors.accentColor = hexToRGB565(_theme.accentColor.c_str());
    colors.timeColor = hexToRGB565(_theme.timeColor.c_str());
    colors.dateColor = hexToRGB565(_theme.dateColor.c_str());
    colors.mutedText = hexToRGB565(_theme.mutedTextColor.c_str());
    colors.modeBadgeBg = hexToRGB565(_theme.cardBgColor.c_str());
    colors.modeBadgeText = hexToRGB565(_theme.accentColor.c_str());
    colors.modeLabel = "DAY";
    return colors;
}
