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

    _history = (_history << 1) | digitalRead(_pin);

    // read last 4 bits with bitmask (4 samples * 5ms = 20ms debounce).
    if (!_isDown && (_history & 0x0F) == 0x00) {
        _isDown = true;
        _pressStartTime = now;
        _longPressTriggered = false;
    }
    // 0x0F (binary 00001111) => newest 4 readings were all HIGH
    else if (_isDown && (_history & 0x0F) == 0x0F) {
        _isDown = false;
        _lastReleaseTime = now;

        if (!_longPressTriggered) {
            _clickCount++;
        }
    }

    // handle long press
    if (_isDown && !_longPressTriggered) {
        if (now - _pressStartTime >= LONG_PRESS_MS) {
            _longPressTriggered = true;
            _clickCount = 0; // Invalidate any clicks
            if (_longPressCb) _longPressCb();
        }
    }

    // handle clicks
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
