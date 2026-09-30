#include "depthHandler.hpp"
#include <Arduino.h>
#include <HardwareSerial.h>

#define NUM_SAMPLES 1350
#define SERIAL_BUFFER 4096

const uint8_t START_BYTE = 0xAA;
const size_t PAYLOAD_SIZE = 6 + NUM_SAMPLES;
const size_t TOTAL_PACKET_SIZE = 1 + PAYLOAD_SIZE + 1;

HardwareSerial ser(2); // use UART2

static uint8_t rxBuffer[SERIAL_BUFFER];
static size_t rxLen = 0;

void DepthHandler::Begin()
{
    ser.setRxBufferSize(SERIAL_BUFFER);
    ser.begin(38400, SERIAL_8N1, 7, -1); // RX=GPI7
}

DepthHandler::DepthHandler(const Settings &settings, InstrumentDataModel &dataModel)
    : dataModel(dataModel)
{
    dataModel.AddSensor(SensorId::WaterDepth, (float)0.0f, SensorUnitEnum::Meter, TextKey::SensorDepth);
}

bool DepthHandler::ReadPacket()
{

    bool foundValidPacket = false;
    uint16_t latestDepthIndex = 0;

    while (ser.available() >= 4)
    {

        // if the first byte isn't our start byte, pop it out and loop again
        if (ser.peek() != START_BYTE)
        {
            ser.read();
            continue;
        }

        // We have a start byte, pull exactly 4 bytes from the buffer
        uint8_t packet[4];
        ser.readBytes(packet, 4);

        // Verify Checksum
        uint8_t calcChecksum = packet[1] ^ packet[2];
        if (calcChecksum != packet[3])
        {
            // checksum mismatch
            continue;
        }

        latestDepthIndex = packet[1] | (packet[2] << 8);
        foundValidPacket = true;
    }

    if (foundValidPacket)
    {
        float actualDepthMeters = latestDepthIndex * 0.009504f;
        
        dataModel.UpdateSensor(SensorId::WaterDepth, actualDepthMeters);
        return true;
    }

    return false;
}
