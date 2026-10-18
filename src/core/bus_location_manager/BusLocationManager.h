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

    unordered_map<string, vector<string>> busesOnRoute;

public:
    void addBus(Bus bus);
    void loadBuses(Database& database);

    bool hasBus(string busID) const;

    bool updateBusLocation(
        string busID,
        double latitude,
        double longitude,
        string lastUpdated
    );

    bool updateBusOccupancy(string busID);

    Bus getBus(string busID) const;
    vector<Bus> getAllBuses() const;
    vector<string> getBusesOnRoute(string routeID) const;
};

#endif