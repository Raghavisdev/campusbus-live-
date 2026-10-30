// Command-line entry point used by the Flask API.
// Usage: recommend.exe <currentStopID> <destinationStopID> [databasePath]
// Prints one line of JSON as the last line of output.

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "src/core/database/database.h"
#include "src/core/bus_location_manager/BusLocationManager.h"
#include "src/core/bus_recommendation/BusRecommendation.h"

using namespace std;

void printError(string message)
{
    cout << "{\"ok\": false, \"error\": \"" << message << "\"}" << endl;
}

bool stopExists(const vector<BusStop>& stops, string stopID)
{
    for (size_t i = 0; i < stops.size(); i++)
    {
        if (stops[i].getStopID() == stopID)
        {
            return true;
        }
    }

    return false;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printError("Usage: recommend <currentStopID> <destinationStopID> [databasePath]");
        return 1;
    }

    string currentStop = argv[1];
    string destination = argv[2];
    string databasePath = argc > 3 ? argv[3] : "database/campusbus.db";

    // sqlite3_open would silently create an empty file, so check first
    ifstream databaseFile(databasePath.c_str());

    if (!databaseFile.good())
    {
        printError("Database not found. Run: python database/init_db.py");
        return 1;
    }

    databaseFile.close();

    Database database(databasePath);

    vector<BusStop> stops = database.getAllStops();

    if (!stopExists(stops, currentStop) || !stopExists(stops, destination))
    {
        printError("Unknown stop");
        return 1;
    }

    if (currentStop == destination)
    {
        printError("Start and destination must be different");
        return 1;
    }

    vector<Route> routes = database.getAllRoutes();

    BusLocationManager manager;
    manager.loadBuses(database);

    Recommendation result = BusRecommendation::recommendLive(
        manager,
        routes,
        currentStop,
        destination
    );

    if (result.bus.getBusID() == "")
    {
        printError("No bus available for this trip");
        return 0;
    }

    int seatsLeft = result.bus.getCapacity() - result.bus.getOccupancy();

    cout << "{\"ok\": true"
         << ", \"bus\": \"" << result.bus.getBusID() << "\""
         << ", \"route\": \"" << result.bus.getRouteID() << "\""
         << ", \"wait_min\": " << result.waitTime
         << ", \"ride_min\": " << result.rideTime
         << ", \"eta_min\": " << result.waitTime + result.rideTime
         << ", \"seats_left\": " << seatsLeft
         << "}" << endl;

    return 0;
}
