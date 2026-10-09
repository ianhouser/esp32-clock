#include "cards/NotificationCard.h"

NotificationCard::NotificationCard()
    : _enabled(false), // Disabled by default until configured
      _order(4),
      _showBadges(true),
      _timeoutSec(30),
      _container(nullptr) {}

void NotificationCard::serializeConfig(JsonObject& doc) const {
    doc["show_badges"] = _showBadges;
    doc["timeout_sec"] = _timeoutSec;
}

void NotificationCard::deserializeConfig(const JsonObjectConst& doc) {
    if (doc["show_badges"].is<bool>()) {
        _showBadges = doc["show_badges"].as<bool>();
    }
    if (doc["timeout_sec"].is<int>()) {
        _timeoutSec = doc["timeout_sec"].as<int>();
    }
}

void NotificationCard::init(lv_obj_t* parent) {
    _container = parent;
}

void NotificationCard::applyTheme(const ThemeColors& theme) {
    // Styling
}

void NotificationCard::setVisible(bool visible) {
    if (_container) {
        if (visible) {
            lv_obj_clear_flag(_container, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(_container, LV_OBJ_FLAG_HIDDEN);
        }
    }
}
