#pragma once

#include "cards/Card.h"

class CalendarCard : public Card {
public:
    CalendarCard();

    const char* getId() const override { return "calendar"; }
    const char* getTitle() const override { return "Calendar & Events"; }
    const char* getDescription() const override { return "Upcoming calendar events and agenda timeline"; }
    const char* getIcon() const override { return "calendar"; }

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

    int getMaxEvents() const { return _maxEvents; }
    bool showCountdown() const { return _showCountdown; }

private:
    bool _enabled;
    int _order;
    int _maxEvents;
    bool _showCountdown;
    String _calendarUrl;
    lv_obj_t* _container;
};
