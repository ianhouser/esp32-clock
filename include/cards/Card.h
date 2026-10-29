#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <lvgl.h>
#include "DisplayTheme.h"

/**
 * Base abstract class for modular dashboard cards.
 * Enables adding new cards (Calendar, Notifications, Smart Home, etc.)
 * with zero modifications to the core web server or config pipeline.
 */
class Card {
public:
    virtual ~Card() = default;

    // Unique machine identifier (e.g. "clock", "weather", "calendar", "notifications")
    virtual const char* getId() const = 0;

    // Display title & metadata
    virtual const char* getTitle() const = 0;
    virtual const char* getDescription() const = 0;
    virtual const char* getIcon() const = 0;

    // Enabled state & display ordering
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual int getOrder() const = 0;
    virtual void setOrder(int order) = 0;

    // JSON Configuration serialization for REST API & LittleFS /config.json
    virtual void serializeConfig(JsonObject& doc) const = 0;
    virtual void deserializeConfig(const JsonObjectConst& doc) = 0;

    // UI & LVGL Lifecycle
    virtual void init(lv_obj_t* parent) = 0;
    virtual void applyTheme(const ThemeColors& theme) = 0;
    virtual void setVisible(bool visible) = 0;
    virtual lv_obj_t* getContainer() const = 0;
};
