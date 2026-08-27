#pragma once

// 1. Define the Master Table
#define TRANSLATION_TABLE(X) \
    X(Kmph,           "km/h",       "km/h") \
    X(KilometerShort, "km",         "km") \
    X(KilometerLong,  "kilometers", "kilometer") \
    X(MeterLong,      "meters",     "meter") \
    X(MeterShort,     "m",          "m") \
    X(KnotsLong,      "Knots",      "Csomo") \
    X(KnotsShort,     "kts",        "kts") \
    X(FeetsLong,      "feet",       "lab") \
    X(FeetsShort,     "ft",         "ft") \
    X(CelsiusLong,    "Celsius",    "Celsius") \
    X(CelsiusShort,   "°C",         "°C") \
    X(DegreesLong,    "degrees",    "fok") \
    X(DegreesShort,   "deg",         "fok") \
    X(BlankUnit,      "",           "")  \
    X(SettingOn, "On", "Be") \
    X(SettingOff, "Off", "Ki") \
    X(SettingBtnBrightness, "Button Brightness", "Gomb Fenyero") \
    X(SettingBtnLedEnabled, "Button LED On", "Gomb Led be") \
    X(SettingLCDBrightness, "LCD Brightness", "LCD Fenyero") \
    X(SettingLCDLedEnabled, "LCD LED On", "LCD Led be") \
    X(SettingDisplayDateTime, "Display Date and Time", "Datum es Ido Megjeleniese") \
    X(SettingDisplayTimeOnly, "Display Time Only", "Csak Ido Megjeleniese") \
    X(SettingDisplayUpdateTime, "Update screen every ms", "Kijelzo frissitese ms-enkent") \
    X(SensorTiltPitch, "Tilt Pitch", "Doles bolintas") \
    X(SensorTiltPitchMin, "Pitch Min", "Bolint. Min") \
    X(SensorTiltPitchMax, "Pitch Max", "Bolint. Max") \
    X(SensorTiltPitchAvg, "Pitch Avg", "Bolint Atl") \
    X(SensorTiltRoll, "Tilt Roll", "Doles") \
    X(SensorTiltRollMin, "Roll Min", "Doles Min") \
    X(SensorTiltRollMax, "Roll Max", "Doles Max") \
    X(SensorTiltRollAvg, "Roll Avg", "Doles atl") \
    X(SensorDepth, "Water Depth", "Vízmélység") \

// 2. Generate the Enum
#define GENERATE_ENUM(KEY, EN, HU) KEY,
enum class TextKey {
    TRANSLATION_TABLE(GENERATE_ENUM)
    Count
};
#undef GENERATE_ENUM // Clean up macro to prevent pollution

// 3. Declare the Class
class Translator {
private:
    static int currentLang;

public:
    static void setLanguage(int langId);
    static const char* get(TextKey key);
};