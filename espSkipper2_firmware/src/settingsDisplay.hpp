#pragma once
#include "buttonHandler.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// forward declerations
class Settings;
class DisplayHandler;

class SettingsDisplay
{
public:
    SettingsDisplay(Settings& settings, DisplayHandler& displayHandler);
    void drawSettingsUI();
    void handleButtonPress(ButtonEvent btnEvent, ButtonName btnName);
private:
    bool processButtonEvent(ButtonEvent btnEvent, ButtonName btnName);
    Settings& settings;
    DisplayHandler& displayHandler;
// FreeRTOS Native Primitives
    SemaphoreHandle_t dataMutex;
    SemaphoreHandle_t wakeupSemaphore;
    ButtonEvent lastButtonEvent;
    ButtonName lastButtonName;
};
