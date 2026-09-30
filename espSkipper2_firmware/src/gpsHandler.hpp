#include "instrumentDataModel.hpp"

#define GPS_BAUD_SLOW 9600
#define GPS_BAUD 115200
#define GPS_BUFFER 2048

class GpsHandler {
private:
    InstrumentDataModel& dataModel;
public:
    GpsHandler(InstrumentDataModel& dataModel);
    void Init();
    void UpdateGpsData();
    void Reload();
};