#pragma once

#include "cards/Card.h"

class NotificationCard : public Card {
public:
    NotificationCard();

    const char* getId() const override { return "notifications"; }
    const char* getTitle() const override { return "Email & Notifications"; }
    const char* getDescription() const override { return "Unread email badges and messaging alert banners"; }
    const char* getIcon() const override { return "bell"; }

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

    bool showBadges() const { return _showBadges; }
    int getTimeoutSec() const { return _timeoutSec; }

private:
    bool _enabled;
    int _order;
    bool _showBadges;
    int _timeoutSec;
    lv_obj_t* _container;
};
