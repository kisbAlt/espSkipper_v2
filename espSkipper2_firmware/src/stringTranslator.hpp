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
    X(SettingBtnBrightness, "Button Brightness", "Gomb Fenyeros") \
    X(SettingDisplayDateTime, "Display Date and Time", "Datum es Ido Megjeleniese") \
    X(SettingDisplayTimeOnly, "Display Time Only", "Csak Ido Megjeleniese") \
    X(SensorTiltPitch, "Tilt Pitch", "Doles bolintas") \
    X(SensorTiltPitchMin, "Tilt Pitch", "Bolint. Min") \
    X(SensorTiltPitchMax, "Tilt Pitch", "Bolint. Max") \
    X(SensorTiltPitchAvg, "Tilt Pitch", "Bolint Atl") \
    X(SensorTiltRoll, "Tilt Roll", "Doles") \
    X(SensorTiltRollMin, "Tilt Roll", "Doles Min") \
    X(SensorTiltRollMax, "Tilt Roll", "Doles Max") \
    X(SensorTiltRollAvg, "Tilt Roll", "Doles atl") \

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