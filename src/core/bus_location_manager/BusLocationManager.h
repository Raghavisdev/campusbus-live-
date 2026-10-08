#ifndef BUS_LOCATION_MANAGER_H
#define BUS_LOCATION_MANAGER_H

#include <unordered_map>
#include <string>
#include <vector>
#include "../models/Bus.h"
#include "../flask_client/FlaskClient.h"
#include "../database/Database.h"

using namespace std;

class BusLocationManager
{
private:
     unordered_map<string, Bus> buses;

public:
    void addBus(Bus bus);
    void loadBuses(Database& database);

    void updateBusLocation(
        string busID,
        double latitude,
        double longitude,
        string lastUpdated
    );

    void updateBusOccupancy(string busID);

    Bus getBus(string busID) const;
    vector<Bus> getAllBuses() const;
};

#endif