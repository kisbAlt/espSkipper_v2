#pragma once

#include <array>
#include <mutex>
#include <variant>
#include <vector>

#include "instrumentDataFormats.hpp"
#include "settingsHandler.hpp"

class InstrumentDataModel
{
private:
    struct SensorRecord
    {
        SensorId id;
        SensorValue value;
        const char *title;
        SensorUnit unit;
        bool isAverage = false;
        bool hasData = false;
        int dataCount = 0;

        void UpdateValue(SensorValue newValue);
    };

    std::array<SensorRecord, static_cast<size_t>(SensorId::MAX_SENSORS)> m_data;
    mutable std::mutex m_mutex;
    const Settings& m_settings;
    

public:
    InstrumentDataModel(const Settings& settings);
    int count = 0;
    void updateSensor(SensorId id, SensorValue val);
    void updateSensorIfLarger(SensorId id, SensorValue val);
    void updateSensorIfSmaller(SensorId id, SensorValue val);
    void addSensor(SensorId id, SensorValue val, SensorUnit unit, const char *title, bool isAverage = false);
    bool isSensorEnabled(SensorId id) const;
    bool sensorHaveData(SensorId id) const;
    SensorValue getSensorValue(SensorId id) const;
    bool isSensorEnabledAndHaveData(SensorId id) const;
    std::vector<std::pair<SensorId, SensorValue>> getDisplaySnapshot() const;
    int getActiveSensorCount() const;
    void reload();

    inline static int fast_round_positive(float x) {
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
                std::visit([](const auto& arg) {
                }, m_data[i].value);
                cb(displayIndex, m_data[i].title, m_data[i].value, m_data[i].unit, m_data[i].id);
                displayIndex++;
            }
        }
    }
};