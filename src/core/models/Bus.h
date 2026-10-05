#ifndef BUS_H
#define BUS_H

#include <string>

using namespace std;

class Bus
{
private:

    string busID;
    string routeID;
    string dispatchTime;
    string lastUpdated;
    double latitude;
    double longitude;
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

    double getLatitude() const;
    double getLongitude() const;

    int getCapacity() const;
    int getOccupancy() const;
};

#endif