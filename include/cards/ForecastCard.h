#pragma once

#include "cards/Card.h"

class ForecastCard : public Card {
public:
    ForecastCard();

    const char* getId() const override { return "forecast"; }
    const char* getTitle() const override { return "2-Day Forecast"; }
    const char* getDescription() const override { return "Upcoming forecast and high/low temperatures"; }
    const char* getIcon() const override { return "calendar-days"; }

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

    void setContainer(lv_obj_t* container) { _container = container; }

private:
    bool _enabled;
    int _order;
    lv_obj_t* _container;
};
