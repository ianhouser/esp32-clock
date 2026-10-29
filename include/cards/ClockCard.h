#pragma once

#include "cards/Card.h"

class ClockCard : public Card {
public:
    ClockCard();

    const char* getId() const override { return "clock"; }
    const char* getTitle() const override { return "Clock & Date"; }
    const char* getDescription() const override { return "Anti-aliased digital time and calendar date"; }
    const char* getIcon() const override { return "clock"; }

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
    bool is12Hour() const { return _format12h; }
    bool showSeconds() const { return _showSeconds; }
    const String& getDateFormat() const { return _dateFormat; }

    void setContainer(lv_obj_t* container) { _container = container; }

private:
    bool _enabled;
    int _order;
    bool _format12h;
    bool _showSeconds;
    String _dateFormat;
    lv_obj_t* _container;
};
