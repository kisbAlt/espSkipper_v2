#include "ledHandler.hpp"
#include "Arduino.h"

LedHandler::LedHandler(uint8_t pin, const Settings& settings)
    : _pin(pin), _settings(settings), _isOn(false), _isBlinking(false), 
      _blinkInterval(0), _lastBlinkTime(0) {
        _brightness = settings.getButtonBrightnessLevel();
      }

void LedHandler::begin() {
    // Legacy Core v2.x API (only use if v3.x fails to compile)
    const int pwmChannel = 0; // You'd need to manage channels per pin
    ledcSetup(pwmChannel, 5000, 8);
    ledcAttachPin(_pin, pwmChannel);
    off();
}

void LedHandler::setBrightness(uint8_t brightness) {
    _brightness = brightness;
    
    // If the LED is currently ON and not blinking, apply immediately
    if (_isOn && !_isBlinking) {
        ledcWrite(_pin, _brightness);
    }
}

void LedHandler::on() {
    _isBlinking = false;
    _isOn = true;
    ledcWrite(_pin, _brightness); // Apply current brightness level
}

void LedHandler::off() {
    _isBlinking = false;
    _isOn = false;
    ledcWrite(_pin, 0); // 0 duty cycle = OFF
}

void LedHandler::toggle() {
    _isBlinking = false;
    _isOn = !_isOn;
    ledcWrite(_pin, _isOn ? _brightness : 0);
}

void LedHandler::blink(uint32_t intervalMillis) {
    _isBlinking = true;
    _blinkInterval = intervalMillis;
}

void LedHandler::update() {
    if (_isBlinking) {
        unsigned long currentMillis = millis();
        if (currentMillis - _lastBlinkTime >= _blinkInterval) {
            _lastBlinkTime = currentMillis;
            
            // Toggle the current state
            _isOn = !_isOn;
            ledcWrite(_pin, _isOn ? _brightness : 0);
        }
    }
}