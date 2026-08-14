#pragma once
#include <mutex>
#include "buttonHandler.hpp"
#include <condition_variable>

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
    Settings& settings;
    DisplayHandler& displayHandler;
    std::mutex uiMutex;
    bool dirty = false;
    ButtonEvent lastButtonEvent;
    ButtonName lastButtonName;
    std::condition_variable cv;
};
