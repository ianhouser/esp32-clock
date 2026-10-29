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
    void setContainer(lv_obj_t* container) { _container = container; }

    bool showBadges() const { return _showBadges; }
    int getTimeoutSec() const { return _timeoutSec; }
    const String& getNtfyServer() const { return _ntfyServer; }
    const String& getNtfyTopic() const { return _ntfyTopic; }
    int getPollIntervalSec() const { return _pollIntervalSec; }
    int getCarouselIntervalSec() const { return _carouselIntervalSec; }
    bool skipEmptyAgenda() const { return _skipEmptyAgenda; }

    void setNtfyServer(const String& s) { _ntfyServer = s; }
    void setNtfyTopic(const String& t) { _ntfyTopic = t; }
    void setPollIntervalSec(int sec) { _pollIntervalSec = sec; }
    void setTimeoutSec(int sec) { _timeoutSec = sec; }
    void setShowBadges(bool b) { _showBadges = b; }
    void setCarouselIntervalSec(int sec) { _carouselIntervalSec = sec; }
    void setSkipEmptyAgenda(bool b) { _skipEmptyAgenda = b; }

private:
    bool _enabled;
    int _order;
    bool _showBadges;
    int _timeoutSec;
    String _ntfyServer;
    String _ntfyTopic;
    int _pollIntervalSec;
    int _carouselIntervalSec;
    bool _skipEmptyAgenda;
    lv_obj_t* _container;
};
