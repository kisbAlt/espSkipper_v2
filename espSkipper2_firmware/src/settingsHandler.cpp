#include "settingsHandler.hpp"

Settings::Settings()
{
    disableSensor(SensorId::MinGpsSpeed);
    disableSensor(SensorId::TiltPitchAvg);
    disableSensor(SensorId::TiltRollAvg);
    disableSensor(SensorId::DateTimeHour);
    disableSensor(SensorId::DateTimeMinute);
    disableSensor(SensorId::DateTimeYear);
    disableSensor(SensorId::DateTimeMonth);
    disableSensor(SensorId::DateTimeDay);
}

bool Settings::isSensorDisabled(SensorId id) const
{
    if (id >= SensorId::MAX_SENSORS) return false;
    return disabledSensors[static_cast<size_t>(id)];
}

void Settings::disableSensor(SensorId id)
{
    if (id < SensorId::MAX_SENSORS) disabledSensors[static_cast<size_t>(id)] = true;
}

void Settings::enableSensor(SensorId id)
{
    if (id < SensorId::MAX_SENSORS) disabledSensors[static_cast<size_t>(id)] = false;
}
