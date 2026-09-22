#pragma once
#include "instrumentDataModel.hpp"

#define RX_PIN  16  // Connects to SP3485 RX-I
#define TX_PIN  15  // Connects to SP3485 TX-O
#define RTS_PIN 6   // Connects to SP3485 RTS (Transmit/Receive Control)

class WindHandler {
private:
    struct ApparentWind {
        float angle;
        int speed;
    };
    InstrumentDataModel& dataModel;
    const static constexpr byte windSensorReq[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B};
    const static int EXPECTED_RESPONSE_LEN = 9;
    byte responseBuffer[EXPECTED_RESPONSE_LEN];
    uint16_t calculateCRC(byte* buf, int len);
    enum WindState { WIND_IDLE, WIND_WAITING_RX };
    WindState currentState = WIND_IDLE;
    ApparentWind calculateApparentWind(const float gpsSpeed, int windDirection, float windSpeed) const;
    
    unsigned long lastRequestTime = 0;
    int bytesRead = 0;
public:
    WindHandler(InstrumentDataModel& dataModel);
    void init();
    void updateWindData();
    void reload();
};