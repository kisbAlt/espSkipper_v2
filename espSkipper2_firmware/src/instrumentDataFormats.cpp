#include "instrumentDataFormats.hpp"
#include "stringTranslator.hpp"

const char *GetUnitString(SensorUnitEnum unitEnum)
{
    switch (unitEnum)
    {
    case SensorUnitEnum::Kmph:
        return Translator::Get(TextKey::Kmph);
    case SensorUnitEnum::Mps:
        return Translator::Get(TextKey::Mps);
    case SensorUnitEnum::Kilometer:
        return Translator::Get(TextKey::KilometerShort);
    case SensorUnitEnum::Meter:
        return Translator::Get(TextKey::MeterShort);
    case SensorUnitEnum::Knots:
        return Translator::Get(TextKey::KnotsShort);
    case SensorUnitEnum::Feet:
        return Translator::Get(TextKey::FeetsShort);
    case SensorUnitEnum::Celsius:
        return Translator::Get(TextKey::CelsiusShort);
    case SensorUnitEnum::Degrees:
        return Translator::Get(TextKey::DegreesShort);
    case SensorUnitEnum::BlankUnit:
        return "";
    default:
        return nullptr;
    }
}