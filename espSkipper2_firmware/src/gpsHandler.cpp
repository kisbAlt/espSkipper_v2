#include "gpsHandler.hpp"
#include "pinout.hpp"
#include <Arduino.h>
#include <TinyGPSPlus.h>

TinyGPSPlus gps;
HardwareSerial gpsSerial(1); // Using UART1

GpsHandler::GpsHandler(InstrumentDataModel& dataModel) : dataModel(dataModel)
{
    dataModel.addSensor(SensorId::GpsSpeed, 0.0f, SensorUnit(SensorUnitEnum::Kmph), (char*)"GPS Speed");
    dataModel.addSensor(SensorId::SatelliteCount, 0, SensorUnit(SensorUnitEnum::BlankUnit), (char*)"Satellites");
    dataModel.addSensor(SensorId::MinGpsSpeed, 0, SensorUnit(SensorUnitEnum::Kmph), (char*)"Speed Min.");
    dataModel.addSensor(SensorId::MaxGpsSpeed, 0, SensorUnit(SensorUnitEnum::Kmph), (char*)"Speed Max.");
    dataModel.addSensor(SensorId::AvgGpsSpeed, 0, SensorUnit(SensorUnitEnum::Kmph), (char*)"Speed Avg.", true);
    dataModel.addSensor(SensorId::DateTimeHour, 0, SensorUnit(SensorUnitEnum::BlankUnit), (char*)"Hour");
    dataModel.addSensor(SensorId::DateTimeMinute, 0, SensorUnit(SensorUnitEnum::BlankUnit), (char*)"Minute");
    dataModel.addSensor(SensorId::DateTimeSecond, 0, SensorUnit(SensorUnitEnum::BlankUnit), (char*)"Second");
    dataModel.addSensor(SensorId::DateTimeDay, 0, SensorUnit(SensorUnitEnum::BlankUnit), (char*)"Day");
    dataModel.addSensor(SensorId::DateTimeMonth, 0, SensorUnit(SensorUnitEnum::BlankUnit), (char*)"Month");
    dataModel.addSensor(SensorId::DateTimeYear, 0, SensorUnit(SensorUnitEnum::BlankUnit), (char*)"Year");
    dataModel.updateSensor(SensorId::GpsSpeed, 0.0f);
    dataModel.updateSensor(SensorId::SatelliteCount, 0);
    dataModel.updateSensor(SensorId::MinGpsSpeed, 0.0f);
    dataModel.updateSensor(SensorId::MaxGpsSpeed, 0.0f);
    dataModel.updateSensor(SensorId::AvgGpsSpeed, 0.0f);
    dataModel.disableSensor(SensorId::MinGpsSpeed);
    dataModel.disableSensor(SensorId::MaxGpsSpeed);
    dataModel.disableSensor(SensorId::AvgGpsSpeed);
}

void GpsHandler::init()
{
    // Force GPS module on if required by your hardware
    pinMode(FORCE_ON_PIN, OUTPUT);
    digitalWrite(FORCE_ON_PIN, HIGH); 
    delay(2000);

    // 1. Start Serial 1 with the slow default baud rate
    gpsSerial.begin(GPS_BAUD_SLOW, SERIAL_8N1, RX_PIN, TX_PIN);
    delay(2000);

    // 2. Send command to change GPS baud rate to 115200
    gpsSerial.println("$PMTK251,115200*1F");
    delay(500);

    // 3. Reinitialize Arduino serial to the new fast baud rate
    gpsSerial.end();
    delay(500);

    gpsSerial.setRxBufferSize(GPS_BUFFER);
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);

    // 4. Send additional MTK commands
    gpsSerial.println("$PMTK225,0*2B");
    delay(100);
    gpsSerial.println("$PMTK353,1,1,1,0,0*2A");

    Serial.println("GPS Initialized. Waiting for satellite fix...");
}

void GpsHandler::updateGpsData()
{
    // Feed the GPS parser with incoming serial data
    while (gpsSerial.available() > 0) {
        char c = gpsSerial.read();
        
        // This will print the raw data coming from the GPS
        //Serial.print(c); 
        
        gps.encode(c);
    }

    // Print data only when a new location packet is successfully parsed
    if (gps.location.isUpdated()) {
        Serial.println("--- GPS Data Updated ---");

        Serial.print("Satellites: ");
        Serial.println(gps.satellites.value());

        Serial.print("Latitude:   ");
        Serial.println(gps.location.lat(), 6); // Print to 6 decimal places

        Serial.print("Longitude:  ");
        Serial.println(gps.location.lng(), 6);

        Serial.print("Speed:      ");
        Serial.print(gps.speed.kmph());
        dataModel.updateSensor(SensorId::GpsSpeed, (float)gps.speed.kmph());
        dataModel.updateSensor(SensorId::AvgGpsSpeed, (float)gps.speed.kmph());
        dataModel.updateSensorIfSmaller(SensorId::MinGpsSpeed, (float)gps.speed.kmph());
        dataModel.updateSensorIfLarger(SensorId::MaxGpsSpeed, (float)gps.speed.kmph());
        dataModel.updateSensor(SensorId::SatelliteCount, (int)gps.satellites.value());

        dataModel.updateSensor(SensorId::DateTimeHour, (int)gps.time.hour());
        dataModel.updateSensor(SensorId::DateTimeMinute, (int)gps.time.minute());
        dataModel.updateSensor(SensorId::DateTimeSecond, (int)gps.time.second());
        dataModel.updateSensor(SensorId::DateTimeDay, (int)gps.date.day());
        dataModel.updateSensor(SensorId::DateTimeMonth, (int)gps.date.month());
        dataModel.updateSensor(SensorId::DateTimeYear, (int)gps.date.year());
        Serial.print("hour:      ");
        Serial.println((int)gps.time.hour());
        Serial.print("minute:    ");
        Serial.println((int)gps.time.minute());

        Serial.println(" km/h");

        Serial.println("------------------------\n");
    }
}
