#ifndef LED_HANDLER_HPP
#define LED_HANDLER_HPP

#include <Arduino.h>
#include "settingsHandler.hpp"

class LedHandler {
public:
    using IsLedEnabledCallback = std::function<bool()>;
    using GetLedBrightnessCallback = std::function<uint8_t()>;

    LedHandler(uint8_t pin, IsLedEnabledCallback isLedEnabled, GetLedBrightnessCallback getLedBrightness, uint8_t pwmChannel = 0);

    void Begin();
    void Blink(uint32_t intervalMillis);
    void Update();
    void Reload();

private:
    uint8_t pin;
    uint8_t pwmChannel;
    bool isBlinking;
    uint32_t blinkInterval;
    unsigned long lastBlinkTime;
    IsLedEnabledCallback isLedEnabled;
    GetLedBrightnessCallback getLedBrightness;
};

#endif