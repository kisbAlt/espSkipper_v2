#pragma once
#include "buttonHandler.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "display/displayHandler.hpp"

// forward declerations
class Settings;

class SettingsDisplay
{
public:
    SettingsDisplay(Settings& settings, DisplayHandler& displayHandler);
    void DrawSettingsUI(QueueHandle_t buttonEventQueue);
private:
    bool ProcessButtonEvent(ButtonEvent btnEvent, ButtonName btnName);
    SettingsDisplayStatus status;
    Settings& settings;
    DisplayHandler& displayHandler;
};
