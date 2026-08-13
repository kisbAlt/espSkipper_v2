#include "accelerometerHandler.hpp"
#include <Arduino.h>
#include "SparkFunLIS3DH.h"
#include "Wire.h"
#include "SPI.h"

LIS3DH SensorOne(SPI_MODE, LIS3DH_CS);


AccelerometerHandler::AccelerometerHandler(InstrumentDataModel& dataModel) : dataModel(dataModel)
{
    SensorUnit degreeUnit = SensorUnit(SensorUnitEnum::Degrees);
    dataModel.addSensor(SensorId::TiltPitch, 0.0f, degreeUnit, (char*)"Tilt Pitch");
    dataModel.addSensor(SensorId::TiltPitchMin, 0.0f, degreeUnit, (char*)"Pitch Min");
    dataModel.addSensor(SensorId::TiltPitchMax, 0.0f, degreeUnit, (char*)"Pitch Max");
    dataModel.addSensor(SensorId::TiltPitchAvg, 0.0f, degreeUnit, (char*)"Pitch Avg", true);
    dataModel.addSensor(SensorId::TiltRoll, 0.0f, degreeUnit, (char*)"Tilt Roll");
    dataModel.addSensor(SensorId::TiltRollMin, 0.0f, degreeUnit, (char*)"Roll Min");
    dataModel.addSensor(SensorId::TiltRollMax, 0.0f, degreeUnit, (char*)"Roll Max");
    dataModel.addSensor(SensorId::TiltRollAvg, 0.0f, degreeUnit, (char*)"Roll Avg", true);
    dataModel.disableSensor(SensorId::TiltPitchMin);
    dataModel.disableSensor(SensorId::TiltPitchMax);
    dataModel.disableSensor(SensorId::TiltPitchAvg);
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
    float raw_x = SensorOne.readFloatAccelX();
    float raw_y = SensorOne.readFloatAccelY();
    float raw_z = SensorOne.readFloatAccelZ();

    // Swapping Y and Z because gravity is acting on the physical Y-axis
    float virt_x = raw_x;  
    float virt_y = -raw_z; // The negative sign keeps the rotation direction standard
    float virt_z = raw_y;  // Change to -raw_y if your angles are perfectly upside down

    // Now do the math using the virtual axes
    float lastRoll = atan2(-virt_x, sqrt((virt_y * virt_y) + (virt_z * virt_z))) * (180.0 / PI);
    float lastPitch  = atan2(virt_y, virt_z) * (180.0 / PI);

    dataModel.updateSensor(SensorId::TiltPitch, lastPitch);
    dataModel.updateSensor(SensorId::TiltPitchAvg, lastPitch);
    dataModel.updateSensorIfSmaller(SensorId::TiltPitchMin, lastPitch);
    dataModel.updateSensorIfLarger(SensorId::TiltPitchMax, lastPitch);
    dataModel.updateSensor(SensorId::TiltRoll, lastRoll);
    dataModel.updateSensor(SensorId::TiltRollAvg, lastRoll);
    dataModel.updateSensorIfSmaller(SensorId::TiltRollMin, lastRoll);
    dataModel.updateSensorIfLarger(SensorId::TiltRollMax, lastRoll);

    // Serial.print("\nAccelerometer:\n");

    // Serial.print(" lastPitch = ");
    // Serial.println(lastPitch);
    // Serial.print(" lastRoll = ");
    // Serial.println(lastRoll);
}
