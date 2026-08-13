#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <Arduino.h>

// Strongly typed enum for our button events
enum class ButtonEvent {
    NONE,
    SINGLE_CLICK,
    DOUBLE_CLICK,
    LONG_PRESS
};

enum class ButtonName {
    BUTTON0,
    BUTTON1,
    BUTTON2,
    BUTTON3
};

class ButtonHandler {
public:
    // Constructor takes the GPIO pin number
    ButtonHandler(uint8_t pin);
    
    // Initialize the button pin
    void begin();
    
    // Call this repeatedly in your loop()
    ButtonEvent update();

private:
    uint8_t _pin;
    bool _lastState;
    bool _currentState;
    bool _isPressing;

    unsigned long _lastDebounceTime;
    unsigned long _pressedTime;
    unsigned long _releasedTime;

    int _clickCount;
    bool _longPressHandled;

    // Timing thresholds (in milliseconds)
    static const unsigned long DEBOUNCE_DELAY = 50;
    static const unsigned long LONG_PRESS_DELAY = 800;
    static const unsigned long DOUBLE_CLICK_DELAY = 250;
};

#endif