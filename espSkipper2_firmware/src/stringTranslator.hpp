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
    X(BlankUnit,      "",           "") 

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