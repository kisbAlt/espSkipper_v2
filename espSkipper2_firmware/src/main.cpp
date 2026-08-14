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

Settings settings;

InstrumentDataModel instrumentDataModel(settings);
DisplayHandler displayHandler(instrumentDataModel);
AccelerometerHandler accelerometerHandler(instrumentDataModel);
GpsHandler gpsHandler(instrumentDataModel);

ButtonHandler btn1(8);
ButtonHandler btn2(17);
ButtonHandler btn3(14);
ButtonHandler btn4(21);
TaskHandle_t SensorTaskHandle;

LedHandler btnLed(3 , settings);

// 1. Define the task that will run on Core 0
void sensorDisplayTask(void *pvParameters) {
    displayHandler.ResetDisplay();
    // FreeRTOS tasks need an infinite loop
    for(;;) {
        // Heavy blocking operations go here
        displayHandler.updateDisplay();
        accelerometerHandler.readAccelerometerData();
        gpsHandler.updateGpsData();
        
        // vTaskDelay is the FreeRTOS equivalent of delay().
        // It tells Core 0 to wait for 500ms before running the loop again, 
        // yielding the core to background WiFi/Bluetooth tasks in the meantime.
        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Give serial a moment to wake up

  Translator::setLanguage(0);

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

  // Initialize buttons
  btn1.begin();
  btn2.begin();
  btn3.begin();
  btn4.begin();
  // 2. Launch the sensor task and pin it to Core 0
  xTaskCreatePinnedToCore(
      sensorDisplayTask,   // The function we just wrote above
      "SensorTask",        // A name for debugging
      8192,                // Stack size (8KB is generous, good for displays/GPS)
      NULL,                // Task input parameter (not needed here)
      1,                   // Task priority (1 is standard)
      &SensorTaskHandle,   // Task handle
      0                    // Pin this specific task to Core 0
  );

  btnLed.begin();
  btnLed.off();

  Serial.println("Setup complete, resetting display.");

  
}

void handleButtonEvent(const ButtonName btnName, ButtonEvent event) {
    switch(event) {
        case ButtonEvent::SINGLE_CLICK:
            switch(btnName) {
                case ButtonName::BUTTON0:
                    displayHandler.stepFocusedSensor();
                    break;
                case ButtonName::BUTTON1:
                    displayHandler.nextDisplayPage();
            }
            Serial.printf("%d: Single Click\n", btnName);
            break;
        case ButtonEvent::DOUBLE_CLICK:
            Serial.printf("%d: Double Click\n", btnName);
            break;
        case ButtonEvent::LONG_PRESS:
            if(btnName == ButtonName::BUTTON0) {
              btnLed.toggle();
            }
            Serial.printf("%d: Long Press\n", btnName);
            break;
        case ButtonEvent::NONE:
        default:
            break; // Do nothing
    }
}

void loop() {
    // 3. The main loop runs on Core 1 by default.
    // It will now exclusively handle buttons without waiting for sensors.
    handleButtonEvent(ButtonName::BUTTON0, btn1.update());
    handleButtonEvent(ButtonName::BUTTON1, btn2.update());
    handleButtonEvent(ButtonName::BUTTON2, btn3.update());
    handleButtonEvent(ButtonName::BUTTON3, btn4.update());
    
    // A tiny delay prevents the FreeRTOS watchdog timer from crashing 
    // the core for hogging 100% of the CPU.
    vTaskDelay(pdMS_TO_TICKS(5)); 
}