#include "gpsHandler.hpp"
#include "pinout.hpp"
#include <Arduino.h>
#include <TinyGPSPlus.h>

TinyGPSPlus gps;
HardwareSerial gpsSerial(1); // Using UART1

GpsHandler::GpsHandler(InstrumentDataModel& dataModel) : dataModel(dataModel)
{
    dataModel.addSensor(SensorId::GpsSpeed, 0.0f, SensorUnit::Knot, (char*)"GPS Speed");
    dataModel.addSensor(SensorId::SatelliteCount, 0, SensorUnit::BlankUnit, (char*)"Satellites");
}

void GpsHandler::init()
{
    // Start PC Serial Monitor
    Serial.begin(115200);
    while (!Serial) { ; } // Wait for serial port to connect
    Serial.println("Starting GPS Test...");

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
        dataModel.updateSensor(SensorId::GpsSpeed, (float)gps.speed.knots());
        dataModel.updateSensor(SensorId::SatelliteCount, (int)gps.satellites.value());
        Serial.println(" km/h");

        Serial.println("------------------------\n");
    }
}
