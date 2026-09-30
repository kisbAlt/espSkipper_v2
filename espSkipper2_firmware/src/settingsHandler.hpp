#pragma once

#include <cstddef>
#include <Arduino.h>
#include "instrumentDataFormats.hpp"
#include "buttonHandler.hpp"
#include "stringTranslator.hpp"
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
    SpeedUnit,
    DistanceUnit,
    TemperatureUnit,
    Language,
    DisabledSensors
};
using SettingValue = std::variant<bool, int, float, uint8_t, TextKey>;
#define OPTION_COUNT 8

struct OptionList
{
    SettingValue items[OPTION_COUNT];
    std::size_t count;
    template <typename... Args>
    constexpr OptionList(Args... args) : items{args...}, count(sizeof...(args)) {}
};

struct SettingDef
{
    SettingsKey key;
    SettingValue defaultValue;
    TextKey displayName;
    OptionList options;
    TextKey optionTexts[OPTION_COUNT];
    bool useOptionsText = false;

    const char *GetTitleString() const;
    void GetOptionString(const SettingValue &currentValue, char *buffer, std::size_t bufferSize) const;
};

class Settings
{
public:
    Settings();
    void Begin();
    bool IsSensorDisabled(SensorId id) const;
    constexpr uint8_t GetSettingsCount() const { return Count; }

    template <typename T>
    bool SetValue(SettingsKey key, T value)
    {
        std::size_t idx = GetIndex(key);

        if (std::holds_alternative<T>(currentValues[idx]))
        {
            currentValues[idx] = value;
            SaveToNVS(key, currentValues[idx]);
            return true;
        }
        return false;
    }

    template <typename T>
    T GetValue(SettingsKey key) const
    {
        std::size_t idx = GetIndex(key);
        return GetValue<T>(idx);
    }

    template <typename T>
    T GetValue(size_t idx) const
    {
        if (const T *val = std::get_if<T>(&currentValues[idx]))
        {
            return *val;
        }
        return T{};
    }

    const SettingValue &GetValueVariant(std::size_t index) const;
    const SettingDef &GetSettingDef(SettingsKey key) const;
    const SettingDef &GetSettingDef(size_t index) const;

    void SetNextValue(size_t index);
    void SetPreviousValue(size_t index);
    void EnableSensor(SensorId id);
    void DisableSensor(SensorId id);
    bool IsSensorEnabled(SensorId id) const;

private:
    static constexpr std::size_t Count = 11;

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
         OptionList((int)62, (int)100, (int)200, (int)300, (int)400, (int)500, (int)1000, (int)2000)},
        {SettingsKey::SpeedUnit,
         (uint8_t)SensorUnitEnum::Kmph,
         TextKey::SettingSpeedUnit,
         OptionList((uint8_t)SensorUnitEnum::Kmph, (uint8_t)SensorUnitEnum::Mps, (uint8_t)SensorUnitEnum::Knots),
         {TextKey::Kmph, TextKey::Mps, TextKey::KnotsShort},
         true},
        {SettingsKey::DistanceUnit,
         (uint8_t)SensorUnitEnum::Kilometer,
         TextKey::SettingDistanceUnit,
         OptionList((uint8_t)SensorUnitEnum::Kilometer, (uint8_t)SensorUnitEnum::Meter),
         {TextKey::KilometerShort, TextKey::MeterShort},
         true},
        {SettingsKey::Language,
         (uint8_t)0,
         TextKey::SettingLanguage,
         OptionList((uint8_t)0, (uint8_t)1),
         {TextKey::LanguageEnglishShort, TextKey::LanguageHungarianShort},
         true},
        {SettingsKey::DisabledSensors,
         bool(true),
         TextKey::SettingDisabledSensors,
         OptionList(bool(false), bool(true)),
         {TextKey::Empty, TextKey::Empty},
         true},

    };
    SettingValue currentValues[Count];

    static constexpr std::size_t GetIndex(SettingsKey key)
    {
        for (std::size_t i = 0; i < Count; ++i)
        {
            if (Schema[i].key == key)
                return i;
        }
        return 0; // Fallback
    }
    bool disabledSensors[static_cast<size_t>(SensorId::MAX_SENSORS)] = {false};
    Preferences preferences;

    void SaveToNVS(SettingsKey key, const SettingValue &value);

    void LoadFromNVS();

    const char *GetSettingNvsKey(SettingsKey key) const;
    const char *GetSensorNvsKey(SensorId id) const;
};