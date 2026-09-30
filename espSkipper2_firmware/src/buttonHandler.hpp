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

struct ButtonMessage
{
    ButtonName name;
    ButtonEvent event;
};

using ButtonCallback = std::function<void()>;

class ButtonHandler {
private:
    uint8_t pin;
    
    // Shift register for noise filtering
    uint8_t history;
    bool isDown;
    
    unsigned long pressStartTime;
    unsigned long lastReleaseTime;
    
    uint8_t clickCount;
    bool longPressTriggered;

    const unsigned long LONG_PRESS_MS   = 600;
    const unsigned long MULTI_CLICK_MS  = 300;

    ButtonCallback singleClickCallback = nullptr;
    ButtonCallback doubleClickCallback = nullptr;
    ButtonCallback longPressCallback = nullptr;

public:
    ButtonHandler(uint8_t pin);
    void Begin();
    void Process();
    void OnSingleClick(ButtonCallback cb);
    void OnDoubleClick(ButtonCallback cb);
    void OnLongPress(ButtonCallback cb);
    void Reload();
};

#endif