#include <Arduino.h>
#include "display/displayHandler.hpp"
#include "accelerometerHandler.hpp"
#include "gpsHandler.hpp"
#include "Wire.h"
#include "SPI.h"
#include "display/displayUtils.hpp"
#include "instrumentDataModel.hpp"

InstrumentDataModel instrumentDataModel;
DisplayHandler displayHandler(instrumentDataModel);
AccelerometerHandler accelerometerHandler(instrumentDataModel);
GpsHandler gpsHandler(instrumentDataModel);

void setup() {
  Serial.begin(115200);
  delay(1000); // Give serial a moment to wake up

  Serial.println("Init PINs and SPI");
  pinMode(LIS3DH_CS, OUTPUT);
  digitalWrite(LIS3DH_CS, HIGH);
  pinMode(LCD_CS, OUTPUT);
  digitalWrite(LCD_CS, HIGH);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  Serial.println("Init Display");
  displayHandler.init();
  Serial.println("Init Accelerometer");
  accelerometerHandler.init();
  Serial.println("Init GPS");
  gpsHandler.init();

  Serial.println("Setup complete.");
}

int dataCount = 3;
DisplayDataEntity datapoints[10] = {
    {"Speed", "Knots", 0.95, 0},
    {"Accel Y", "m/s^2", 5.0, 1},
    {"Accel Z", "m/s^2", 3.0, 2},
};

void loop() {
  displayHandler.updateDisplay(datapoints, dataCount);
  accelerometerHandler.readAccelerometerData();
  gpsHandler.updateGpsData();
  delay(500);
  Serial.println("test");
}
