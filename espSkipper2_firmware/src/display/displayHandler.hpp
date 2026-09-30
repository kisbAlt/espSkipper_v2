#pragma once
#include "pinout.hpp"
#include "displayUtils.hpp"
#include "instrumentDataModel.hpp"


enum class UpdatePage {
    MAIN_SCREEN,
    SETTINGS_SCREEN
};

enum class SettingDisplayMode {
    SettingsList,
    SensorList
};

struct SettingsDisplayStatus {
    uint8_t currentSettingIndex = 0;
    bool isEditing = false;
    SettingDisplayMode mode = SettingDisplayMode::SettingsList;
};

class DisplayHandler
{
private:
    enum class DisplayLayout
    {
        ThreeColTwoRow,
        WindPage,
        MAX_LAYOUTS
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
    void ConvertValueToString(char *valueStr, int len, SensorValue value, SensorId id);
    int CurrentSensorDrawn() const;
    
public:
    DisplayHandler(InstrumentDataModel &dataModel, const Settings& settings);
    void Init();
    void UpdateDisplay();
    void DrawLayout();
    void StepFocusedSensor();
    void NextDisplayPage();
    void ResetDisplay();
    void DrawSettingsPage(const SettingsDisplayStatus &status);
};