#include <iostream>
#include <vector>

#include "src/core/database/database.h"
#include "src/core/datastructures/Graph.h"

using namespace std;

int main()
{
    Database database("database/campusbus.db");

    vector<BusStop> stops = database.getAllStops();

    cout << "Stops loaded: " << stops.size() << endl;

    for (size_t i = 0; i < stops.size(); i++)
    {
        cout << "  " << stops[i].getStopID()
             << " - " << stops[i].getStopName() << endl;
    }

    cout << endl;

    vector<Route> routes = database.getAllRoutes();

    cout << "Routes loaded: " << routes.size() << endl;

    Graph graph;

    for (size_t i = 0; i < routes.size(); i++)
    {
        vector<BusStop> routeStops = routes[i].getRoute();
        vector<double> times = routes[i].getTravelTimes();

        cout << "  " << routes[i].getRouteId() << ": "
             << routeStops[0].getStopID();

        for (size_t j = 1; j < routeStops.size(); j++)
        {
            cout << " -(" << times[j - 1] << " min)-> "
                 << routeStops[j].getStopID();
        }

        cout << endl;

        graph.addRoute(routes[i]);
    }

    cout << endl << "Graph built from database routes." << endl;

    return 0;
}