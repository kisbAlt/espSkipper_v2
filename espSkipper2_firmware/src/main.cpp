#include <Arduino.h>
#include "display/displayHandler.hpp"
#include "accelerometerHandler.hpp"
#include "gpsHandler.hpp"
#include "Wire.h"
#include "SPI.h"

DisplayHandler displayHandler;
AccelerometerHandler accelerometerHandler;
GpsHandler gpsHandler;

void setup() {
  Serial.begin(115200);
  delay(1000); // Give serial a moment to wake up

  Serial.println("Init PINs and SPI");
  pinMode(LIS3DH_CS, OUTPUT);
  digitalWrite(LIS3DH_CS, HIGH);
  pinMode(5, OUTPUT); // Display CS (Pin 5)
  digitalWrite(5, HIGH);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  Serial.println("Init Display");
  displayHandler.init();
  Serial.println("Init Accelerometer");
  accelerometerHandler.init();
  Serial.println("Init GPS");
  gpsHandler.init();

  Serial.println("Setup complete.");
}

void loop() {
  displayHandler.updateDisplay();
  accelerometerHandler.readAccelerometerData();
  gpsHandler.updateGpsData();
  delay(1000);
  Serial.println("test");
}
