#pragma once
#include <Arduino.h>
#include "instrumentDataModel.hpp"
#include "settingsHandler.hpp"

class DepthHandler
{
private:
    InstrumentDataModel &dataModel;

public:
    DepthHandler(const Settings &settings, InstrumentDataModel &dataModel);
    bool ReadPacket();
    void Begin();
};