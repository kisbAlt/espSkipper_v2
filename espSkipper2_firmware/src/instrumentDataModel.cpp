#include "instrumentDataModel.hpp"
#include "stringTranslator.hpp"
#include "instrumentDataFormats.hpp"

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
    std::visit([&val, this, id](auto &currentVal)
               {
        using T = std::decay_t<decltype(currentVal)>;
        
        if constexpr (std::is_arithmetic_v<T>) {
            const T newVal = std::get<T>(val);
            if(!m_data[static_cast<size_t>(id)].hasData ||static_cast<T>(currentVal < newVal)) {
                m_data[static_cast<size_t>(id)].UpdateValue(newVal);
            }
        } }, m_data[static_cast<size_t>(id)].value);
}

void InstrumentDataModel::updateSensorIfSmaller(SensorId id, SensorValue val)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::visit([&val, this, id](auto &currentVal)
               {
        using T = std::decay_t<decltype(currentVal)>;
        
        if constexpr (std::is_arithmetic_v<T>) {
            const T newVal = std::get<T>(val);
            if(!m_data[static_cast<size_t>(id)].hasData || static_cast<T>(currentVal > newVal)) {
                m_data[static_cast<size_t>(id)].UpdateValue(newVal);
            }
        } }, m_data[static_cast<size_t>(id)].value);
}

void InstrumentDataModel::addSensor(SensorId id, SensorValue val, SensorUnit unit, TextKey titleKey, bool isAverage)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_data[static_cast<size_t>(id)] = {id, val, titleKey, unit, isAverage};
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

void InstrumentDataModel::reload()
{
}

SensorUnit::SensorUnit() : sensorUnit(SensorUnitEnum::BlankUnit)
{
}

SensorUnit::SensorUnit(SensorUnitEnum sensorUnitEnum) : sensorUnit(sensorUnitEnum)
{
}

const char *SensorUnit::GetString()
{
    return GetUnitString(sensorUnit);
}
UnitType SensorUnit::GetUnitType(SensorUnitEnum sensorUnitEnum) const
{
    switch (sensorUnitEnum)
    {
    case SensorUnitEnum::Kmph:
    case SensorUnitEnum::Mps:
    case SensorUnitEnum::Knots:
        return UnitType::Speed;
    case SensorUnitEnum::Kilometer:
    case SensorUnitEnum::Meter:
    case SensorUnitEnum::Feet:
        return UnitType::Distance;
    case SensorUnitEnum::Celsius:
        return UnitType::Temperature;
    case SensorUnitEnum::Degrees:
        return UnitType::Angle;
    case SensorUnitEnum::BlankUnit:
    default:
        return UnitType::None;
    }
};

UnitType SensorUnit::GetUnitType() const
{
    return GetUnitType(sensorUnit);
}

bool SensorUnit::IsSameUnitType(SensorUnitEnum sensorUnitEnum) const
{
    return GetUnitType() == GetUnitType(sensorUnitEnum);
}

void InstrumentDataModel::SensorRecord::UpdateValue(SensorValue newValue)
{
    if (!isAverage || !hasData)
    {
        value = newValue;
        dataCount = isAverage ? 1 : 0;
        hasData = true;
        return;
    }

    dataCount++;
    std::visit([&newValue, this](auto &currentVal)
               {
        using T = std::decay_t<decltype(currentVal)>;
        
        if constexpr (std::is_arithmetic_v<T>) {
            const T newVal = std::get<T>(newValue);
            
            currentVal = static_cast<T>(currentVal + (newVal - currentVal) / static_cast<float>(dataCount));
        } }, value);
}

SensorValue InstrumentDataModel::SensorRecord::GetValueInUnit(SensorUnitEnum targetUnitEnum) const
{
    if (unit.GetUnitEnum() == targetUnitEnum)
    {
        return value;
    }

    if (!unit.IsSameUnitType(targetUnitEnum) || unit.GetUnitType() == UnitType::None)
    {
        return value;
    }

    return std::visit([this, targetUnitEnum](auto &&arg) -> SensorValue
                      {
        using T = std::decay_t<decltype(arg)>;
        
        // Only attempt mathematical conversion on numerical types
        if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int>) {
            
            float val = static_cast<float>(arg);
            float convertedVal = val;

            // --- SPEED CONVERSIONS (Base unit: Mps) ---
            if (unit.GetUnitType() == UnitType::Speed) {
                float mps = val;
                
                switch (unit.GetUnitEnum()) {
                    case SensorUnitEnum::Kmph:  mps = val / 3.6f; break;
                    case SensorUnitEnum::Knots: mps = val * 0.514444f; break;
                    case SensorUnitEnum::Mps:   mps = val; break;
                    default: break;
                }
                
                switch (targetUnitEnum) {
                    case SensorUnitEnum::Kmph:  convertedVal = mps * 3.6f; break;
                    case SensorUnitEnum::Knots: convertedVal = mps / 0.514444f; break;
                    case SensorUnitEnum::Mps:   convertedVal = mps; break;
                    default: break;
                }
            }
            // --- DISTANCE CONVERSIONS (Base unit: Meter) ---
            else if (unit.GetUnitType() == UnitType::Distance) {
                float meters = val;
                
                // Convert current unit to base unit (Meter)
                switch (unit.GetUnitEnum()) {
                    case SensorUnitEnum::Kilometer: meters = val * 1000.0f; break;
                    case SensorUnitEnum::Feet:      meters = val * 0.3048f; break;
                    case SensorUnitEnum::Meter:     meters = val; break;
                    default: break;
                }
                
                // Convert base unit (Meter) to target unit
                switch (targetUnitEnum) {
                    case SensorUnitEnum::Kilometer: convertedVal = meters / 1000.0f; break;
                    case SensorUnitEnum::Feet:      convertedVal = meters / 0.3048f; break;
                    case SensorUnitEnum::Meter:     convertedVal = meters; break;
                    default: break;
                }
            }
            
            return convertedVal;

        } else {
            return arg;
        } }, value);
}
