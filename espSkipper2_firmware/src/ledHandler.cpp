#include "ledHandler.hpp"
#include "Arduino.h"

LedHandler::LedHandler(uint8_t pin, IsLedEnabledCallback isLedEnabled, GetLedBrightnessCallback getLedBrightness, uint8_t pwmChannel)
    : _pin(pin), _pwmChannel(pwmChannel), isLedEnabled(isLedEnabled), getLedBrightness(getLedBrightness), _isBlinking(false),
      _blinkInterval(0), _lastBlinkTime(0)
{
}

void LedHandler::begin()
{
    ledcSetup(_pwmChannel, 5000, 8);
    ledcAttachPin(_pin, _pwmChannel);
    reload();
}

void LedHandler::update()
{
    if (_isBlinking)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - _lastBlinkTime >= _blinkInterval)
        {
            _lastBlinkTime = currentMillis;
            reload();
        }
    }
}

void LedHandler::reload()
{
    ledcWrite(_pwmChannel, isLedEnabled() ? getLedBrightness() : 0);
}
