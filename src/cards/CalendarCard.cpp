#include "cards/CalendarCard.h"

CalendarCard::CalendarCard()
    : _enabled(false), // Disabled by default until configured
      _order(3),
      _maxEvents(3),
      _showCountdown(true),
      _calendarUrl(""),
      _container(nullptr) {}

void CalendarCard::serializeConfig(JsonObject& doc) const {
    doc["max_events"] = _maxEvents;
    doc["show_countdown"] = _showCountdown;
    doc["calendar_url"] = _calendarUrl;

    JsonArray sourcesArr = doc["sources"].to<JsonArray>();
    for (const auto& src : _sources) {
        JsonObject srcObj = sourcesArr.add<JsonObject>();
        srcObj["name"] = src.name;
        srcObj["url"] = src.url;
        srcObj["color"] = src.color;
    }
}

void CalendarCard::deserializeConfig(const JsonObjectConst& doc) {
    if (doc["max_events"].is<int>()) {
        _maxEvents = doc["max_events"].as<int>();
    }
    if (doc["show_countdown"].is<bool>()) {
        _showCountdown = doc["show_countdown"].as<bool>();
    }
    if (doc["calendar_url"].is<const char*>()) {
        _calendarUrl = doc["calendar_url"].as<String>();
    }

    _sources.clear();
    if (doc["sources"].is<JsonArrayConst>()) {
        for (JsonObjectConst srcObj : doc["sources"].as<JsonArrayConst>()) {
            CalendarSource src;
            src.name = srcObj["name"] | "Calendar";
            src.url = srcObj["url"] | "";
            src.color = srcObj["color"] | "#3B82F6";
            if (src.url.length() > 0) {
                _sources.push_back(src);
            }
        }
    }

    // Fallback: if no sources in array but legacy calendar_url is configured
    if (_sources.empty() && _calendarUrl.length() > 0) {
        CalendarSource def;
        def.name = "Primary";
        def.url = _calendarUrl;
        def.color = "#3B82F6";
        _sources.push_back(def);
    }
}

void CalendarCard::init(lv_obj_t* parent) {
    _container = parent;
}

void CalendarCard::applyTheme(const ThemeColors& theme) {
    // Styling
}

void CalendarCard::setVisible(bool visible) {
    if (_container) {
        if (visible) {
            lv_obj_clear_flag(_container, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(_container, LV_OBJ_FLAG_HIDDEN);
        }
    }
}
