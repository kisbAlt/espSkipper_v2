#include "settingsHandler.hpp"
#include "stringTranslator.hpp"

Settings::Settings()
{
}

void Settings::begin()
{
    loadFromNVS();
    disableSensor(SensorId::MinGpsSpeed);
    disableSensor(SensorId::TiltPitchAvg);
    disableSensor(SensorId::TiltRollAvg);
    disableSensor(SensorId::DateTimeHour);
    disableSensor(SensorId::DateTimeMinute);
    disableSensor(SensorId::DateTimeYear);
    disableSensor(SensorId::DateTimeMonth);
    disableSensor(SensorId::DateTimeDay);
    Translator::setLanguage(getValue<uint8_t>(SettingsKey::Language));
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
    const OptionList &opts = Schema[index].options;
    SettingsKey key = Schema[index].key;
    if (opts.count == 0)
        return;

    std::size_t currentOptionIdx = 0;
    for (std::size_t i = 0; i < opts.count; ++i)
    {
        if (opts.items[i] == currentValues[index])
        {
            currentOptionIdx = i;
            break;
        }
    }

    std::size_t nextIdx = (currentOptionIdx + 1) % opts.count;
    currentValues[index] = opts.items[nextIdx];
    saveToNVS(key, currentValues[index]);
}

void Settings::setPreviousValue(size_t index)
{
    const OptionList &opts = Schema[index].options;
    SettingsKey key = Schema[index].key;
    if (opts.count == 0)
        return;

    std::size_t currentOptionIdx = 0;
    for (std::size_t i = 0; i < opts.count; ++i)
    {
        if (opts.items[i] == currentValues[index])
        {
            currentOptionIdx = i;
            break;
        }
    }

    std::size_t prevIdx = (currentOptionIdx == 0) ? (opts.count - 1) : (currentOptionIdx - 1);
    currentValues[index] = opts.items[prevIdx];
    saveToNVS(key, currentValues[index]);
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

bool Settings::isSensorEnabled(SensorId id) const
{
    if (id < SensorId::MAX_SENSORS)
        return !disabledSensors[static_cast<size_t>(id)];
    return false;
}

const char *SettingDef::GetTitleString() const
{
    return Translator::get(displayName);
}

void SettingDef::GetOptionString(const SettingValue &currentValue, char *buffer, std::size_t bufferSize) const
{
    if (useOptionsText)
    {
        for (std::size_t i = 0; i < options.count; ++i)
        {
            if (options.items[i] == currentValue)
            {
                snprintf(buffer, bufferSize, "%s", Translator::get(optionTexts[i]));
                return;
            }
        }
    }
    std::visit([&buffer, bufferSize](const auto &arg)
               {
    using T = std::decay_t<decltype(arg)>;

    if constexpr (std::is_same_v<T, float>) {
        dtostrf(arg, 1, 1, buffer);
    } 
    else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, uint8_t>) {
        snprintf(buffer, bufferSize, "%d", static_cast<int>(arg));
    } 
    else if constexpr (std::is_same_v<T, bool>) {
        snprintf(buffer, bufferSize, "%s", arg ? Translator::get(TextKey::SettingOn) : Translator::get(TextKey::SettingOff));
    } }, currentValue);
}
