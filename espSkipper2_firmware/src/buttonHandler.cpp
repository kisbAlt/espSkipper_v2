#include "buttonHandler.hpp"

ButtonHandler::ButtonHandler(uint8_t pin)
        : pin(pin), history(0xFF), isDown(false),
            pressStartTime(0), lastReleaseTime(0),
            clickCount(0), longPressTriggered(false) {}

void ButtonHandler::Begin() {
    pinMode(pin, INPUT_PULLUP);
}


void ButtonHandler::Process() {
    unsigned long now = millis();

    history = (history << 1) | digitalRead(pin);

    // read last 4 bits with bitmask (4 samples * 5ms = 20ms debounce).
    if (!isDown && (history & 0x0F) == 0x00) {
        isDown = true;
        pressStartTime = now;
        longPressTriggered = false;
    }
    // 0x0F (binary 00001111) => newest 4 readings were all HIGH
    else if (isDown && (history & 0x0F) == 0x0F) {
        isDown = false;
        lastReleaseTime = now;

        if (!longPressTriggered) {
            clickCount++;
        }
    }

    // handle long press
    if (isDown && !longPressTriggered) {
        if (now - pressStartTime >= LONG_PRESS_MS) {
            longPressTriggered = true;
            clickCount = 0; // Invalidate any clicks
            if (longPressCallback) longPressCallback();
        }
    }

    // handle clicks
    if (!isDown && clickCount > 0) {
        if (now - lastReleaseTime >= MULTI_CLICK_MS) {
            if (clickCount == 1) {
                if (singleClickCallback) singleClickCallback();
            } else if (clickCount >= 2) {
                if (doubleClickCallback) doubleClickCallback();
            }
            clickCount = 0; // Reset
        }
    }
}

 void ButtonHandler::OnSingleClick(ButtonCallback cb) { singleClickCallback = cb; }

void ButtonHandler::OnDoubleClick(ButtonCallback cb) { doubleClickCallback = cb; }

void ButtonHandler::OnLongPress(ButtonCallback cb)   { longPressCallback = cb; }
void ButtonHandler::Reload()
{
}
