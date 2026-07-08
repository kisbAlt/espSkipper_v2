#include "instrumentDataModel.hpp"
#include "stringTranslator.hpp"

void InstrumentDataModel::updateSensor(SensorId id, SensorValue val)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_data[static_cast<size_t>(id)].value = val;
    m_data[static_cast<size_t>(id)].hasData = true;
}
void InstrumentDataModel::addSensor(SensorId id, SensorValue val, SensorUnit unit, char *title)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_data[static_cast<size_t>(id)] = {id, val, title, unit, true};
    count++;
}

std::vector<std::pair<SensorId, SensorValue>> InstrumentDataModel::getDisplaySnapshot() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::pair<SensorId, SensorValue>> snapshot;

    for (size_t i = 0; i < m_data.size(); ++i)
    {
        if (m_data[i].hasData)
        {
            snapshot.push_back({static_cast<SensorId>(i), m_data[i].value});
        }
    }
    return snapshot;
}

SensorUnit::SensorUnit() : sensorUnit(SensorUnitEnum::BlankUnit)
{
}

SensorUnit::SensorUnit(SensorUnitEnum sensorUnitEnum) : sensorUnit(sensorUnitEnum)
{
}

const char *SensorUnit::GetString()
{
    switch (sensorUnit)
    {
    case SensorUnitEnum::Kmph:
        return Translator::get(TextKey::Kmph);
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
};
