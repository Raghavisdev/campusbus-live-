#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"
#include <string>
#include <vector>
#include "../models/Bus.h"
#include "../models/BusStop.h"
#include "../models/Route.h"

using namespace std;

class Database
{
private:

    sqlite3* db;

public:

    Database(string databasePath);
    ~Database();

    sqlite3* getDatabase() const;

    Bus getBus(string busID);
    vector<Bus> getAllBuses();
    Bus getBusStatus(string busID);
    vector<BusStop> getAllStops();
    Route getRoute(string routeID);
    vector<Route> getAllRoutes();
};

#endif