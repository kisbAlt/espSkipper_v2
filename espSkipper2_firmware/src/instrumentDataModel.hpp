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
    SatelliteCount,
    WaterDepth,
    WindSpeed,
    WindDirection,
    Temperature,
    TiltPitch,
    TiltRoll,
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

using SensorValue = std::variant<float, int, char*>;

class InstrumentDataModel
{
private:
    struct SensorRecord
    {
        SensorId id;
        SensorValue value;
        char *title;
        SensorUnit unit;
        bool hasData = false;
    };

    std::array<SensorRecord, static_cast<size_t>(SensorId::MAX_SENSORS)> m_data;
    mutable std::mutex m_mutex;
    

public:
    
    int count = 0;
    void updateSensor(SensorId id, SensorValue val);
    void addSensor(SensorId id, SensorValue val, SensorUnit unit, char *title);
    std::vector<std::pair<SensorId, SensorValue>> getDisplaySnapshot() const;

    template <typename Callback>
    void drawActiveSensors(Callback cb) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        int displayIndex = 0; // Tracks consecutive 0, 1, 2, 3 for your grid math

        for (size_t i = 0; i < m_data.size(); ++i)
        {
            if (m_data[i].hasData)
            {
                Serial.print("drawActiveSensors hasdata");
                std::visit([](const auto& arg) {
                    Serial.println(arg);
                }, m_data[i].value);
                cb(displayIndex, m_data[i].title, m_data[i].value, m_data[i].unit, m_data[i].id);
                displayIndex++;
            }
        }
    }
};