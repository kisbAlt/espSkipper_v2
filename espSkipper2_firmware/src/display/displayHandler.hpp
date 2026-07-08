#pragma once
#include "pinout.hpp"
#include "displayUtils.hpp"
#include "instrumentDataModel.hpp"
#include "instrumentDataModel.hpp"

#define DISPLAY_WIDTH 300

class DisplayHandler {
    private:
    enum class DisplayLayout
    {
        ThreeColTwoRow
    };
    OsptekBWR lcd;
    DisplayLayout currentLayout;
    InstrumentDataModel& dataModel;
    SensorId focusedSensor = SensorId::GpsSpeed;
    void convertValueToString(char* valueStr, SensorValue value);
public:
    DisplayHandler(InstrumentDataModel& dataModel);
    void init();
    void updateDisplay();
    void DrawLayout();
};