#include "settingsHandler.hpp"
#include "stringTranslator.hpp"

Settings::Settings()
{
    for (std::size_t i = 0; i < Count; ++i)
    {
        currentValues[i] = Schema[i].defaultValue;
    }
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
    if (id >= SensorId::MAX_SENSORS)
        return false;
    return disabledSensors[static_cast<size_t>(id)];
}

void Settings::disableSensor(SensorId id)
{
    if (id < SensorId::MAX_SENSORS)
        disabledSensors[static_cast<size_t>(id)] = true;
}

void Settings::enableSensor(SensorId id)
{
    if (id < SensorId::MAX_SENSORS)
        disabledSensors[static_cast<size_t>(id)] = false;
}

const char *SettingDef::GetString() const
{
    return Translator::get(displayName);
}
