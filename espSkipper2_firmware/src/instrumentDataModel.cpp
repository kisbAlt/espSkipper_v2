#include "instrumentDataModel.hpp"
#include "stringTranslator.hpp"

InstrumentDataModel::InstrumentDataModel(const Settings &settings) : m_settings(settings)
{
}

void InstrumentDataModel::updateSensor(SensorId id, SensorValue val)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_data[static_cast<size_t>(id)].UpdateValue(val);
}

void InstrumentDataModel::updateSensorIfLarger(SensorId id, SensorValue val)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::visit([&val, this, id](auto& currentVal) {
        using T = std::decay_t<decltype(currentVal)>;
        
        if constexpr (std::is_arithmetic_v<T>) {
            const T newVal = std::get<T>(val);
            if(!m_data[static_cast<size_t>(id)].hasData ||static_cast<T>(currentVal < newVal)) {
                m_data[static_cast<size_t>(id)].UpdateValue(newVal);
            }
        }
    }, m_data[static_cast<size_t>(id)].value);
}

void InstrumentDataModel::updateSensorIfSmaller(SensorId id, SensorValue val)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::visit([&val, this, id](auto& currentVal) {
        using T = std::decay_t<decltype(currentVal)>;
        
        if constexpr (std::is_arithmetic_v<T>) {
            const T newVal = std::get<T>(val);
            if(!m_data[static_cast<size_t>(id)].hasData || static_cast<T>(currentVal > newVal)) {
                m_data[static_cast<size_t>(id)].UpdateValue(newVal);
            }
        }
    }, m_data[static_cast<size_t>(id)].value);
}

void InstrumentDataModel::addSensor(SensorId id, SensorValue val, SensorUnit unit, char *title, bool isAverage)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_data[static_cast<size_t>(id)] = {id, val, title, unit, isAverage};
    count++;
}

bool InstrumentDataModel::isSensorEnabled(SensorId id) const
{
    return !m_settings.isSensorDisabled(id);
}

bool InstrumentDataModel::sensorHaveData(SensorId id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_data[static_cast<size_t>(id)].hasData;
}

SensorValue InstrumentDataModel::getSensorValue(SensorId id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
   return m_data[static_cast<size_t>(id)].value;
}

bool InstrumentDataModel::isSensorEnabledAndHaveData(SensorId id) const
{
    return isSensorEnabled(id) && sensorHaveData(id);
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

int InstrumentDataModel::getActiveSensorCount() const
{
    int count = 0;
    const int size = static_cast<int>(SensorId::MAX_SENSORS);
    for (size_t i = 0; i < static_cast<int>(SensorId::MAX_SENSORS); ++i)
    {
        if (isSensorEnabledAndHaveData(static_cast<SensorId>(i)))
        {
            count++;
        }
    }
    return count;
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

void InstrumentDataModel::SensorRecord::UpdateValue(SensorValue newValue)
{
    if (!isAverage || !hasData) {
        value = newValue;
        dataCount = isAverage ? 1 : 0;
        hasData = true;
        return;
    }
    
    dataCount++;
    std::visit([&newValue, this](auto& currentVal) {
        using T = std::decay_t<decltype(currentVal)>;
        
        if constexpr (std::is_arithmetic_v<T>) {
            const T newVal = std::get<T>(newValue);
            
            currentVal = static_cast<T>(currentVal + (newVal - currentVal) / static_cast<float>(dataCount));
        }
    }, value);
}
