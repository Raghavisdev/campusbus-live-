#include <iostream>
#include <vector>

#include "src/core/models/Bus.h"
#include "src/core/models/BusStop.h"
#include "src/core/models/Route.h"
#include "src/core/datastructures/Graph.h"
#include "src/core/ETA/ETAEngine.h"
#include "src/core/bus_location_manager/BusLocationManager.h"
#include "src/core/bus_recommendation/BusRecommendation.h"

using namespace std;

int main()
{
    // Stops

    BusStop stop1("S1", "Main Gate", 28.6139, 77.2090);
    BusStop stop2("S2", "Library", 28.6200, 77.2100);
    BusStop stop3("S3", "Canteen", 28.6300, 77.2150);
    BusStop stop4("S4", "Hostel", 28.6400, 77.2200);
    BusStop stop5("S5", "Sports Complex", 28.6500, 77.2250);
    BusStop stop6("S6", "Academic Block", 28.6600, 77.2300);

    // Route R1

    Route route1("R1");

    route1.addstop(stop1);
    route1.addstop(stop2, 5.0);
    route1.addstop(stop3, 7.0);
    route1.addstop(stop4, 4.0);
    route1.addstop(stop5, 6.0);
    route1.addstop(stop6, 5.0);

    // Route R2

    Route route2("R2");

    route2.addstop(stop1);
    route2.addstop(stop2, 4.0);
    route2.addstop(stop4, 6.0);
    route2.addstop(stop6, 5.0);

    vector<Route> routes;

    routes.push_back(route1);
    routes.push_back(route2);

    // Graph

    Graph graph;

    graph.addRoute(route1);
    graph.addRoute(route2);

    // Buses

    Bus bus1("B1", "R1", 50);
    Bus bus2("B2", "R1", 50);
    Bus bus3("B3", "R2", 40);

    bus1.updateOccupancy(30);
    bus2.updateOccupancy(50);
    bus3.updateOccupancy(20);

    // Bus manager

    BusLocationManager manager;

    manager.addBus(bus1);
    manager.addBus(bus2);
    manager.addBus(bus3);

    // User input

    string currentStop;
    string destination;

    cout << "Enter current stop ID: ";
    cin >> currentStop;

    cout << "Enter destination stop ID: ";
    cin >> destination;

    // Recommendation

    Bus recommendedBus = BusRecommendation::recommend(
        manager,
        graph,
        routes,
        currentStop,
        destination
    );

    // No bus found

    if (recommendedBus.getBusID() == "")
    {
        cout << endl;
        cout << "No suitable bus found." << endl;

        return 0;
    }

    // Calculate ETA for recommended bus

    double eta = -1.0;

    for (size_t i = 0; i < routes.size(); i++)
    {
        if (routes[i].getRouteId() == recommendedBus.getRouteID())
        {
            eta = ETA::calculateETA(
                graph,
                routes[i],
                currentStop,
                destination
            );

            break;
        }
    }

    // Display recommendation

    cout << endl;
    cout << "Recommended Bus: "
         << recommendedBus.getBusID() << endl;

    cout << "Route: "
         << recommendedBus.getRouteID() << endl;

    cout << "Available Seats: "
         << recommendedBus.getCapacity()
         - recommendedBus.getOccupancy() << endl;

    cout << "ETA: "
         << eta << " minutes" << endl;

    return 0;
}