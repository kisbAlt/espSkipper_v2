#include "ledHandler.hpp"
#include "Arduino.h"

// I added pwmChannel as a parameter, defaulting to 0 for your use case
LedHandler::LedHandler(uint8_t pin, const Settings &settings, uint8_t pwmChannel)
    : _pin(pin), _pwmChannel(pwmChannel), _settings(settings), _isOn(false), _isBlinking(false),
      _blinkInterval(0), _lastBlinkTime(0)
{
}

void LedHandler::begin()
{
    // Setup the channel and attach the pin to it
    ledcSetup(_pwmChannel, 5000, 8);
    ledcAttachPin(_pin, _pwmChannel);
    off();
}

void LedHandler::on()
{
    _isBlinking = false;
    _isOn = true;
    ledcWrite(_pwmChannel, _settings.getButtonBrightnessLevel()); // Changed _pin to _pwmChannel
}

void LedHandler::off()
{
    _isBlinking = false;
    _isOn = false;
    ledcWrite(_pwmChannel, 0); // Changed _pin to _pwmChannel
}

void LedHandler::toggle()
{
    _isBlinking = false;
    _isOn = !_isOn;
    ledcWrite(_pwmChannel, _isOn ? _settings.getButtonBrightnessLevel() : 0); // Changed _pin to _pwmChannel
}

void LedHandler::update()
{
    if (_isBlinking)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - _lastBlinkTime >= _blinkInterval)
        {
            _lastBlinkTime = currentMillis;

            _isOn = !_isOn;
            ledcWrite(_pwmChannel, _isOn ? _settings.getButtonBrightnessLevel() : 0); // Changed _pin to _pwmChannel
        }
    }
}