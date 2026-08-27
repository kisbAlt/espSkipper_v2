#ifndef LED_HANDLER_HPP
#define LED_HANDLER_HPP

#include <Arduino.h>
#include "settingsHandler.hpp"

class LedHandler {
public:
    using IsLedEnabledCallback = std::function<bool()>;
    using GetLedBrightnessCallback = std::function<uint8_t()>;
    // Constructor takes the GPIO pin number
    LedHandler(uint8_t pin, IsLedEnabledCallback isLedEnabled, GetLedBrightnessCallback getLedBrightness, uint8_t pwmChannel = 0);

    
    // Initializes the pin
    void begin();
    
    // Non-blocking blink functionality
    void blink(uint32_t intervalMillis);
    
    // Must be called in the loop if using the blink feature
    void update();
    void reload();

private:
    uint8_t _pin;
    uint8_t _pwmChannel;
    bool _isBlinking;
    uint32_t _blinkInterval;
    unsigned long _lastBlinkTime;
    IsLedEnabledCallback isLedEnabled;
    GetLedBrightnessCallback getLedBrightness;
};

#endif