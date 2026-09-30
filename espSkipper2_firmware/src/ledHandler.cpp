#include "ledHandler.hpp"
#include "Arduino.h"

LedHandler::LedHandler(uint8_t pin, IsLedEnabledCallback isLedEnabled, GetLedBrightnessCallback getLedBrightness, uint8_t pwmChannel)
        : pin(pin), pwmChannel(pwmChannel), isLedEnabled(isLedEnabled), getLedBrightness(getLedBrightness), isBlinking(false),
            blinkInterval(0), lastBlinkTime(0)
{
}

void LedHandler::Begin()
{
    ledcSetup(pwmChannel, 5000, 8);
    ledcAttachPin(pin, pwmChannel);
    Reload();
}

void LedHandler::Update()
{
    if (isBlinking)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - lastBlinkTime >= blinkInterval)
        {
            lastBlinkTime = currentMillis;
            Reload();
        }
    }
}

void LedHandler::Reload()
{
    ledcWrite(pwmChannel, isLedEnabled() ? getLedBrightness() : 0);
}
