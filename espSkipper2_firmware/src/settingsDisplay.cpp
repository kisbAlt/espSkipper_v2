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
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
    {
        lastButtonEvent = btnEvent;
        lastButtonName = btnName;
        xSemaphoreGive(dataMutex);
    }

    // Wake up the display thread! (Equivalent to cv.notify_one)
    xSemaphoreGive(wakeupSemaphore);
}

bool SettingsDisplay::processButtonEvent(ButtonEvent btnEvent, ButtonName btnName)
{
    switch (btnEvent)
    {
    case ButtonEvent::SINGLE_CLICK:
        if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
        {
            switch (btnName)
            {
            case ButtonName::BUTTON0:
                status.isEditing = status.isEditing ? false : true;
                break;
            case ButtonName::BUTTON1:
                if (status.isEditing)
                {
                    settings.setPreviousValue(status.currentSettingIndex);
                }
                else
                {
                    status.currentSettingIndex = status.currentSettingIndex > 0 ? status.currentSettingIndex - 1 : settings.getSettingsCount() - 1;
                }

                break;
            case ButtonName::BUTTON2:
                if (status.isEditing)
                {
                    settings.setNextValue(status.currentSettingIndex);
                }
                else
                {
                    status.currentSettingIndex = (status.currentSettingIndex + 1) % settings.getSettingsCount();
                }

                break;
            case ButtonName::BUTTON3:
                xSemaphoreGive(dataMutex);
                return false;
                break;
            }
            xSemaphoreGive(dataMutex);
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

    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
    {
        lastButtonEvent = ButtonEvent::NONE;
        lastButtonName = ButtonName::NONE;
        xSemaphoreGive(dataMutex); 
    }

    Serial.println("drawSettingsUI1");
    displayHandler.DrawSettingsPage(status);
    xQueueReset(wakeupSemaphore);

    while (running)
    {
        ButtonEvent currButtonEvent = ButtonEvent::NONE;
        ButtonName currButtonName = ButtonName::NONE;

        xSemaphoreTake(wakeupSemaphore, portMAX_DELAY);
        if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
        {
            Serial.println("drawSettingsUI2");
            currButtonEvent = lastButtonEvent;
            currButtonName = lastButtonName;
            lastButtonEvent = ButtonEvent::NONE;
            lastButtonName = ButtonName::NONE;
            xSemaphoreGive(dataMutex);
        }
        
        running = processButtonEvent(currButtonEvent, currButtonName);

        if (running) 
        {
            displayHandler.DrawSettingsPage(status);
        }
    }
}
