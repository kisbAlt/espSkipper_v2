#include "settingsHandler.hpp"
#include "stringTranslator.hpp"
#include <cstdio>

Settings::Settings()
{
}

void Settings::Begin()
{
    LoadFromNVS();
    Translator::SetLanguage(GetValue<uint8_t>(SettingsKey::Language));
}

bool Settings::IsSensorDisabled(SensorId id) const
{
    if (id >= SensorId::MAX_SENSORS)
        return false;
    return disabledSensors[static_cast<size_t>(id)];
}

const SettingValue &Settings::GetValueVariant(std::size_t index) const
{
    return currentValues[index];
}

const SettingDef &Settings::GetSettingDef(SettingsKey key) const
{
    std::size_t idx = GetIndex(key);
    return GetSettingDef(idx);
}

const SettingDef &Settings::GetSettingDef(size_t index) const
{
    return Schema[index];
}

void Settings::SetNextValue(size_t index)
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
    SaveToNVS(key, currentValues[index]);
}

void Settings::SetPreviousValue(size_t index)
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
    SaveToNVS(key, currentValues[index]);
}

void Settings::DisableSensor(SensorId id)
{
    if (id < SensorId::MAX_SENSORS)
    {
        disabledSensors[static_cast<size_t>(id)] = true;
        preferences.putBool(GetSensorNvsKey(id), disabledSensors[static_cast<size_t>(id)]);
    }
        
}

void Settings::EnableSensor(SensorId id)
{
    if (id < SensorId::MAX_SENSORS)
    {
        disabledSensors[static_cast<size_t>(id)] = false;
        preferences.putBool(GetSensorNvsKey(id), disabledSensors[static_cast<size_t>(id)]);
    }
       
}

bool Settings::IsSensorEnabled(SensorId id) const
{
    if (id < SensorId::MAX_SENSORS)
        return !disabledSensors[static_cast<size_t>(id)];
    return false;
}

void Settings::SaveToNVS(SettingsKey key, const SettingValue &value)
{
    Serial.println("Saving NVS");

    const char *nvsKey = GetSettingNvsKey(key);

    if (const bool *v = std::get_if<bool>(&value))
        preferences.putBool(nvsKey, *v);
    else if (const int *v = std::get_if<int>(&value))
    {
        preferences.putInt(nvsKey, *v);
        Serial.println("saving int");
        Serial.println(GetValue<int>(key));
    }

    else if (const float *v = std::get_if<float>(&value))
        preferences.putFloat(nvsKey, *v);
    else if (const uint8_t *v = std::get_if<uint8_t>(&value))
        preferences.putUChar(nvsKey, *v);
}

void Settings::LoadFromNVS()
{
    Serial.println("loading NVS");
    preferences.begin("settings", false);

    for (std::size_t i = 0; i < Count; ++i)
    {
        const char *nvsKey = GetSettingNvsKey(Schema[i].key);
        const SettingValue &def = Schema[i].defaultValue;

        if (const bool *d = std::get_if<bool>(&def))
            currentValues[i] = preferences.getBool(nvsKey, *d);
        else if (const int *d = std::get_if<int>(&def))
        {
            currentValues[i] = preferences.getInt(nvsKey, *d);
            Serial.println("loaded int");
            Serial.println(GetValue<int>(i));
        }

        else if (const float *d = std::get_if<float>(&def))
            currentValues[i] = preferences.getFloat(nvsKey, *d);
        else if (const uint8_t *d = std::get_if<uint8_t>(&def))
            currentValues[i] = preferences.getUChar(nvsKey, *d);
    }

    for (std::size_t i = 0; i < static_cast<std::size_t>(SensorId::MAX_SENSORS); ++i)
    {
        SensorId id = static_cast<SensorId>(i);
        const bool defaultDisabled = id == SensorId::MinGpsSpeed ||
                                        id == SensorId::TiltPitchAvg ||
                                        id == SensorId::TiltRollAvg ||
                                        id == SensorId::DateTimeHour ||
                                        id == SensorId::DateTimeMinute ||
                                        id == SensorId::DateTimeYear ||
                                        id == SensorId::DateTimeMonth ||
                                        id == SensorId::DateTimeDay;
        disabledSensors[i] = preferences.getBool(GetSensorNvsKey(id), defaultDisabled);
    }
}

const char *Settings::GetSettingNvsKey(SettingsKey key) const
{
    static char nvsKey[16];
    snprintf(nvsKey, sizeof(nvsKey), "sk_%u", static_cast<unsigned>(key));
    return nvsKey;
}

const char *Settings::GetSensorNvsKey(SensorId id) const
{
    static char nvsKey[16];
    snprintf(nvsKey, sizeof(nvsKey), "sensor_%u", static_cast<unsigned>(id));
    return nvsKey;
}

const char *SettingDef::GetTitleString() const
{
    return Translator::Get(displayName);
}

void SettingDef::GetOptionString(const SettingValue &currentValue, char *buffer, std::size_t bufferSize) const
{
    if (useOptionsText)
    {
        for (std::size_t i = 0; i < options.count; ++i)
        {
            if (options.items[i] == currentValue)
            {
                snprintf(buffer, bufferSize, "%s", Translator::Get(optionTexts[i]));
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
        snprintf(buffer, bufferSize, "%s", arg ? Translator::Get(TextKey::SettingOn) : Translator::Get(TextKey::SettingOff));
    } }, currentValue);
}
