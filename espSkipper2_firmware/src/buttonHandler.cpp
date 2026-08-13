#include "buttonHandler.hpp"

ButtonHandler::ButtonHandler(uint8_t pin)
    : _pin(pin), _lastState(HIGH), _currentState(HIGH), _isPressing(false),
      _lastDebounceTime(0), _pressedTime(0), _releasedTime(0),
      _clickCount(0), _longPressHandled(false) {}

void ButtonHandler::begin() {
    // Enable internal pull-up resistor (Button normally HIGH, goes LOW when pressed)
    pinMode(_pin, INPUT_PULLUP);
}

ButtonEvent ButtonHandler::update() {
    ButtonEvent event = ButtonEvent::NONE;
    bool reading = digitalRead(_pin);
    unsigned long currentMillis = millis();

    // 1. Eager Debounce Logic (Ignore noise after a state change)
    if ((currentMillis - _lastDebounceTime) > DEBOUNCE_DELAY) {
        if (reading != _currentState) {
            _currentState = reading;
            _lastDebounceTime = currentMillis; // Lockout further changes

            if (_currentState == LOW) { 
                // Button was JUST pressed
                _isPressing = true;
                _pressedTime = currentMillis;
                _longPressHandled = false;
            } else { 
                // Button was JUST released
                _isPressing = false;
                _releasedTime = currentMillis;

                // Only count the release if it wasn't part of a long press
                if (!_longPressHandled) {
                    _clickCount++;
                }
            }
        }
    }

    // 2. Evaluate Long Press (Fires immediately while holding)
    if (_isPressing && !_longPressHandled) {
        if ((currentMillis - _pressedTime) > LONG_PRESS_DELAY) {
            _longPressHandled = true;
            _clickCount = 0; // Cancel any pending short clicks
            event = ButtonEvent::LONG_PRESS;
        }
    }

    // 3. Evaluate Single and Double Clicks (Fires after release timeout)
    if (!_isPressing && _clickCount > 0) {
        if ((currentMillis - _releasedTime) > DOUBLE_CLICK_DELAY) {
            if (_clickCount == 1) {
                event = ButtonEvent::SINGLE_CLICK;
            } else if (_clickCount >= 2) {
                event = ButtonEvent::DOUBLE_CLICK;
            }
            _clickCount = 0; // Reset after event is dispatched
        }
    }

    return event;
}