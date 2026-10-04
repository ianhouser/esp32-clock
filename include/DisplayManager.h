#pragma once

#include <Arduino.h>
#include "DisplayTheme.h"
#include "config.h"

class DisplayManager {
public:
    DisplayManager();

    // Initialize backlight pin and PWM if configured
    void begin(int blPin = PIN_TFT_BL,
               uint8_t channel = PWM_BL_CHANNEL,
               uint32_t freq = PWM_BL_FREQ,
               uint8_t resolution = PWM_BL_RESOLUTION);

    // Evaluate time schedule and transition theme/backlight if necessary.
    // Returns true if a mode transition occurred.
    bool update(int hour, int minute);

    // Manual mode override
    void setMode(DisplayMode mode);

    // Query active state
    DisplayMode getMode() const { return _currentMode; }
    const ThemeColors& getTheme() const;
    bool isNightMode() const { return _currentMode == DisplayMode::NIGHT; }
    const char* getModeString() const { return getTheme().modeLabel; }

    // Update schedule window
    void setSchedule(int startHour, int startMin, int endHour, int endMin);

private:
    int _blPin;
    uint8_t _pwmChannel;
    uint32_t _pwmFreq;
    uint8_t _pwmResolution;
    bool _pwmEnabled;

    int _nightStartHour;
    int _nightStartMin;
    int _nightEndHour;
    int _nightEndMin;

    DisplayMode _currentMode;
    int _lastEvaluatedHour;
    int _lastEvaluatedMin;

    bool isNightTime(int hour, int minute) const;
    void applyBacklight(DisplayMode mode);
};
