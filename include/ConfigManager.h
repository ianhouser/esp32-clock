#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <functional>
#include "DisplayTheme.h"

struct DeviceThemeConfig {
    String preset;
    String accentColor;
    String bgColor;
    String cardBgColor;
    String cardBorderColor;
    String timeColor;
    String dateColor;
    String mutedTextColor;

    uint8_t dayDuty;
    uint8_t nightDuty;
    uint8_t nightStartHour;
    uint8_t nightStartMin;
    uint8_t nightEndHour;
    uint8_t nightEndMin;
};

struct DeviceSystemConfig {
    String mdnsHostname;
    String timezone;
    String ntpServer;
};

class ConfigManager {
public:
    static ConfigManager& getInstance();

    bool begin();
    bool load();
    bool save();

    // Export & Import full JSON
    void serializeConfig(JsonDocument& doc) const;
    bool deserializeConfig(const JsonDocument& doc);

    // Dynamic Theme struct accessor
    ThemeColors getActiveThemeColors(DisplayMode mode) const;

    // Getters
    const DeviceThemeConfig& getThemeConfig() const { return _theme; }
    const DeviceSystemConfig& getSystemConfig() const { return _system; }

    // Change listener callback
    using ConfigChangeCallback = std::function<void()>;
    void onConfigChanged(ConfigChangeCallback cb) { _onChanged = cb; }

private:
    ConfigManager();
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    void setDefaults();

    DeviceThemeConfig _theme;
    DeviceSystemConfig _system;
    ConfigChangeCallback _onChanged;
    static constexpr const char* CONFIG_PATH = "/config.json";
};
