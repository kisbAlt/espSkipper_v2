#pragma once
#include <Arduino.h>
#include "instrumentDataModel.hpp"
#include "settingsHandler.hpp"

class DepthHandler
{
private:
    const Settings &settings;
    InstrumentDataModel &dataModel;
    //static void uartTaskRoute(void* pvParameters); // FreeRTOS Task

public:
    //void startBackgroundTask();
    DepthHandler(const Settings &settings, InstrumentDataModel &dataModel);
    bool ReadPacket();
    void begin();
    u_int16_t lastDepth = 0;
    float temperature, driveVoltage;
    uint16_t *samplesOut;
};