#include "settingsDisplay.hpp"
#include "settingsHandler.hpp"
#include "display/displayHandler.hpp"

SettingsDisplay::SettingsDisplay(Settings &settings, DisplayHandler &displayHandler)
    : settings(settings), displayHandler(displayHandler)
{
    dataMutex = xSemaphoreCreateMutex();
    wakeupSemaphore = xSemaphoreCreateBinary();
}

void SettingsDisplay::handleButtonPress(ButtonEvent btnEvent, ButtonName btnName)
{
    // Safely write the button data
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
        lastButtonEvent = btnEvent;
        lastButtonName = btnName;
        xSemaphoreGive(dataMutex);
    }
    
    // Wake up the display thread! (Equivalent to cv.notify_one)
    xSemaphoreGive(wakeupSemaphore);
}

bool SettingsDisplay::processButtonEvent(ButtonEvent btnEvent, ButtonName btnName)
{
    switch(btnEvent) {
        case ButtonEvent::SINGLE_CLICK:
            switch(btnName) {
                case ButtonName::BUTTON0:
                    break;
                case ButtonName::BUTTON1:
                    break;
                case ButtonName::BUTTON2:
                    break;
                case ButtonName::BUTTON3:
                    return false;
                    break;
            }
            Serial.printf("%d: Single Click\n", btnName);
            break;
        case ButtonEvent::DOUBLE_CLICK:
            break;
        case ButtonEvent::LONG_PRESS:
            break;
        case ButtonEvent::NONE:
        default:
            break;
    }
    return true;
}

void SettingsDisplay::drawSettingsUI()
{
    bool running = true;
    // Force an initial draw when the screen first loads
    displayHandler.updateDisplay(UpdatePage::SETTINGS_SCREEN);

    // Make sure the semaphore is empty before we start waiting
    xQueueReset(wakeupSemaphore); 

    while (running) {
        ButtonEvent currButtonEvent;
        ButtonName currButtonName;

        xSemaphoreTake(wakeupSemaphore, portMAX_DELAY);

        // We woke up! Safely grab the button data.
        if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
            currButtonEvent = lastButtonEvent;
            currButtonName = lastButtonName;
            xSemaphoreGive(dataMutex);
        }

        running = processButtonEvent(currButtonEvent, currButtonName);        

        displayHandler.updateDisplay(UpdatePage::SETTINGS_SCREEN);
    }
}
