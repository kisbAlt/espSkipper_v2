#include "buttonHandler.hpp"

ButtonHandler::ButtonHandler(uint8_t pin)
    : _pin(pin), _history(0xFF), _isDown(false),
          _pressStartTime(0), _lastReleaseTime(0),
          _clickCount(0), _longPressTriggered(false) {}

void ButtonHandler::begin() {
    pinMode(_pin, INPUT_PULLUP);
}


void ButtonHandler::process() {
    unsigned long now = millis();

    // 1. Shift current reading into history (active-low: 0 = pressed, 1 = released)
    _history = (_history << 1) | digitalRead(_pin);

    // 2. Detect stable state transitions
    // 0b11000000 -> Confirmed transition to Pressed (stable LOW for multiple samples)
    if (!_isDown && (_history == 0xC0 || _history == 0x80 || _history == 0x00)) {
        _isDown = true;
        _pressStartTime = now;
        _longPressTriggered = false;
    }
    // 0b00111111 -> Confirmed transition to Released (stable HIGH for multiple samples)
    else if (_isDown && (_history == 0x3F || _history == 0x7F || _history == 0xFF)) {
        _isDown = false;
        _lastReleaseTime = now;

        // Only count as a click if it wasn't already consumed by a long press
        if (!_longPressTriggered) {
            _clickCount++;
        }
    }

    // 3. Handle Long Press (fires while button is still held down)
    if (_isDown && !_longPressTriggered) {
        if (now - _pressStartTime >= LONG_PRESS_MS) {
            _longPressTriggered = true;
            _clickCount = 0; // Invalidate any clicks
            if (_longPressCb) _longPressCb();
        }
    }

    // 4. Handle Click Resolution (Single vs Double click after release timeout)
    if (!_isDown && _clickCount > 0) {
        if (now - _lastReleaseTime >= MULTI_CLICK_MS) {
            if (_clickCount == 1) {
                if (_singleClickCb) _singleClickCb();
            } else if (_clickCount >= 2) {
                if (_doubleClickCb) _doubleClickCb();
            }
            _clickCount = 0; // Reset
        }
    }
}

 void ButtonHandler::onSingleClick(ButtonCallback cb) { _singleClickCb = cb; }

void ButtonHandler::onDoubleClick(ButtonCallback cb) { _doubleClickCb = cb; }

void ButtonHandler::onLongPress(ButtonCallback cb)   { _longPressCb = cb; } 