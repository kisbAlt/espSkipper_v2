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

    // 1. Shift current reading into history (active-low)
    _history = (_history << 1) | digitalRead(_pin);

    // 2. Detect stable state transitions using a BITMASK
    // We only care about the newest 4 bits (4 samples * 5ms = 20ms debounce).
    // 0x00 means the newest 4 readings were all LOW.
    if (!_isDown && (_history & 0x0F) == 0x00) {
        _isDown = true;
        _pressStartTime = now;
        _longPressTriggered = false;
    }
    // 0x0F (binary 00001111) means the newest 4 readings were all HIGH.
    else if (_isDown && (_history & 0x0F) == 0x0F) {
        _isDown = false;
        _lastReleaseTime = now;

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

    // 4. Handle Click Resolution
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
void ButtonHandler::reload()
{
}
