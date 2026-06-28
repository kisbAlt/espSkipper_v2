#define GPS_BAUD_SLOW 9600
#define GPS_BAUD 115200
#define GPS_BUFFER 2048

class GpsHandler {

public:
    GpsHandler();
    void init();
    void updateGpsData();
};