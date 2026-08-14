#pragma once

#include <cstddef>
#include <Arduino.h>
#include "instrumentDataFormats.hpp"

class Settings {
public:
    Settings();
    bool isSensorDisabled(SensorId id) const;

    void disableSensor(SensorId id);
    void enableSensor(SensorId id);
    void setButtonBrightnessLevel(uint8_t level) { buttonBrightnessLevel = level; }
    uint8_t getButtonBrightnessLevel() const { return buttonBrightnessLevel; }
private:
    bool disabledSensors[static_cast<size_t>(SensorId::MAX_SENSORS)] = {false};
    uint8_t buttonBrightnessLevel = 10;
};