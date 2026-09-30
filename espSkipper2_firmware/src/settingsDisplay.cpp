#include "settingsDisplay.hpp"
#include "settingsHandler.hpp"
#include "display/displayHandler.hpp"

SettingsDisplay::SettingsDisplay(Settings &settings, DisplayHandler &displayHandler)
    : settings(settings), displayHandler(displayHandler)
{
}

bool SettingsDisplay::ProcessButtonEvent(ButtonEvent btnEvent, ButtonName btnName)
{
    switch (btnEvent)
    {
    case ButtonEvent::SINGLE_CLICK:
        switch (btnName)
        {
        case ButtonName::BUTTON0:
            if (settings.GetSettingDef(status.currentSettingIndex).key == SettingsKey::DisabledSensors)
            {
                status.mode = (status.mode == SettingDisplayMode::SettingsList) ? SettingDisplayMode::SensorList : SettingDisplayMode::SettingsList;
                status.currentSettingIndex = 0;
            }
            else
                status.isEditing = !status.isEditing;
            break;
        case ButtonName::BUTTON1:
            if (status.mode == SettingDisplayMode::SensorList)
            {
                if (status.isEditing)
                {
                    if (settings.IsSensorEnabled(static_cast<SensorId>(status.currentSettingIndex)))
                        settings.DisableSensor(static_cast<SensorId>(status.currentSettingIndex));
                    else
                        settings.EnableSensor(static_cast<SensorId>(status.currentSettingIndex));
                }
                else
                    status.currentSettingIndex = status.currentSettingIndex > 0 ? status.currentSettingIndex - 1 : static_cast<int>(SensorId::MAX_SENSORS) - 1;
            }
            else if (status.isEditing)
                settings.SetPreviousValue(status.currentSettingIndex);
            else
                status.currentSettingIndex = status.currentSettingIndex > 0 ? status.currentSettingIndex - 1 : settings.GetSettingsCount() - 1;
            break;
        case ButtonName::BUTTON2:
            if (status.mode == SettingDisplayMode::SensorList)
            {
                if (status.isEditing)
                {
                    if (settings.IsSensorEnabled(static_cast<SensorId>(status.currentSettingIndex)))
                        settings.DisableSensor(static_cast<SensorId>(status.currentSettingIndex));
                    else
                        settings.EnableSensor(static_cast<SensorId>(status.currentSettingIndex));
                }
                else
                    status.currentSettingIndex = (status.currentSettingIndex + 1) % static_cast<int>(SensorId::MAX_SENSORS);
            }
            else if (status.isEditing)
                settings.SetNextValue(status.currentSettingIndex);
            else
                status.currentSettingIndex = (status.currentSettingIndex + 1) % settings.GetSettingsCount();
            break;
        case ButtonName::BUTTON3:
            if (status.mode == SettingDisplayMode::SensorList)
            {
                status.isEditing = false;
                status.currentSettingIndex = 0;
                status.mode = SettingDisplayMode::SettingsList;
            }
            else
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

void SettingsDisplay::DrawSettingsUI(QueueHandle_t buttonEventQueue)
{
    Serial.println("drawSettingsUI1");
    displayHandler.DrawSettingsPage(status);

    ButtonMessage buttonMessage;
    while (xQueueReceive(buttonEventQueue, &buttonMessage, portMAX_DELAY) == pdTRUE)
    {
        if (!ProcessButtonEvent(buttonMessage.event, buttonMessage.name))
            break;
        displayHandler.DrawSettingsPage(status);
    }
}
