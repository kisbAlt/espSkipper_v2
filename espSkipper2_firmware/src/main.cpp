#include <Arduino.h>
#include "display/displayHandler.hpp"
#include "accelerometerHandler.hpp"
#include "gpsHandler.hpp"
#include "Wire.h"
#include "SPI.h"
#include "display/displayUtils.hpp"
#include "instrumentDataModel.hpp"
#include "stringTranslator.hpp"
#include "buttonHandler.hpp"
#include "ledHandler.hpp"
#include "settingsHandler.hpp"
#include "settingsDisplay.hpp"
#include "depthHandler.hpp"
#include "windHandler.hpp"

Settings settings{};

InstrumentDataModel instrumentDataModel(settings);
DisplayHandler displayHandler(instrumentDataModel, settings);
AccelerometerHandler accelerometerHandler(instrumentDataModel);
GpsHandler gpsHandler(instrumentDataModel);
SettingsDisplay settingsDisplay(settings, displayHandler);
DepthHandler depthHandler(settings, instrumentDataModel);
WindHandler windHandler(instrumentDataModel);

ButtonHandler btn0(8);
ButtonHandler btn1(17);
ButtonHandler btn2(14);
ButtonHandler btn3(21);
QueueHandle_t buttonEventQueue;

void postButtonEvent(const ButtonName btnName, const ButtonEvent event)
{
    const ButtonMessage message{btnName, event};
    xQueueSend(buttonEventQueue, &message, 0);
}

void handleButtonEvent(const ButtonName btnName, const ButtonEvent event);

void globalButtonTask(void* arg) {
    while (true) {
        btn0.Process();
        btn1.Process();
        btn2.Process();
        btn3.Process();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

LedHandler btnLed(3, []() { return settings.GetValue<bool>(SettingsKey::BtnLedEnabled); }, []() { return settings.GetValue<uint8_t>(SettingsKey::BtnBrightness); }, 0);
LedHandler lcdLed(18, []() { return settings.GetValue<bool>(SettingsKey::LCDLedEnabled); }, []() { return settings.GetValue<uint8_t>(SettingsKey::LCDBrightness); }, 1);

enum class AppState
{
    MAIN_SCREEN,
    SETTINGS_SCREEN
};
AppState currentState = AppState::MAIN_SCREEN;
void reloadAll() {
    btnLed.Reload();
    lcdLed.Reload();
    gpsHandler.Reload();
    accelerometerHandler.Reload();
    instrumentDataModel.Reload();
    windHandler.Reload();
    Translator::SetLanguage(settings.GetValue<uint8_t>(SettingsKey::Language));
}

void sensorDisplayTask(void *pvParameters)
{
    //displayHandler.ResetDisplay();
    for (;;)
    {
        ButtonMessage buttonMessage;
        while (xQueueReceive(buttonEventQueue, &buttonMessage, 0) == pdTRUE)
        {
            handleButtonEvent(buttonMessage.name, buttonMessage.event);
            if (currentState == AppState::SETTINGS_SCREEN)
                break;
        }

        if (currentState == AppState::SETTINGS_SCREEN)
        {
            settingsDisplay.DrawSettingsUI(buttonEventQueue);
            reloadAll();
            currentState = AppState::MAIN_SCREEN;
        }
            displayHandler.UpdateDisplay();
            accelerometerHandler.ReadAccelerometerData();
            gpsHandler.UpdateGpsData();
            depthHandler.ReadPacket();
            windHandler.UpdateWindData();
        vTaskDelay(pdMS_TO_TICKS(settings.GetValue<int>(SettingsKey::DisplayScreenUpdate)));
    }
}

void handleButtonEvent(const ButtonName btnName, const ButtonEvent event)
{
    switch (event)
    {
    case ButtonEvent::SINGLE_CLICK:
        switch (btnName)
        {
        case ButtonName::BUTTON0:
            displayHandler.StepFocusedSensor();
            break;
        case ButtonName::BUTTON1:
            displayHandler.NextDisplayPage();
            break;
        case ButtonName::BUTTON3:
            currentState = AppState::SETTINGS_SCREEN;
            break;
        }
        break;
    case ButtonEvent::DOUBLE_CLICK:
        break;
    case ButtonEvent::LONG_PRESS:
        if (btnName == ButtonName::BUTTON0)
        {
            const bool newValue = !settings.GetValue<bool>(SettingsKey::LCDLedEnabled);
            settings.SetValue(SettingsKey::BtnLedEnabled, newValue);
            settings.SetValue(SettingsKey::LCDLedEnabled, newValue);
            btnLed.Reload();
            lcdLed.Reload();
        }
        break;
    case ButtonEvent::NONE:
    default:
        break;
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000); // Give serial a moment to wake up
    settings.Begin();

    Serial.println("Init PINs and SPI");
    pinMode(LIS3DH_CS, OUTPUT);
    digitalWrite(LIS3DH_CS, HIGH);
    pinMode(LCD_CS, OUTPUT);
    digitalWrite(LCD_CS, HIGH);

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    Serial.println("Init Display");
    displayHandler.Init();
    Serial.println("Init Accelerometer");
    accelerometerHandler.Init();
    Serial.println("Init GPS");
    gpsHandler.Init();
    depthHandler.Begin();
    windHandler.Init();

    // Initialize buttons
    btn0.Begin();
    btn0.OnDoubleClick([]()
                                             { postButtonEvent(ButtonName::BUTTON0, ButtonEvent::DOUBLE_CLICK); });
    btn0.OnLongPress([]()
                                         { postButtonEvent(ButtonName::BUTTON0, ButtonEvent::LONG_PRESS); });
    btn0.OnSingleClick([]()
                                             { postButtonEvent(ButtonName::BUTTON0, ButtonEvent::SINGLE_CLICK); });
    btn1.Begin();
    btn1.OnDoubleClick([]()
                                             { postButtonEvent(ButtonName::BUTTON1, ButtonEvent::DOUBLE_CLICK); });
    btn1.OnLongPress([]()
                                         { postButtonEvent(ButtonName::BUTTON1, ButtonEvent::LONG_PRESS); });
    btn1.OnSingleClick([]()
                                             { postButtonEvent(ButtonName::BUTTON1, ButtonEvent::SINGLE_CLICK); });
    btn2.Begin();
    btn2.OnDoubleClick([]()
                                             { postButtonEvent(ButtonName::BUTTON2, ButtonEvent::DOUBLE_CLICK); });
    btn2.OnLongPress([]()
                                         { postButtonEvent(ButtonName::BUTTON2, ButtonEvent::LONG_PRESS); });
    btn2.OnSingleClick([]()
                                             { postButtonEvent(ButtonName::BUTTON2, ButtonEvent::SINGLE_CLICK); });
    btn3.Begin();
    btn3.OnDoubleClick([]()
                                             { postButtonEvent(ButtonName::BUTTON3, ButtonEvent::DOUBLE_CLICK); });
    btn3.OnLongPress([]()
                                         { postButtonEvent(ButtonName::BUTTON3, ButtonEvent::LONG_PRESS); });
    btn3.OnSingleClick([]()
                                             { postButtonEvent(ButtonName::BUTTON3, ButtonEvent::SINGLE_CLICK); });

        buttonEventQueue = xQueueCreate(16, sizeof(ButtonMessage));
        if (buttonEventQueue == nullptr)
        {
                Serial.println("Failed to create button event queue");
                return;
        }
    
    xTaskCreatePinnedToCore(
        sensorDisplayTask,
        "SensorTask",
        8192,
        NULL,
        1,
        NULL,
        0
    );

    xTaskCreatePinnedToCore(
        globalButtonTask,
        "ButtonManager",
        2048,
        NULL,
        2,
        NULL,
        1
    );

    btnLed.Begin();
    lcdLed.Begin();

    Serial.println("Setup complete, resetting display.");
}

void loop()
{

    vTaskDelay(pdMS_TO_TICKS(5000));
}