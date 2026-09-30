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
    dataModel.AddSensor(SensorId::TiltPitch, 0, degreeUnit, TextKey::SensorTiltPitch);
    dataModel.AddSensor(SensorId::TiltPitchMin, 0, degreeUnit, TextKey::SensorTiltPitchMin);
    dataModel.AddSensor(SensorId::TiltPitchMax, 0, degreeUnit, TextKey::SensorTiltPitchMax);
    dataModel.AddSensor(SensorId::TiltPitchAvg, 0, degreeUnit, TextKey::SensorTiltPitchAvg, true);
    dataModel.AddSensor(SensorId::TiltRoll, 0, degreeUnit, TextKey::SensorTiltRoll);
    dataModel.AddSensor(SensorId::TiltRollMin, 0, degreeUnit, TextKey::SensorTiltRollMin);
    dataModel.AddSensor(SensorId::TiltRollMax, 0, degreeUnit, TextKey::SensorTiltRollMax);
    dataModel.AddSensor(SensorId::TiltRollAvg, 0, degreeUnit, TextKey::SensorTiltRollAvg, true);
}

void AccelerometerHandler::Init()
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

void AccelerometerHandler::ReadAccelerometerData()
{
    const float rawX = SensorOne.readFloatAccelX();
    const float rawY = SensorOne.readFloatAccelY();
    const float rawZ = SensorOne.readFloatAccelZ();

    // swapping Y and Z for PCB position
    const float virtualX = rawX;
    const float virtualY = -rawZ; // keep the rotation direction standard
    const float virtualZ = rawY;

    const float lastRoll = atan2(-virtualX, sqrt((virtualY * virtualY) + (virtualZ * virtualZ))) * (180.0 / PI);
    const float lastPitch  = atan2(virtualY, virtualZ) * (180.0 / PI);

    const int rollRounded = InstrumentDataModel::FastRoundPositive(InstrumentDataModel::MakePositive(lastRoll));
    const int pitchRounded = InstrumentDataModel::FastRoundPositive(InstrumentDataModel::MakePositive(lastPitch));

    dataModel.UpdateSensor(SensorId::TiltPitch, pitchRounded);
    dataModel.UpdateSensor(SensorId::TiltPitchAvg, pitchRounded);
    dataModel.UpdateSensorIfSmaller(SensorId::TiltPitchMin, pitchRounded);
    dataModel.UpdateSensorIfLarger(SensorId::TiltPitchMax, pitchRounded);
    dataModel.UpdateSensor(SensorId::TiltRoll, rollRounded);
    dataModel.UpdateSensor(SensorId::TiltRollAvg, rollRounded);
    dataModel.UpdateSensorIfSmaller(SensorId::TiltRollMin, rollRounded);
    dataModel.UpdateSensorIfLarger(SensorId::TiltRollMax, rollRounded);
}

void AccelerometerHandler::Reload()
{
}
