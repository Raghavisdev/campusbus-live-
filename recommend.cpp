// Command-line entry point used by the Flask API.
// Usage: recommend.exe <currentStopID> <destinationStopID> [databasePath]
// Prints one line of JSON as the last line of output.

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

#include "src/core/database/database.h"
#include "src/core/bus_location_manager/BusLocationManager.h"
#include "src/core/bus_recommendation/BusRecommendation.h"
#include "src/core/datastructures/Graph.h"

using namespace std;

double roundMinutes(double minutes)
{
    return round(minutes * 10.0) / 10.0;
}

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

string stopName(const vector<BusStop>& stops, string stopID)
{
    for (size_t i = 0; i < stops.size(); i++)
    {
        if (stops[i].getStopID() == stopID)
        {
            return stops[i].getStopName();
        }
    }

    return stopID;
}

// The fastest journey as JSON: total minutes, number of bus changes and one entry per leg
// (consecutive stops on the same route form one leg).
string planJson(const Journey& journey, const vector<BusStop>& stops)
{
    string legs = "";
    int legCount = 0;
    size_t start = 0;

    for (size_t i = 1; i <= journey.legRoutes.size(); i++)
    {
        if (i == journey.legRoutes.size() || journey.legRoutes[i] != journey.legRoutes[start])
        {
            if (legCount > 0)
            {
                legs += ", ";
            }

            legs += "{\"route\": \"" + journey.legRoutes[start] + "\""
                  + ", \"from\": \"" + stopName(stops, journey.stops[start]) + "\""
                  + ", \"to\": \"" + stopName(stops, journey.stops[i]) + "\"}";

            legCount++;
            start = i;
        }
    }

    string json = "{\"minutes\": " + to_string((int)round(journey.minutes))
                + ", \"transfers\": " + to_string(legCount - 1)
                + ", \"legs\": [" + legs + "]}";

    return json;
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
        // No live bus: still show how the stops connect, using Dijkstra on the route graph
        Graph graph;

        for (size_t i = 0; i < routes.size(); i++)
        {
            graph.addRoute(routes[i]);
        }

        Journey journey = graph.shortestJourney(currentStop, destination);

        if (!journey.found)
        {
            printError("No bus available for this trip");
            return 0;
        }

        cout << "{\"ok\": false, \"error\": \"No bus available for this trip\", \"plan\": "
             << planJson(journey, stops) << "}" << endl;
        return 0;
    }

    int seatsLeft = result.bus.getCapacity() - result.bus.getOccupancy();

    cout << "{\"ok\": true"
         << ", \"bus\": \"" << result.bus.getBusID() << "\""
         << ", \"route\": \"" << result.bus.getRouteID() << "\""
         << ", \"wait_min\": " << roundMinutes(result.waitTime)
         << ", \"ride_min\": " << roundMinutes(result.rideTime)
         << ", \"eta_min\": " << roundMinutes(result.waitTime + result.rideTime)
         << ", \"seats_left\": " << seatsLeft
         << "}" << endl;

    return 0;
}
