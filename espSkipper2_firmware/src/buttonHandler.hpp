#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <Arduino.h>

// Strongly typed enum for our button events
enum class ButtonEvent
{
    NONE,
    SINGLE_CLICK,
    DOUBLE_CLICK,
    LONG_PRESS
};

enum class ButtonName
{
    NONE,
    BUTTON0,
    BUTTON1,
    BUTTON2,
    BUTTON3
};

using ButtonCallback = std::function<void()>;

class ButtonHandler {
private:
    uint8_t _pin;
    
    // Shift register for noise filtering
    uint8_t _history;
    bool _isDown;
    
    unsigned long _pressStartTime;
    unsigned long _lastReleaseTime;
    
    uint8_t _clickCount;
    bool _longPressTriggered;

    const unsigned long LONG_PRESS_MS   = 600;
    const unsigned long MULTI_CLICK_MS  = 300;

    ButtonCallback _singleClickCb = nullptr;
    ButtonCallback _doubleClickCb = nullptr;
    ButtonCallback _longPressCb   = nullptr;

public:
    ButtonHandler(uint8_t pin);
    void begin();
    void process();
    void onSingleClick(ButtonCallback cb);
    void onDoubleClick(ButtonCallback cb);
    void onLongPress(ButtonCallback cb);
    void reload();
};

#endif