#pragma once
#include "pinout.hpp"
#include "displayUtils.hpp"
#include "instrumentDataModel.hpp"
#include "instrumentDataModel.hpp"

#define DISPLAY_WIDTH 300

class DisplayHandler
{
private:
    enum class DisplayLayout
    {
        ThreeColTwoRow
    };
    enum class StringConvertType
    {
        Number,
        TwoDigitNumber,
        ThreeDigitNumber,
    };
    int pageIndex = 0;
    OsptekBWR lcd;
    DisplayLayout currentLayout;
    InstrumentDataModel &dataModel;
    SensorId focusedSensor = SensorId::GpsSpeed;
    void convertValueToString(char *valueStr, int len, SensorValue value);
    int currentSensorDrawn() const;

public:
    DisplayHandler(InstrumentDataModel &dataModel);
    void init();
    void updateDisplay();
    void DrawLayout();
    void stepFocusedSensor();
    void nextDisplayPage();
    void ResetDisplay();
};