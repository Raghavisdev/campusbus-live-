#ifndef BUS_LOCATION_MANAGER_H
#define BUS_LOCATION_MANAGER_H

#include <unordered_map>
#include <string>
#include <vector>
#include "../models/Bus.h"

using namespace std;

class BusLocationManager
{
private:
     unordered_map<string, Bus> buses;

public:
    void addBus(Bus bus);
    void updateBusLocation(
        string busID,
        double latitude,
        double longitude
    );  
    Bus getBus(string busID) const;
    vector<Bus> getAllBuses() const;
};

#endif