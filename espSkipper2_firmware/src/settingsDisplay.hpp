#pragma once
#include "buttonHandler.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "display/displayHandler.hpp"

// forward declerations
class Settings;

class SettingsDisplay
{
public:
    SettingsDisplay(Settings& settings, DisplayHandler& displayHandler);
    void drawSettingsUI();
    void handleButtonPress(ButtonEvent btnEvent, ButtonName btnName);
private:
    bool processButtonEvent(ButtonEvent btnEvent, ButtonName btnName);
    SettingsDisplayStatus status;
    Settings& settings;
    DisplayHandler& displayHandler;
    // FreeRTOS Native Primitives
    SemaphoreHandle_t dataMutex;
    SemaphoreHandle_t wakeupSemaphore;
    ButtonEvent lastButtonEvent;
    ButtonName lastButtonName;
};
