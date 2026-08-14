#ifndef LED_HANDLER_HPP
#define LED_HANDLER_HPP

#include <Arduino.h>
#include "settingsHandler.hpp"

class LedHandler {
public:
    // Constructor takes the GPIO pin number
    LedHandler(uint8_t pin, const Settings& settings, uint8_t pwmChannel = 0);
    
    // Initializes the pin
    void begin();
    
    // Direct control methods
    void on();
    void off();
    void toggle();
    
    // Non-blocking blink functionality
    void blink(uint32_t intervalMillis);
    
    // Must be called in the loop if using the blink feature
    void update();

private:
    uint8_t _pin;
    uint8_t _pwmChannel;
    bool _isOn;
    bool _isBlinking;
    uint32_t _blinkInterval;
    unsigned long _lastBlinkTime;
    const Settings& _settings;
};

#endif