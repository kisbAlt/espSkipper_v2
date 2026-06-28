#include "accelerometerHandler.hpp"
#include <Arduino.h>
#include "SparkFunLIS3DH.h"
#include "Wire.h"
#include "SPI.h"

LIS3DH SensorOne(SPI_MODE, LIS3DH_CS);


AccelerometerHandler::AccelerometerHandler()
{
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
    float accelx = SensorOne.readFloatAccelX();
    float accely = SensorOne.readFloatAccelY();
    float accelz = SensorOne.readFloatAccelZ();

    float lastPitch = -atan2(accelx / 9.8, accelz / 9.8) / 2 / 3.141592654 * 360;
    float lastRoll = -atan2(accely / 9.8, accelz / 9.8) / 2 / 3.141592654 * 360;

    Serial.print("\nAccelerometer:\n");

    Serial.print(" lastPitch = ");
    Serial.println(lastPitch);
    Serial.print(" lastRoll = ");
    Serial.println(lastRoll);
}
