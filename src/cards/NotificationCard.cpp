#include "cards/NotificationCard.h"

NotificationCard::NotificationCard()
    : _enabled(false), // Disabled by default until configured
      _order(4),
      _showBadges(true),
      _timeoutSec(30),
      _ntfyServer("https://ntfy.sh"),
      _ntfyTopic(""),
      _pollIntervalSec(10),
      _carouselIntervalSec(15),
      _skipEmptyAgenda(true),
      _container(nullptr) {}

void NotificationCard::serializeConfig(JsonObject& doc) const {
    doc["show_badges"] = _showBadges;
    doc["timeout_sec"] = _timeoutSec;
    doc["ntfy_server"] = _ntfyServer;
    doc["ntfy_topic"] = _ntfyTopic;
    doc["poll_interval_sec"] = _pollIntervalSec;
    doc["carousel_interval_sec"] = _carouselIntervalSec;
    doc["skip_empty_agenda"] = _skipEmptyAgenda;
}

void NotificationCard::deserializeConfig(const JsonObjectConst& doc) {
    if (doc["show_badges"].is<bool>()) {
        _showBadges = doc["show_badges"].as<bool>();
    }
    if (doc["timeout_sec"].is<int>()) {
        _timeoutSec = doc["timeout_sec"].as<int>();
    }
    if (doc["ntfy_server"].is<const char*>()) {
        _ntfyServer = doc["ntfy_server"].as<const char*>();
    }
    if (doc["ntfy_topic"].is<const char*>()) {
        _ntfyTopic = doc["ntfy_topic"].as<const char*>();
    }
    if (doc["poll_interval_sec"].is<int>()) {
        _pollIntervalSec = doc["poll_interval_sec"].as<int>();
    }
    if (doc["carousel_interval_sec"].is<int>()) {
        _carouselIntervalSec = doc["carousel_interval_sec"].as<int>();
    }
    if (doc["skip_empty_agenda"].is<bool>()) {
        _skipEmptyAgenda = doc["skip_empty_agenda"].as<bool>();
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
