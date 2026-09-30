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


    pinMode(RX_PIN, INPUT_PULLUP);
    windSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
    windSerial.setPins(RX_PIN, TX_PIN, -1, RTS_PIN); 
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
                while (windSerial.available()) {
                    windSerial.read();
                }

                windSerial.write(windSensorReq, sizeof(windSensorReq));

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

            if (bytesRead == EXPECTED_RESPONSE_LEN || (bytesRead == EXPECTED_RESPONSE_LEN - 1 && responseBuffer[0] == 0x03))
            {
                if (bytesRead == EXPECTED_RESPONSE_LEN - 1)
                {
                    for (int i = bytesRead; i > 0; i--) {
                        responseBuffer[i] = responseBuffer[i - 1];
                    }
                    responseBuffer[0] = 0x01;
                }

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