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

const SettingDef &Settings::getSettingDef(SettingsKey key) const
{
    std::size_t idx = getIndex(key);
    return getSettingDef(idx);
}

const SettingDef &Settings::getSettingDef(size_t index) const
{
    return Schema[index];
}

void Settings::setNextValue(size_t index)
{
    const OptionList& opts = Schema[index].options;

    if (opts.count == 0) return;

    std::size_t currentOptionIdx = 0;
    for (std::size_t i = 0; i < opts.count; ++i) {
        if (opts.items[i] == currentValues[index]) {
            currentOptionIdx = i;
            break;
        }
    }

    std::size_t nextIdx = (currentOptionIdx + 1) % opts.count;
    currentValues[index] = opts.items[nextIdx];
}

void Settings::setPreviousValue(size_t index)
{
    const OptionList& opts = Schema[index].options;

    if (opts.count == 0) return;

    std::size_t currentOptionIdx = 0;
    for (std::size_t i = 0; i < opts.count; ++i) {
        if (opts.items[i] == currentValues[index]) {
            currentOptionIdx = i;
            break;
        }
    }

    std::size_t prevIdx = (currentOptionIdx == 0) ? (opts.count - 1) : (currentOptionIdx - 1);
    currentValues[index] = opts.items[prevIdx];
    return;
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
