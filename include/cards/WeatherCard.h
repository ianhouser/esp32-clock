#pragma once

#include "cards/Card.h"
#include "config.h"

class WeatherCard : public Card {
public:
    WeatherCard();

    const char* getId() const override { return "weather"; }
    const char* getTitle() const override { return "Current Weather"; }
    const char* getDescription() const override { return "Live temperature, sky conditions, and humidity"; }
    const char* getIcon() const override { return "cloud-sun"; }

    bool isEnabled() const override { return _enabled; }
    void setEnabled(bool enabled) override { _enabled = enabled; }
    int getOrder() const override { return _order; }
    void setOrder(int order) override { _order = order; }

    void serializeConfig(JsonObject& doc) const override;
    void deserializeConfig(const JsonObjectConst& doc) override;

    void init(lv_obj_t* parent) override;
    void applyTheme(const ThemeColors& theme) override;
    void setVisible(bool visible) override;
    lv_obj_t* getContainer() const override { return _container; }

    // Configuration getters
    const String& getCityName() const { return _cityName; }
    float getLat() const { return _lat; }
    float getLon() const { return _lon; }
    bool isFahrenheit() const { return _useFahrenheit; }
    int getUpdateIntervalMin() const { return _updateIntervalMin; }

    void setContainer(lv_obj_t* container) { _container = container; }

private:
    bool _enabled;
    int _order;
    String _cityName;
    float _lat;
    float _lon;
    bool _useFahrenheit;
    int _updateIntervalMin;
    lv_obj_t* _container;
};
