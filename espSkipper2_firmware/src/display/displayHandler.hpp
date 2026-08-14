#pragma once
#include "pinout.hpp"
#include "displayUtils.hpp"
#include "instrumentDataModel.hpp"
#include "instrumentDataModel.hpp"

#define DISPLAY_WIDTH 300


enum class UpdatePage {
    MAIN_SCREEN,
    SETTINGS_SCREEN
};

struct SettingsDisplayStatus {
    uint8_t currentSettingIndex = 0;
    bool isEditing = false;
};

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
    const Settings& settings;

    SensorId focusedSensor = SensorId::GpsSpeed;
    void convertValueToString(char *valueStr, int len, SensorValue value);
    int currentSensorDrawn() const;
    
public:
    DisplayHandler(InstrumentDataModel &dataModel, const Settings& settings);
    void init();
    void updateDisplay();
    void DrawLayout();
    void stepFocusedSensor();
    void nextDisplayPage();
    void ResetDisplay();
    void DrawSettingsPage(const SettingsDisplayStatus &status);
};