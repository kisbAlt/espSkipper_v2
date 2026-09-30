#pragma once

#define TRANSLATION_TABLE(X) \
    X(Empty,          "",          "") \
    X(Kmph,           "km/h",       "km/h") \
    X(Mps,            "m/s",        "m/s") \
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
    X(Settings, "Settings", "Beallitasok") \
    X(SettingOn, "On", "Be") \
    X(SettingOff, "Off", "Ki") \
    X(SettingBtnBrightness, "Button Brightness", "Gomb Fenyero") \
    X(SettingBtnLedEnabled, "Button LED On", "Gomb Led be") \
    X(SettingLCDBrightness, "LCD Brightness", "LCD Fenyero") \
    X(SettingLCDLedEnabled, "LCD LED On", "LCD Led be") \
    X(SettingDisplayDateTime, "Display Date and Time", "Datum es Ido Megjeleniese") \
    X(SettingDisplayTimeOnly, "Display Time Only", "Csak Ido Megjeleniese") \
    X(SettingDisplayUpdateTime, "Update screen every ms", "Kijelzo frissitese ms-enkent") \
    X(SettingSpeedUnit, "Speed Unit", "Sebesseg Egyseg") \
    X(SettingDistanceUnit, "Distance Unit", "Tavolsag Egyseg") \
    X(SettingLanguage, "Language", "Nyelv") \
    X(SettingDisabledSensors, "Disabled Sensors", "Letiltott Szenzorok") \
    X(SensorTiltPitch, "Tilt Pitch", "Doles bolintas") \
    X(SensorTiltPitchMin, "Pitch Min", "Bolint. Min") \
    X(SensorTiltPitchMax, "Pitch Max", "Bolint. Max") \
    X(SensorTiltPitchAvg, "Pitch Avg", "Bolint Atl") \
    X(SensorTiltRoll, "Tilt Roll", "Doles") \
    X(SensorTiltRollMin, "Roll Min", "Doles Min") \
    X(SensorTiltRollMax, "Roll Max", "Doles Max") \
    X(SensorTiltRollAvg, "Roll Avg", "Doles atl") \
    X(SensorDepth, "Water Depth", "Vizmelyseg") \
    X(SensorWind, "Wind", "Szel") \
    X(SensorWindDirection, "Wind Direction", "Szelirany") \
    X(SensorWindDirectionAWA, "AWA", "AWA") \
    X(SensorWindSpeed, "TWS", "TWS") \
    X(SensorWindSpeedAWS, "AWS", "AWS") \
    X(SensorGpsSpeed, "GPS Speed", "GPS Sebesseg") \
    X(SensorSatelliteCount, "Satellite Count", "Muholdak") \
    X(SensorMinGpsSpeed, "GPS Speed Min", "GPS Sebesseg Min") \
    X(SensorMaxGpsSpeed, "GPS Speed Max", "GPS Sebesseg Max") \
    X(SensorAvgGpsSpeed, "GPS Speed Avg", "GPS Sebesseg Atl") \
    X(SensorGpsCourse, "GPS Course", "GPS Irany") \
    X(SensorDateTimeHour, "Hour", "Ora") \
    X(SensorDateTimeMinute, "Minute", "Perc") \
    X(SensorDateTimeSecond, "Second", "Masodperc") \
    X(SensorDateTimeDay, "Day", "Nap") \
    X(SensorDateTimeMonth, "Month", "Honap") \
    X(SensorDateTimeYear, "Year", "Ev") \
    X(LanguageHungarianShort, "HU", "HU") \
    X(LanguageEnglishShort, "EN", "EN") \

#define GENERATE_ENUM(KEY, EN, HU) KEY,
enum class TextKey {
    TRANSLATION_TABLE(GENERATE_ENUM)
    Count
};
#undef GENERATE_ENUM

class Translator {
private:
    static int currentLang;

public:
    static void setLanguage(int langId);
    static const char* get(const TextKey key);
};