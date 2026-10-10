#include "cards/ForecastCard.h"

ForecastCard::ForecastCard()
    : _enabled(true),
      _order(2),
      _container(nullptr) {}

void ForecastCard::serializeConfig(JsonObject& doc) const {
    doc["show_icons"] = true;
}

void ForecastCard::deserializeConfig(const JsonObjectConst& doc) {
    // Config properties
}

void ForecastCard::init(lv_obj_t* parent) {
    _container = parent;
}

void ForecastCard::applyTheme(const ThemeColors& theme) {
    // Theme styling applied through UIManager / styles
}

void ForecastCard::setVisible(bool visible) {
    if (_container) {
        if (visible) {
            lv_obj_clear_flag(_container, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(_container, LV_OBJ_FLAG_HIDDEN);
        }
    }
}
