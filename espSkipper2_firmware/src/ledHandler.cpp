#include "ledHandler.hpp"

LedHandler::LedHandler(uint8_t pin)
    : _pin(pin), _isOn(false), _isBlinking(false), 
      _blinkInterval(0), _lastBlinkTime(0) {}

void LedHandler::begin() {
    pinMode(_pin, OUTPUT);
    off(); // Ensure it starts turned off
}

void LedHandler::on() {
    _isBlinking = false;
    _isOn = true;
    digitalWrite(_pin, HIGH); // HIGH turns ON the S8050 NPN transistor
}

void LedHandler::off() {
    _isBlinking = false;
    _isOn = false;
    digitalWrite(_pin, LOW);  // LOW turns OFF the S8050 NPN transistor
}

void LedHandler::toggle() {
    _isBlinking = false;
    _isOn = !_isOn;
    digitalWrite(_pin, _isOn ? HIGH : LOW);
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
            digitalWrite(_pin, _isOn ? HIGH : LOW);
        }
    }
}