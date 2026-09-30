#include "gpsHandler.hpp"
#include "pinout.hpp"
#include <Arduino.h>
#include <TinyGPSPlus.h>

TinyGPSPlus gps;
HardwareSerial gpsSerial(1); // Using UART1

GpsHandler::GpsHandler(InstrumentDataModel& dataModel) : dataModel(dataModel)
{
    dataModel.AddSensor(SensorId::GpsSpeed, 0.0f, SensorUnit(SensorUnitEnum::Kmph), TextKey::SensorGpsSpeed);
    dataModel.AddSensor(SensorId::SatelliteCount, 0.0f, SensorUnit(SensorUnitEnum::BlankUnit), TextKey::SensorSatelliteCount);
    dataModel.AddSensor(SensorId::MinGpsSpeed, 0.0f, SensorUnit(SensorUnitEnum::Kmph), TextKey::SensorMinGpsSpeed);
    dataModel.AddSensor(SensorId::MaxGpsSpeed, 0.0f, SensorUnit(SensorUnitEnum::Kmph), TextKey::SensorMaxGpsSpeed);
    dataModel.AddSensor(SensorId::AvgGpsSpeed, 0.0f, SensorUnit(SensorUnitEnum::Kmph), TextKey::SensorAvgGpsSpeed, true);
    dataModel.AddSensor(SensorId::GpsCourse, SensorValueString{}, SensorUnit(SensorUnitEnum::Degrees), TextKey::SensorGpsCourse);
    dataModel.AddSensor(SensorId::DateTimeHour, 0, SensorUnit(SensorUnitEnum::BlankUnit), TextKey::SensorDateTimeHour);
    dataModel.AddSensor(SensorId::DateTimeMinute, 0, SensorUnit(SensorUnitEnum::BlankUnit), TextKey::SensorDateTimeMinute);
    dataModel.AddSensor(SensorId::DateTimeSecond, 0, SensorUnit(SensorUnitEnum::BlankUnit), TextKey::SensorDateTimeSecond);
    dataModel.AddSensor(SensorId::DateTimeDay, 0, SensorUnit(SensorUnitEnum::BlankUnit), TextKey::SensorDateTimeDay);
    dataModel.AddSensor(SensorId::DateTimeMonth, 0, SensorUnit(SensorUnitEnum::BlankUnit), TextKey::SensorDateTimeMonth);
    dataModel.AddSensor(SensorId::DateTimeYear, 0, SensorUnit(SensorUnitEnum::BlankUnit), TextKey::SensorDateTimeYear);
}

void GpsHandler::Init()
{
    pinMode(GPS_FORCE_ON_PIN, OUTPUT);
    digitalWrite(GPS_FORCE_ON_PIN, HIGH); 
    delay(2000);

    // start serial with default baud rate
    gpsSerial.begin(GPS_BAUD_SLOW, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    delay(2000);

    // command to increase baud to 115200
    gpsSerial.println("$PMTK251,115200*1F");
    delay(500);

    // reinitialize serial with new baud rate
    gpsSerial.end();
    delay(500);

    gpsSerial.setRxBufferSize(GPS_BUFFER);
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    // enable all NMEA sentences and set update rate to 1hz
    gpsSerial.println("$PMTK225,0*2B");
    delay(100);
    gpsSerial.println("$PMTK353,1,1,1,0,0*2A");

    Serial.println("GPS Initialized. Waiting for satellite fix...");
}

void GpsHandler::UpdateGpsData()
{
    while (gpsSerial.available() > 0) {
        char c = gpsSerial.read();
        gps.encode(c);
    }

    if (gps.location.isUpdated()) {
        Serial.println("--- GPS Data Updated ---");

        Serial.print("Satellites: ");
        Serial.println(gps.satellites.value());

        Serial.print("Latitude:   ");
        Serial.println(gps.location.lat(), 6);

        Serial.print("Longitude:  ");
        Serial.println(gps.location.lng(), 6);

        Serial.print("Speed:      ");
        Serial.print(gps.speed.kmph());
        dataModel.UpdateSensor(SensorId::GpsSpeed, (float)gps.speed.kmph());
        dataModel.UpdateSensor(SensorId::AvgGpsSpeed, (float)gps.speed.kmph());
        dataModel.UpdateSensorIfSmaller(SensorId::MinGpsSpeed, (float)gps.speed.kmph());
        dataModel.UpdateSensorIfLarger(SensorId::MaxGpsSpeed, (float)gps.speed.kmph());
        dataModel.UpdateSensor(SensorId::SatelliteCount, (int)gps.satellites.value());

        SensorValueString courseStr;
        snprintf(courseStr.text, sizeof(courseStr.text), "%03d", (int)gps.course.deg());
        Serial.print("Course:     ");
        Serial.println(courseStr.text);
        dataModel.UpdateSensor(SensorId::GpsCourse, courseStr);

        dataModel.UpdateSensor(SensorId::DateTimeHour, (int)gps.time.hour());
        dataModel.UpdateSensor(SensorId::DateTimeMinute, (int)gps.time.minute());
        dataModel.UpdateSensor(SensorId::DateTimeSecond, (int)gps.time.second());
        dataModel.UpdateSensor(SensorId::DateTimeDay, (int)gps.date.day());
        dataModel.UpdateSensor(SensorId::DateTimeMonth, (int)gps.date.month());
        dataModel.UpdateSensor(SensorId::DateTimeYear, (int)gps.date.year());
    }
}

void GpsHandler::Reload()
{
}
