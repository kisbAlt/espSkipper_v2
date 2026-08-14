#pragma once
#include <iostream>
#include <variant>
#include <mutex>
#include <array>
#include <vector>
#include <thread>
#include <chrono>
#include <string>
#include <Arduino.h>
enum class SensorId : size_t
{
    GpsSpeed = 0,
    MaxGpsSpeed,
    MinGpsSpeed,
    AvgGpsSpeed,
    SatelliteCount,
    GpsCourse,
    WaterDepth,
    WindSpeed,
    WindDirection,
    Temperature,
    TiltPitch,
    TiltPitchMin,
    TiltPitchMax,
    TiltPitchAvg,
    TiltRoll,
    TiltRollMin,
    TiltRollMax,
    TiltRollAvg,
    DateTimeHour,
    DateTimeMinute,
    DateTimeSecond,
    DateTimeDay,
    DateTimeMonth,
    DateTimeYear,
    MAX_SENSORS
};

enum class SensorUnitEnum
{
    Kmph,
    Kilometer,
    Meter,
    Knots,
    Feet,
    Celsius,
    Degrees,
    BlankUnit
};

class SensorUnit {
    private:
    SensorUnitEnum sensorUnit;
    public:
    SensorUnit();
    SensorUnit(SensorUnitEnum sensorUnitEnum);
    const char* GetString();
};

struct SensorValueString {
    char text[32];
};
using SensorValue = std::variant<float, int, SensorValueString>;

class InstrumentDataModel
{
private:
    struct SensorRecord
    {
        SensorId id;
        SensorValue value;
        char *title;
        SensorUnit unit;
        bool isAverage = false;
        bool hasData = false;
        bool enabled = true;
        

        int dataCount = 0;

        void UpdateValue(SensorValue newValue);
    };

    std::array<SensorRecord, static_cast<size_t>(SensorId::MAX_SENSORS)> m_data;
    mutable std::mutex m_mutex;
    

public:
    
    int count = 0;
    void updateSensor(SensorId id, SensorValue val);
    void updateSensorIfLarger(SensorId id, SensorValue val);
    void updateSensorIfSmaller(SensorId id, SensorValue val);
    void addSensor(SensorId id, SensorValue val, SensorUnit unit, char *title, bool isAverage = false);
    bool isSensorEnabled(SensorId id) const;
    void disableSensor(SensorId id);
    void enableSensor(SensorId id);
    bool sensorHaveData(SensorId id) const;
    SensorValue getSensorValue(SensorId id) const;
    bool isSensorEnabledAndHaveData(SensorId id) const;
    std::vector<std::pair<SensorId, SensorValue>> getDisplaySnapshot() const;
    int getActiveSensorCount() const;
    inline static int fast_round_positive(float x) {
        return static_cast<int>(x + 0.5f);
    }

    template <typename Callback>
    void drawActiveSensors(Callback cb) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        int displayIndex = 0; // Tracks consecutive 0, 1, 2, 3 for your grid math

        for (size_t i = 0; i < m_data.size(); ++i)
        {
            if (m_data[i].hasData && m_data[i].enabled)
            {
                std::visit([](const auto& arg) {
                }, m_data[i].value);
                cb(displayIndex, m_data[i].title, m_data[i].value, m_data[i].unit, m_data[i].id);
                displayIndex++;
            }
        }
    }
};