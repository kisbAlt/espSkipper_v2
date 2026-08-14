#pragma once

#include <cstddef>
#include <Arduino.h>
#include "instrumentDataFormats.hpp"
#include "buttonHandler.hpp"
#include "stringTranslator.hpp"
#include <mutex>
#include <condition_variable>

enum class SettingsKey
{
    BtnBrightness,
    DisplayDateTime,
    DisplayTimeOnly,
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
    bool isSensorDisabled(SensorId id) const;
    uint8_t getSettingsCount() const { return Count; }

    template <typename T>
    bool setValue(SettingsKey key, T value)
    {
        std::size_t idx = getIndex(key);

        if (std::holds_alternative<T>(currentValues[idx]))
        {
            currentValues[idx] = value;
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

    SettingValue getValueVariant(std::size_t index) const
    {
        return currentValues[index];
    }

    const SettingDef &getSettingDef(SettingsKey key) const;

    const SettingDef &getSettingDef(size_t index) const;

    void setNextValue(size_t index);
    void setPreviousValue(size_t index);

private:
    static constexpr std::size_t Count = 3;

    // Look how clean the definition is now! All inline, no external variables.
    static constexpr SettingDef Schema[Count] = {
        {SettingsKey::BtnBrightness,
         uint8_t(255),
         TextKey::SettingBtnBrightness,
         OptionList(uint8_t(5), uint8_t(40), uint8_t(80), uint8_t(120), uint8_t(255))},
        {SettingsKey::DisplayDateTime,
         bool(true),
         TextKey::SettingDisplayDateTime,
         OptionList(bool(false), bool(true))},
        {SettingsKey::DisplayTimeOnly,
         bool(true),
         TextKey::SettingDisplayTimeOnly,
         OptionList(bool(false), bool(true))}};
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
};