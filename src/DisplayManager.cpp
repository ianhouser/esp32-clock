#include "DisplayManager.h"
#include <esp_arduino_version.h>

DisplayManager::DisplayManager()
    : _blPin(-1),
      _pwmChannel(PWM_BL_CHANNEL),
      _pwmFreq(PWM_BL_FREQ),
      _pwmResolution(PWM_BL_RESOLUTION),
      _pwmEnabled(false),
      _nightStartHour(NIGHT_MODE_START_HOUR),
      _nightStartMin(NIGHT_MODE_START_MIN),
      _nightEndHour(NIGHT_MODE_END_HOUR),
      _nightEndMin(NIGHT_MODE_END_MIN),
      _currentMode(DisplayMode::DAY),
      _lastEvaluatedHour(-1),
      _lastEvaluatedMin(-1) {}

void DisplayManager::begin(int blPin, uint8_t channel, uint32_t freq, uint8_t resolution) {
    _blPin = blPin;
    _pwmChannel = channel;
    _pwmFreq = freq;
    _pwmResolution = resolution;

    if (_blPin >= 0) {
        Serial.printf("[DisplayManager] Initializing hardware backlight PWM on GPIO %d (ch %u, %u Hz)\n",
                      _blPin, _pwmChannel, _pwmFreq);
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
        ledcAttachChannel(_blPin, _pwmFreq, _pwmResolution, _pwmChannel);
#else
        ledcSetup(_pwmChannel, _pwmFreq, _pwmResolution);
        ledcAttachPin(_blPin, _pwmChannel);
#endif
        _pwmEnabled = true;
        applyBacklight(_currentMode);
    } else {
        Serial.println("[DisplayManager] Hardware backlight PWM disabled. Using software theme dimming.");
    }
}

bool DisplayManager::update(int hour, int minute) {
    if (hour < 0 || minute < 0) {
        return false; // Time not yet valid / synchronized
    }

    if (hour == _lastEvaluatedHour && minute == _lastEvaluatedMin) {
        return false; // Already evaluated this minute
    }

    _lastEvaluatedHour = hour;
    _lastEvaluatedMin = minute;

    bool shouldBeNight = isNightTime(hour, minute);
    DisplayMode targetMode = shouldBeNight ? DisplayMode::NIGHT : DisplayMode::DAY;

    if (targetMode != _currentMode) {
        Serial.printf("[DisplayManager] Mode transition triggered at %02d:%02d -> %s Mode\n",
                      hour, minute, (targetMode == DisplayMode::NIGHT ? "NIGHT" : "DAY"));
        setMode(targetMode);
        return true;
    }

    return false;
}

void DisplayManager::setMode(DisplayMode mode) {
    _currentMode = mode;
    applyBacklight(_currentMode);
}

const ThemeColors& DisplayManager::getTheme() const {
    return (_currentMode == DisplayMode::NIGHT) ? THEME_NIGHT : THEME_DAY;
}

void DisplayManager::setSchedule(int startHour, int startMin, int endHour, int endMin) {
    _nightStartHour = startHour;
    _nightStartMin = startMin;
    _nightEndHour = endHour;
    _nightEndMin = endMin;
    _lastEvaluatedHour = -1; // Force re-evaluation on next update()
}

bool DisplayManager::isNightTime(int hour, int minute) const {
    int currentTotal = hour * 60 + minute;
    int startTotal = _nightStartHour * 60 + _nightStartMin;
    int endTotal = _nightEndHour * 60 + _nightEndMin;

    if (startTotal > endTotal) {
        // Overnight schedule across midnight (e.g. 22:00 to 07:00)
        return (currentTotal >= startTotal || currentTotal < endTotal);
    } else {
        // Intra-day schedule (e.g. 01:00 to 06:00)
        return (currentTotal >= startTotal && currentTotal < endTotal);
    }
}

void DisplayManager::applyBacklight(DisplayMode mode) {
    if (!_pwmEnabled || _blPin < 0) {
        return;
    }

    uint32_t duty = (mode == DisplayMode::NIGHT) ? PWM_BL_NIGHT_DUTY : PWM_BL_DAY_DUTY;

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    ledcWrite(_blPin, duty);
#else
    ledcWrite(_pwmChannel, duty);
#endif

    Serial.printf("[DisplayManager] Backlight duty set to %u (%s)\n",
                  duty, (mode == DisplayMode::NIGHT ? "Night" : "Day"));
}
