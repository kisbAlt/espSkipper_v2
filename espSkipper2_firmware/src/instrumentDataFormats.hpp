#pragma once

#include <cstddef>
#include <variant>

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
    Mps,
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
