#include "cards/ClockCard.h"

ClockCard::ClockCard()
    : _enabled(true),
      _order(0),
      _format12h(true),
      _showSeconds(true),
      _dateFormat("MDY"),
      _container(nullptr) {}

void ClockCard::serializeConfig(JsonObject& doc) const {
    doc["format_12h"] = _format12h;
    doc["show_seconds"] = _showSeconds;
    doc["date_format"] = _dateFormat;
}

void ClockCard::deserializeConfig(const JsonObjectConst& doc) {
    if (doc["format_12h"].is<bool>()) {
        _format12h = doc["format_12h"].as<bool>();
    }
    if (doc["show_seconds"].is<bool>()) {
        _showSeconds = doc["show_seconds"].as<bool>();
    }
    if (doc["date_format"].is<const char*>()) {
        _dateFormat = doc["date_format"].as<String>();
    }
}

void ClockCard::init(lv_obj_t* parent) {
    _container = parent;
}

void ClockCard::applyTheme(const ThemeColors& theme) {
    // Theme styling applied through UIManager / styles
}

void ClockCard::setVisible(bool visible) {
    if (_container) {
        if (visible) {
            lv_obj_clear_flag(_container, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(_container, LV_OBJ_FLAG_HIDDEN);
        }
    }
}
