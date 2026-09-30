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
void globalButtonTask(void* arg) {
    while (true) {
        btn0.process();
        btn1.process();
        btn2.process();
        btn3.process();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

TaskHandle_t SensorTaskHandle;

LedHandler btnLed(3, []() { return settings.getValue<bool>(SettingsKey::BtnLedEnabled); }, []() { return settings.getValue<uint8_t>(SettingsKey::BtnBrightness); }, 0);
LedHandler lcdLed(18, []() { return settings.getValue<bool>(SettingsKey::LCDLedEnabled); }, []() { return settings.getValue<uint8_t>(SettingsKey::LCDBrightness); }, 1);

enum class AppState
{
    MAIN_SCREEN,
    SETTINGS_SCREEN
};
AppState currentState = AppState::MAIN_SCREEN;
void reloadAll() {
    btnLed.reload();
    lcdLed.reload();
    gpsHandler.reload();
    accelerometerHandler.reload();
    instrumentDataModel.reload();
    windHandler.reload();
    Translator::setLanguage(settings.getValue<uint8_t>(SettingsKey::Language));
}

void sensorDisplayTask(void *pvParameters)
{
    //displayHandler.ResetDisplay();
    for (;;)
    {
        if (currentState == AppState::SETTINGS_SCREEN)
        {
            settingsDisplay.drawSettingsUI();
            reloadAll();
            currentState = AppState::MAIN_SCREEN;
        }
        displayHandler.updateDisplay();
        accelerometerHandler.readAccelerometerData();
        gpsHandler.updateGpsData();
        depthHandler.ReadPacket();
        windHandler.updateWindData();
        vTaskDelay(pdMS_TO_TICKS(settings.getValue<int>(SettingsKey::DisplayScreenUpdate)));
    }
}

void handleButtonEvent(const ButtonName btnName, const ButtonEvent event)
{
    if (event != ButtonEvent::NONE && currentState == AppState::SETTINGS_SCREEN)
    {
        settingsDisplay.handleButtonPress(event, btnName);
        return;
    }
    switch (event)
    {
    case ButtonEvent::SINGLE_CLICK:
        switch (btnName)
        {
        case ButtonName::BUTTON0:
            displayHandler.stepFocusedSensor();
            break;
        case ButtonName::BUTTON1:
            displayHandler.nextDisplayPage();
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
            const bool newValue = !settings.getValue<bool>(SettingsKey::LCDLedEnabled);
            settings.setValue(SettingsKey::BtnLedEnabled, newValue);
            settings.setValue(SettingsKey::LCDLedEnabled, newValue);
            btnLed.reload();
            lcdLed.reload();
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
    settings.begin();

    Serial.println("Init PINs and SPI");
    pinMode(LIS3DH_CS, OUTPUT);
    digitalWrite(LIS3DH_CS, HIGH);
    pinMode(LCD_CS, OUTPUT);
    digitalWrite(LCD_CS, HIGH);

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    Serial.println("Init Display");
    displayHandler.init();
    Serial.println("Init Accelerometer");
    accelerometerHandler.init();
    Serial.println("Init GPS");
    gpsHandler.init();
    depthHandler.begin();
    windHandler.init();

    // Initialize buttons
    btn0.begin();
    btn0.onDoubleClick([]()
                       { handleButtonEvent(ButtonName::BUTTON0, ButtonEvent::DOUBLE_CLICK); });
    btn0.onLongPress([]()
                     { handleButtonEvent(ButtonName::BUTTON0, ButtonEvent::LONG_PRESS); });
    btn0.onSingleClick([]()
                       { handleButtonEvent(ButtonName::BUTTON0, ButtonEvent::SINGLE_CLICK); });
    btn1.begin();
    btn1.onDoubleClick([]()
                       { handleButtonEvent(ButtonName::BUTTON1, ButtonEvent::DOUBLE_CLICK); });
    btn1.onLongPress([]()
                     { handleButtonEvent(ButtonName::BUTTON1, ButtonEvent::LONG_PRESS); });
    btn1.onSingleClick([]()
                       { handleButtonEvent(ButtonName::BUTTON1, ButtonEvent::SINGLE_CLICK); });
    btn2.begin();
    btn2.onDoubleClick([]()
                       { handleButtonEvent(ButtonName::BUTTON2, ButtonEvent::DOUBLE_CLICK); });
    btn2.onLongPress([]()
                     { handleButtonEvent(ButtonName::BUTTON2, ButtonEvent::LONG_PRESS); });
    btn2.onSingleClick([]()
                       { handleButtonEvent(ButtonName::BUTTON2, ButtonEvent::SINGLE_CLICK); });
    btn3.begin();
    btn3.onDoubleClick([]()
                       { handleButtonEvent(ButtonName::BUTTON3, ButtonEvent::DOUBLE_CLICK); });
    btn3.onLongPress([]()
                     { handleButtonEvent(ButtonName::BUTTON3, ButtonEvent::LONG_PRESS); });
    btn3.onSingleClick([]()
                       { handleButtonEvent(ButtonName::BUTTON3, ButtonEvent::SINGLE_CLICK); });
    
    xTaskCreatePinnedToCore(
        sensorDisplayTask,
        "SensorTask",
        8192,
        NULL,
        1,
        &SensorTaskHandle,
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

    btnLed.begin();
    lcdLed.begin();

    Serial.println("Setup complete, resetting display.");
}

void loop()
{

    vTaskDelay(pdMS_TO_TICKS(5000));
}