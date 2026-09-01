#include "instrumentDataFormats.hpp"
#include "stringTranslator.hpp"

const char *GetUnitString(SensorUnitEnum unitEnum)
{
    switch (unitEnum)
    {
    case SensorUnitEnum::Kmph:
        return Translator::get(TextKey::Kmph);
    case SensorUnitEnum::Mps:
        return Translator::get(TextKey::Mps);
    case SensorUnitEnum::Kilometer:
        return Translator::get(TextKey::KilometerShort);
    case SensorUnitEnum::Meter:
        return Translator::get(TextKey::MeterShort);
    case SensorUnitEnum::Knots:
        return Translator::get(TextKey::KnotsShort);
    case SensorUnitEnum::Feet:
        return Translator::get(TextKey::FeetsShort);
    case SensorUnitEnum::Celsius:
        return Translator::get(TextKey::CelsiusShort);
    case SensorUnitEnum::Degrees:
        return Translator::get(TextKey::DegreesShort);
    case SensorUnitEnum::BlankUnit:
        return "";
    default:
        return nullptr;
    }
}