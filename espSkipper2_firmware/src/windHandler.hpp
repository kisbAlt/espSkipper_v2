#pragma once
#include "instrumentDataModel.hpp"
#include "pinout.hpp"

class WindHandler {
private:
    struct ApparentWind {
        int angle;
        float speed;
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