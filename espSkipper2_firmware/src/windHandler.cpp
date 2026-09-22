#include "windHandler.hpp"
#include "StringTranslator.hpp"

HardwareSerial windSerial(0); // UART0 for RS485 Wind Sensor

WindHandler::WindHandler(InstrumentDataModel &dataModel) : dataModel(dataModel)
{
}

uint16_t WindHandler::calculateCRC(byte *buf, int len)
{
    uint16_t crc = 0xFFFF;
    for (int pos = 0; pos < len; pos++)
    {
        crc ^= (uint16_t)buf[pos];
        for (int i = 8; i != 0; i--)
        {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

WindHandler::ApparentWind WindHandler::calculateApparentWind(const float gpsSpeed, int windDirection, float windSpeed) const
{
    ApparentWind aw;
    
    // Convert True Wind Angle (degrees) to radians
    float twaRad = windDirection * PI / 180.0;

    // Calculate the vector components
    float crosswind = windSpeed * sin(twaRad);
    float headwind = (windSpeed * cos(twaRad)) + gpsSpeed;

    // 1. Calculate Apparent Wind Speed using the hypotenuse
    aw.speed = hypot(crosswind, headwind);

    // 2. Calculate Apparent Wind Angle
    float awaRad = atan2(crosswind, headwind);
    aw.angle = awaRad * 180.0 / PI;

    // Normalize angle to 0-359
    if (aw.angle < 0) {
        aw.angle += 360.0;
    }

    return aw;
}

void WindHandler::init()
{
    dataModel.addSensor(SensorId::WindSpeed, 0.0f, SensorUnit(SensorUnitEnum::Mps), TextKey::SensorWindSpeed);
    dataModel.addSensor(SensorId::WindSpeedAWS, 0.0f, SensorUnit(SensorUnitEnum::Mps), TextKey::SensorWindSpeedAWS);
    dataModel.addSensor(SensorId::WindDirection, int(0), SensorUnit(SensorUnitEnum::Degrees), TextKey::SensorWindDirection);
    dataModel.addSensor(SensorId::WindDirectionAWA, int(0), SensorUnit(SensorUnitEnum::Degrees), TextKey::SensorWindDirectionAWA);


// NEW LINE: Anchor the RX pin HIGH while the SP3485 is in transmit mode
    // to prevent floating phantom bytes from blinding the UART.
    pinMode(RX_PIN, INPUT_PULLUP);

    // Initialize RS485 Serial port on ESP32-S3
    windSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

    // Tell the ESP32 hardware to take control of the RTS pin
    windSerial.setPins(RX_PIN, TX_PIN, -1, RTS_PIN); 
    
    // Enable automatic hardware toggling for RS485
    windSerial.setMode(UART_MODE_RS485_HALF_DUPLEX);

    currentState = WIND_IDLE;
    lastRequestTime = 0;
    
    Serial.println("Wind sensor Ready (Hardware RS485 Mode).");
}

void WindHandler::updateWindData()
{
    unsigned long currentMillis = millis();

    switch (currentState)
    {
        case WIND_IDLE:
            if (currentMillis - lastRequestTime >= 1000)
            {
                // Clear out any garbage in the buffer
                while (windSerial.available()) {
                    windSerial.read();
                }

                // Send the command - The ESP32 hardware automatically pulls RTS HIGH, 
                // sends the bits, and instantly pulls RTS LOW the nanosecond it finishes!
                windSerial.write(windSensorReq, sizeof(windSensorReq));

                // Move to waiting state and reset counters
                currentState = WIND_WAITING_RX;
                lastRequestTime = currentMillis; 
                bytesRead = 0;
            }
            break;

        case WIND_WAITING_RX:
            while (windSerial.available() && bytesRead < EXPECTED_RESPONSE_LEN)
            {
                responseBuffer[bytesRead] = windSerial.read();
                bytesRead++;
            }

            // Check if we received the full frame OR the 8-byte clipped frame
            if (bytesRead == EXPECTED_RESPONSE_LEN || (bytesRead == EXPECTED_RESPONSE_LEN - 1 && responseBuffer[0] == 0x03))
            {
                // RECOVERY MODE: If we got 8 bytes starting with 0x03, the SP3485 hardware 
                // clipped the first byte (0x01) because the sensor responded too quickly.
                if (bytesRead == EXPECTED_RESPONSE_LEN - 1)
                {
                    // Shift all bytes one position to the right to make room
                    for (int i = bytesRead; i > 0; i--) {
                        responseBuffer[i] = responseBuffer[i - 1];
                    }
                    // Re-insert the missing device address at the beginning
                    responseBuffer[0] = 0x01;
                    // We now have a full 9-byte frame
                }

                // Verify it's the correct device and function code
                if (responseBuffer[0] == 0x01 && responseBuffer[1] == 0x03)
                {
                    uint16_t calculatedCRC = calculateCRC(responseBuffer, 7);
                    uint16_t receivedCRC = responseBuffer[7] | (responseBuffer[8] << 8);

                    if (calculatedCRC == receivedCRC)
                    {
                        uint16_t speedRaw = (responseBuffer[3] << 8) | responseBuffer[4];
                        float windSpeed = speedRaw / 100.0; 

                        uint16_t dirRaw = (responseBuffer[5] << 8) | responseBuffer[6];
                        int windDirection = dirRaw; 
                        const float gpsSpeed = std::get<float>(dataModel.getSensorValueInUnit(SensorId::GpsSpeed, SensorUnitEnum::Mps));
                        ApparentWind apparentWind = calculateApparentWind(gpsSpeed, windDirection, windSpeed);
                        
                        dataModel.updateSensor(SensorId::WindSpeed, windSpeed);
                        dataModel.updateSensor(SensorId::WindSpeedAWS, apparentWind.speed);
                        SensorValueString dirStr;
                        
                        dataModel.updateSensor(SensorId::WindDirection, windDirection);
                        dataModel.updateSensor(SensorId::WindDirectionAWA, apparentWind.angle);
                    }
                    else
                    {
                        Serial.println("Wind Sensor Error: CRC Failed");
                    }
                }
                
                currentState = WIND_IDLE; 
            }
            // Check for timeout (500ms has passed since we asked for data)
            else if (currentMillis - lastRequestTime >= 500)
            {
                if (bytesRead > 0) {
                    Serial.printf("Wind Sensor Error: Partial data (%d bytes). Bytes: ", bytesRead);
                    for (int i = 0; i < bytesRead; i++) {
                        Serial.printf("%02X ", responseBuffer[i]);
                    }
                    Serial.println();
                } else {
                    Serial.println("Wind Sensor Error: Timeout (No response)");
                }
                
                currentState = WIND_IDLE;
            }
            break;
    }
}

void WindHandler::reload()
{
}