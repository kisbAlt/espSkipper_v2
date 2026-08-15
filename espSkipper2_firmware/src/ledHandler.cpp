#include "ledHandler.hpp"
#include "Arduino.h"

LedHandler::LedHandler(uint8_t pin, const Settings &settings, uint8_t pwmChannel)
    : _pin(pin), _pwmChannel(pwmChannel), _settings(settings), _isBlinking(false),
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
    ledcWrite(_pwmChannel, _settings.getValue<bool>(SettingsKey::BtnLedEnabled) ? _settings.getValue<uint8_t>(SettingsKey::BtnBrightness) : 0);
}
