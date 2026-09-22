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
    WindSpeedAWS,
    WindDirection,
    WindDirectionAWA,
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
    None,
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

enum class UnitType
{
    Speed,
    Distance,
    Temperature,
    Angle,
    None
};

const char* GetUnitString(SensorUnitEnum unitEnum);

class SensorUnit {
    private:
    SensorUnitEnum sensorUnit;
    public:
    SensorUnit();
    SensorUnit(SensorUnitEnum sensorUnitEnum);
    const char* GetString();
    UnitType GetUnitType(SensorUnitEnum sensorUnitEnum) const;
    UnitType GetUnitType() const;
    bool IsSameUnitType(SensorUnitEnum sensorUnitEnum) const;
    SensorUnitEnum GetUnitEnum() const { return sensorUnit; }
};

struct SensorValueString {
    char text[32];
};
using SensorValue = std::variant<float, int, SensorValueString>;
