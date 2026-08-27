#pragma once

#include <cstddef>
#include <Arduino.h>
#include "instrumentDataFormats.hpp"
#include "buttonHandler.hpp"
#include "stringTranslator.hpp"
#include <mutex>
#include <condition_variable>
#include <Preferences.h>

enum class SettingsKey
{
    BtnBrightness,
    BtnLedEnabled,
    LCDBrightness,
    LCDLedEnabled,
    DisplayDateTime,
    DisplayTimeOnly,
    DisplayScreenUpdate,
};
using SettingValue = std::variant<bool, int, float, uint8_t>;

struct OptionList
{
    SettingValue items[8];
    std::size_t count;
    template <typename... Args>
    constexpr OptionList(Args... args) : items{args...}, count(sizeof...(args)) {}
};

// 2. The simplified ROM schema definition
struct SettingDef
{
    SettingsKey key;
    SettingValue defaultValue;
    TextKey displayName;
    OptionList options;

    const char *GetString() const;
};

class Settings
{
public:
    Settings();
    void begin();
    bool isSensorDisabled(SensorId id) const;
    uint8_t getSettingsCount() const { return Count; }

    template <typename T>
    bool setValue(SettingsKey key, T value)
    {
        std::size_t idx = getIndex(key);

        if (std::holds_alternative<T>(currentValues[idx]))
        {
            currentValues[idx] = value;
            saveToNVS(key, currentValues[idx]);
            return true;
        }
        return false;
    }

    template <typename T>
    T getValue(SettingsKey key) const
    {
        std::size_t idx = getIndex(key);
        return getValue<T>(idx);
    }

    template <typename T>
    T getValue(size_t idx) const
    {
        if (const T *val = std::get_if<T>(&currentValues[idx]))
        {
            return *val;
        }
        return T{};
    }

    const SettingValue &getValueVariant(std::size_t index) const
    {
        return currentValues[index];
    }

    const SettingDef &getSettingDef(SettingsKey key) const;

    const SettingDef &getSettingDef(size_t index) const;

    void setNextValue(size_t index);
    void setPreviousValue(size_t index);

private:
    static constexpr std::size_t Count = 7;

    static constexpr SettingDef Schema[Count] = {
        {SettingsKey::BtnBrightness,
         uint8_t(255),
         TextKey::SettingBtnBrightness,
         OptionList(uint8_t(5), uint8_t(40), uint8_t(80), uint8_t(120), uint8_t(255))},
        {SettingsKey::BtnLedEnabled,
         bool(false),
         TextKey::SettingBtnLedEnabled,
         OptionList(bool(true), bool(false))},
        {SettingsKey::LCDBrightness,
         uint8_t(255),
         TextKey::SettingLCDBrightness,
         OptionList(uint8_t(5), uint8_t(40), uint8_t(80), uint8_t(120), uint8_t(255))},
        {SettingsKey::LCDLedEnabled,
         bool(false),
         TextKey::SettingLCDLedEnabled,
         OptionList(bool(true), bool(false))},
        {SettingsKey::DisplayDateTime,
         bool(true),
         TextKey::SettingDisplayDateTime,
         OptionList(bool(false), bool(true))},
        {SettingsKey::DisplayTimeOnly,
         bool(true),
         TextKey::SettingDisplayTimeOnly,
         OptionList(bool(false), bool(true))},
        {SettingsKey::DisplayScreenUpdate,
         int(500),
         TextKey::SettingDisplayUpdateTime,
         OptionList((int)62, (int)100, (int)200, (int)300, (int)400, (int)500, (int)1000, (int)2000)}

    };
    SettingValue currentValues[Count];

    static constexpr std::size_t getIndex(SettingsKey key)
    {
        for (std::size_t i = 0; i < Count; ++i)
        {
            if (Schema[i].key == key)
                return i;
        }
        return 0; // Fallback
    }

    void disableSensor(SensorId id);
    void enableSensor(SensorId id);
    bool disabledSensors[static_cast<size_t>(SensorId::MAX_SENSORS)] = {false};
    Preferences preferences;

    void saveToNVS(SettingsKey key, const SettingValue &value)
    {
        Serial.println("Saving NVS");
        String nvsKey = "sk_" + String(static_cast<int>(key));

        if (const bool *v = std::get_if<bool>(&value))
            preferences.putBool(nvsKey.c_str(), *v);
        else if (const int *v = std::get_if<int>(&value))
        {
            preferences.putInt(nvsKey.c_str(), *v);
            Serial.println("saving int");
            Serial.println(getValue<int>(key));
        }

        else if (const float *v = std::get_if<float>(&value))
            preferences.putFloat(nvsKey.c_str(), *v);
        else if (const uint8_t *v = std::get_if<uint8_t>(&value))
            preferences.putUChar(nvsKey.c_str(), *v);
    }

    void loadFromNVS()
    {
        Serial.println("loading NVS");
        preferences.begin("settings", false);

        for (std::size_t i = 0; i < Count; ++i)
        {
            String nvsKey = "sk_" + String(static_cast<int>(Schema[i].key));
            const SettingValue &def = Schema[i].defaultValue;

            if (const bool *d = std::get_if<bool>(&def))
                currentValues[i] = preferences.getBool(nvsKey.c_str(), *d);
            else if (const int *d = std::get_if<int>(&def))
            {
                currentValues[i] = preferences.getInt(nvsKey.c_str(), *d);
                Serial.println("loaded int");
                Serial.println(getValue<int>(i));
            }

            else if (const float *d = std::get_if<float>(&def))
                currentValues[i] = preferences.getFloat(nvsKey.c_str(), *d);
            else if (const uint8_t *d = std::get_if<uint8_t>(&def))
                currentValues[i] = preferences.getUChar(nvsKey.c_str(), *d);
        }
    }
};