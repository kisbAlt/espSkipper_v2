#include "pinout.hpp"
#include "instrumentDataModel.hpp"
class AccelerometerHandler {
private:
    InstrumentDataModel& dataModel;
public:
    AccelerometerHandler(InstrumentDataModel& dataModel);
    void init();
    void readAccelerometerData();
    void reload();
};