#pragma once

#include "cards/Card.h"

class SystemCard : public Card {
public:
    SystemCard();

    const char* getId() const override { return "system"; }
    const char* getTitle() const override { return "System & Network"; }
    const char* getDescription() const override { return "WiFi RSSI, IP address, uptime, and memory diagnostics"; }
    const char* getIcon() const override { return "cpu"; }

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

    bool showWifiRssi() const { return _showWifiRssi; }
    bool showUptime() const { return _showUptime; }
    bool showFreeHeap() const { return _showFreeHeap; }

private:
    bool _enabled;
    int _order;
    bool _showWifiRssi;
    bool _showUptime;
    bool _showFreeHeap;
    lv_obj_t* _container;
};
