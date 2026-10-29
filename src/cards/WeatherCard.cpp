#include "cards/WeatherCard.h"

WeatherCard::WeatherCard()
    : _enabled(true),
      _order(1),
      _cityName("San Francisco"),
      _lat(DEFAULT_WEATHER_LAT),
      _lon(DEFAULT_WEATHER_LON),
      _useFahrenheit(DEFAULT_WEATHER_USE_FAHR),
      _updateIntervalMin(15),
      _container(nullptr) {}

void WeatherCard::serializeConfig(JsonObject& doc) const {
    doc["city_name"] = _cityName;
    doc["lat"] = _lat;
    doc["lon"] = _lon;
    doc["use_fahrenheit"] = _useFahrenheit;
    doc["update_interval_min"] = _updateIntervalMin;
}

void WeatherCard::deserializeConfig(const JsonObjectConst& doc) {
    if (doc["city_name"].is<const char*>()) {
        _cityName = doc["city_name"].as<String>();
    }
    if (doc["lat"].is<float>()) {
        _lat = doc["lat"].as<float>();
    }
    if (doc["lon"].is<float>()) {
        _lon = doc["lon"].as<float>();
    }
    if (doc["use_fahrenheit"].is<bool>()) {
        _useFahrenheit = doc["use_fahrenheit"].as<bool>();
    }
    if (doc["update_interval_min"].is<int>()) {
        _updateIntervalMin = doc["update_interval_min"].as<int>();
    }
}

void WeatherCard::init(lv_obj_t* parent) {
    _container = parent;
}

void WeatherCard::applyTheme(const ThemeColors& theme) {
    // Theme styling applied through UIManager / styles
}

void WeatherCard::setVisible(bool visible) {
    if (_container) {
        if (visible) {
            lv_obj_clear_flag(_container, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(_container, LV_OBJ_FLAG_HIDDEN);
        }
    }
}
