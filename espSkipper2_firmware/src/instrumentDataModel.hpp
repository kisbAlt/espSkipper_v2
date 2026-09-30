#pragma once

#include <array>
#include <variant>
#include <vector>

#include "instrumentDataFormats.hpp"
#include "settingsHandler.hpp"
#include "stringTranslator.hpp"

class InstrumentDataModel
{
private:
    struct SensorRecord
    {
        SensorId id;
        SensorValue value;
        TextKey titleKey;
        SensorUnit unit;
        bool isAverage = false;
        bool hasData = false;
        int dataCount = 0;

        void UpdateValue(SensorValue newValue);
        SensorValue GetValueInUnit(SensorUnitEnum targetUnitEnum) const;
    };

    std::array<SensorRecord, static_cast<size_t>(SensorId::MAX_SENSORS)> data;
    const Settings &settings;
    SensorUnitEnum GetCurrentUnitForSensor(SensorId id) const;

public:
    InstrumentDataModel(const Settings &settings);
    void UpdateSensor(SensorId id, SensorValue val);
    void UpdateSensorIfLarger(SensorId id, SensorValue val);
    void UpdateSensorIfSmaller(SensorId id, SensorValue val);
    void AddSensor(SensorId id, SensorValue val, SensorUnit unit, TextKey titleKey, bool isAverage = false);
    bool IsSensorEnabled(SensorId id) const;
    bool SensorHasData(SensorId id) const;
    SensorValue GetSensorValue(SensorId id) const;
    SensorValue GetSensorValueInUnit(SensorId id, SensorUnitEnum targetUnitEnum) const;
    TextKey GetSensorTitleKey(SensorId id) const;
    bool IsSensorEnabledAndHasData(SensorId id) const;
    std::vector<std::pair<SensorId, SensorValue>> GetDisplaySnapshot() const;
    int GetActiveSensorCount() const;
    void Reload();
    void WriteCurrentUnitString(char* unitString, size_t bufferSize, SensorId id) const;
    SensorValue GetSensorValueCurrentUnit(SensorId id) const;

    inline static int MakePositive(int x)
    {
        return (x < 0) ? -x : x;
    }

    inline static int FastRoundPositive(float x)
    {
        return static_cast<int>(x + 0.5f);
    }

    template <typename Callback>
    void drawActiveSensors(Callback cb) const
    {
        int displayIndex = 0;

        for (size_t i = 0; i < data.size(); ++i)
        {
            if (IsSensorEnabledAndHasData(static_cast<SensorId>(i)))
            {
                std::visit([](const auto &arg) {}, data[i].value);
                SensorUnitEnum settingsUnit = GetCurrentUnitForSensor(data[i].id);
                cb(displayIndex, data[i].titleKey, data[i].GetValueInUnit(settingsUnit), GetUnitString(settingsUnit), data[i].id);
                displayIndex++;
            }
        }
    }
};