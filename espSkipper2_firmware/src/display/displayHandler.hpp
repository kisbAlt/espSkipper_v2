#pragma once
#include "pinout.hpp"
#include "displayUtils.hpp"
#include "instrumentDataModel.hpp"
#include "instrumentDataModel.hpp"

class DisplayHandler {
    private:
    OsptekBWR lcd;
    DisplayLayout currentLayout;
    InstrumentDataModel& dataModel;
    void convertValueToString(char* valueStr, SensorValue value);
public:
    DisplayHandler(InstrumentDataModel& dataModel);
    void init();
    void updateDisplay(const DisplayDataEntity datapoints[10], const int dataCount);
    void DrawLayout();
};