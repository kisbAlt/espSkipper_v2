#include "accelerometerHandler.hpp"
#include "stringTranslator.hpp"
#include <Arduino.h>
#include "SparkFunLIS3DH.h"
#include "Wire.h"
#include "SPI.h"

LIS3DH SensorOne(SPI_MODE, LIS3DH_CS);


AccelerometerHandler::AccelerometerHandler(InstrumentDataModel& dataModel) : dataModel(dataModel)
{
    SensorUnit degreeUnit = SensorUnit(SensorUnitEnum::Degrees);
    dataModel.addSensor(SensorId::TiltPitch, 0, degreeUnit, TextKey::SensorTiltPitch);
    dataModel.addSensor(SensorId::TiltPitchMin, 0, degreeUnit, TextKey::SensorTiltPitchMin);
    dataModel.addSensor(SensorId::TiltPitchMax, 0, degreeUnit, TextKey::SensorTiltPitchMax);
    dataModel.addSensor(SensorId::TiltPitchAvg, 0, degreeUnit, TextKey::SensorTiltPitchAvg, true);
    dataModel.addSensor(SensorId::TiltRoll, 0, degreeUnit, TextKey::SensorTiltRoll);
    dataModel.addSensor(SensorId::TiltRollMin, 0, degreeUnit, TextKey::SensorTiltRollMin);
    dataModel.addSensor(SensorId::TiltRollMax, 0, degreeUnit, TextKey::SensorTiltRollMax);
    dataModel.addSensor(SensorId::TiltRollAvg, 0, degreeUnit, TextKey::SensorTiltRollAvg, true);
}

void AccelerometerHandler::init()
{
    SensorOne.settings.tempEnabled = 1;
    SensorOne.settings.accelSampleRate = 10;
    SensorOne.settings.accelRange = 2;
    
    if (SensorOne.begin() != 0x00) {
        Serial.println("Problem starting the LIS3DH sensor.");
    } else {
        Serial.println("LIS3DH started successfully.");
    }

}

void AccelerometerHandler::readAccelerometerData()
{
    const float raw_x = SensorOne.readFloatAccelX();
    const float raw_y = SensorOne.readFloatAccelY();
    const float raw_z = SensorOne.readFloatAccelZ();

    // swapping Y and Z for PCB position
    const float virt_x = raw_x;  
    const float virt_y = -raw_z; // keep the rotation direction standard
    const float virt_z = raw_y;

    const float lastRoll = atan2(-virt_x, sqrt((virt_y * virt_y) + (virt_z * virt_z))) * (180.0 / PI);
    const float lastPitch  = atan2(virt_y, virt_z) * (180.0 / PI);

    const int rollRounded = InstrumentDataModel::fast_round_positive(lastRoll);
    const int pitchRounded = InstrumentDataModel::fast_round_positive(lastPitch);

    dataModel.updateSensor(SensorId::TiltPitch, pitchRounded);
    dataModel.updateSensor(SensorId::TiltPitchAvg, pitchRounded);
    dataModel.updateSensorIfSmaller(SensorId::TiltPitchMin, pitchRounded);
    dataModel.updateSensorIfLarger(SensorId::TiltPitchMax, pitchRounded);
    dataModel.updateSensor(SensorId::TiltRoll, rollRounded);
    dataModel.updateSensor(SensorId::TiltRollAvg, rollRounded);
    dataModel.updateSensorIfSmaller(SensorId::TiltRollMin, rollRounded);
    dataModel.updateSensorIfLarger(SensorId::TiltRollMax, rollRounded);
}

void AccelerometerHandler::reload()
{
}
