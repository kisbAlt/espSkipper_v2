#pragma once

#include <array>
#include <mutex>
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

    std::array<SensorRecord, static_cast<size_t>(SensorId::MAX_SENSORS)> m_data;
    mutable std::mutex m_mutex;
    const Settings &m_settings;
    SensorUnitEnum getCurrentUnitForSensor(SensorId id) const;

public:
    InstrumentDataModel(const Settings &settings);
    int count = 0;
    void updateSensor(SensorId id, SensorValue val);
    void updateSensorIfLarger(SensorId id, SensorValue val);
    void updateSensorIfSmaller(SensorId id, SensorValue val);
    void addSensor(SensorId id, SensorValue val, SensorUnit unit, TextKey titleKey, bool isAverage = false);
    bool isSensorEnabled(SensorId id) const;
    bool sensorHaveData(SensorId id) const;
    SensorValue getSensorValue(SensorId id) const;
    SensorValue getSensorValueInUnit(SensorId id, SensorUnitEnum targetUnitEnum) const;
    TextKey getSensorTitleKey(SensorId id) const;
    bool isSensorEnabledAndHaveData(SensorId id) const;
    std::vector<std::pair<SensorId, SensorValue>> getDisplaySnapshot() const;
    int getActiveSensorCount() const;
    void reload();
    void writeCurrentUnitString(char* unitString, size_t bufferSize, SensorId id) const;
    SensorValue getSensorValueCurrentUnit(SensorId id) const;

    inline static int fast_round_positive(float x)
    {
        return static_cast<int>(x + 0.5f);
    }

    template <typename Callback>
    void drawActiveSensors(Callback cb) const
    {
        int displayIndex = 0; // Tracks consecutive 0, 1, 2, 3 for your grid math

        for (size_t i = 0; i < m_data.size(); ++i)
        {
            if (isSensorEnabledAndHaveData(static_cast<SensorId>(i)))
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                std::visit([](const auto &arg) {}, m_data[i].value);
                SensorUnitEnum settingsUnit = getCurrentUnitForSensor(m_data[i].id);
                cb(displayIndex, m_data[i].titleKey, m_data[i].GetValueInUnit(settingsUnit), GetUnitString(settingsUnit), m_data[i].id);
                displayIndex++;
            }
        }
    }
};