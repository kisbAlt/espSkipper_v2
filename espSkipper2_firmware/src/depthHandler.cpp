#include "depthHandler.hpp"
#include <Arduino.h>
#include <HardwareSerial.h>

#define NUM_SAMPLES 1350
#define SERIAL_BUFFER 4096

const uint8_t START_BYTE = 0xAA;
const size_t PAYLOAD_SIZE = 6 + NUM_SAMPLES;
const size_t TOTAL_PACKET_SIZE = 1 + PAYLOAD_SIZE + 1;

HardwareSerial ser(2); // Use UART2

// We use a static sliding-window buffer so we don't drop the real
// Start Byte if we accidentally read a fake one in the ADC samples.
static uint8_t rxBuffer[SERIAL_BUFFER];
static size_t rxLen = 0;

void DepthHandler::begin()
{
    ser.setRxBufferSize(SERIAL_BUFFER);
    ser.begin(38400, SERIAL_8N1, 7, -1); // RX=GPI7
    //startBackgroundTask();
}

DepthHandler::DepthHandler(const Settings &settings, InstrumentDataModel &dataModel)
    : settings(settings), dataModel(dataModel)
{
    dataModel.addSensor(SensorId::WaterDepth, (float)0.0f, SensorUnitEnum::Meter, TextKey::SensorDepth);
}

bool DepthHandler::ReadPacket()
{

    bool foundValidPacket = false;
    uint16_t latest_depth_index = 0;

    // 2. Rapidly drain the entire hardware buffer
    while (ser.available() >= 4)
    {

        // If the first byte isn't our start byte, pop it out and loop again
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
            // Checksum mismatch. Discard and keep searching the buffer.
            continue;
        }

        // 3. Valid packet found! Save the raw integer, but DO NOT do the math yet.
        // If there are more packets in the buffer, this will just efficiently overwrite itself.
        latest_depth_index = packet[1] | (packet[2] << 8);
        foundValidPacket = true;
    }

    // 4. Buffer is now fully drained. If we found at least one valid packet,
    // run the heavy math and update the model using ONLY the absolute newest value.
    if (foundValidPacket)
    {
        float actual_depth_meters = latest_depth_index * 0.009504f;
        
        Serial.printf("UPDATED %f ---------------------\n", actual_depth_meters);
        dataModel.updateSensor(SensorId::WaterDepth, actual_depth_meters);
        return true;
    }

    return false;
}
