#ifndef BUS_H
#define BUS_H

#include <string>
#include "Location.h"

using namespace std;

class Bus : public Location
{
private:

    string busID;
    string routeID;
    string dispatchTime;
    string lastUpdated;
    int capacity;
    int occupancy;

    // Position on the route's road line (set from the GPS endpoint's result):
    // -1 = unknown (no road line), 0 = not available (off route / stale / finished),
    // 1.. = next stop ahead (1-based stop order)
    int nextStopOrder;
    double segmentProgress;

    // Actual / planned time over the last few minutes (1 = on plan, 2 = twice as slow)
    double paceRatio;

public:

    Bus(string busID, string routeID, int capacity);

    void updateLocation(double latitude, double longitude);
    void updateOccupancy(int occupancy);

    void setDispatchTime(string dispatchTime);
    void setLastUpdated(string lastUpdated);

    string getDispatchTime() const;
    string getLastUpdated() const;

    string getBusID() const;
    string getRouteID() const;

    int getCapacity() const;
    int getOccupancy() const;

    void setRoutePosition(int nextStopOrder, double segmentProgress);
    int getNextStopOrder() const;
    double getSegmentProgress() const;

    void setPaceRatio(double paceRatio);
    double getPaceRatio() const;
};

#endif