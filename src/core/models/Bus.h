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
};

#endif