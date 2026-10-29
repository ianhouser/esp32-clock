#include "cards/SystemCard.h"

SystemCard::SystemCard()
    : _enabled(true),
      _order(5),
      _showWifiRssi(true),
      _showUptime(true),
      _showFreeHeap(true),
      _container(nullptr) {}

void SystemCard::serializeConfig(JsonObject& doc) const {
    doc["show_wifi_rssi"] = _showWifiRssi;
    doc["show_uptime"] = _showUptime;
    doc["show_free_heap"] = _showFreeHeap;
}

void SystemCard::deserializeConfig(const JsonObjectConst& doc) {
    if (doc["show_wifi_rssi"].is<bool>()) {
        _showWifiRssi = doc["show_wifi_rssi"].as<bool>();
    }
    if (doc["show_uptime"].is<bool>()) {
        _showUptime = doc["show_uptime"].as<bool>();
    }
    if (doc["show_free_heap"].is<bool>()) {
        _showFreeHeap = doc["show_free_heap"].as<bool>();
    }
}

void SystemCard::init(lv_obj_t* parent) {
    _container = parent;
}

void SystemCard::applyTheme(const ThemeColors& theme) {
    // Styling
}

void SystemCard::setVisible(bool visible) {
    if (_container) {
        if (visible) {
            lv_obj_clear_flag(_container, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(_container, LV_OBJ_FLAG_HIDDEN);
        }
    }
}
